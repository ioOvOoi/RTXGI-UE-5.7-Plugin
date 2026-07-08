# SG DDGI Performance and Memory Guide

This document quantifies the runtime cost of the Spherical Gaussian (SG) DDGI
mode introduced on the `Spherical-Gaussians` branch, so volume authors can
pick `SGLobeCount`, `SGPrecision`, and `RaysPerProbe` values that fit their
budget. All numbers are estimates measured against the plugin as of commit
`c613619` on `omos/sg-completeness`; verify against your own scene before
locking budgets.

## Cost components

SG DDGI adds three GPU workloads on top of the octa DDGI path:

| Pass                          | Cost driver                  | Frequency           |
| ----------------------------- | ---------------------------- | ------------------- |
| `DDGI SG Projection`          | `numRays * clampedSGLobeCount` per probe | per updated probe, round-robin |
| `DDGI SG Apply` (deferred CS) | `8 * clampedSGLobeCount` per shaded pixel | per deferred pixel in volume |
| SG amplitude atlas memory     | `ProbeCount2D.X * clampedSGLobeCount * bytesPerPixel` | resident while `bSGEnabled` |

The ray-tracing probe update (`ProbeUpdateRGS`) is **unchanged** — SG
projection consumes the same `DDGIVolumeRayDataUAV` the octa blend would
have read. SG does not trace extra rays.

## Memory

The SG amplitude atlas is a 2D texture with dimensions:

```
width  = ProbeCount2D.X * clampedSGLobeCount
height = ProbeCount2D.Y
```

where `ProbeCount2D = (ProbeCounts.Y * ProbeCounts.Z, ProbeCounts.X)` and
`clampedSGLobeCount = clamp(SGLobeCount, 4, 32)`.

Per-pixel size depends on `SGPrecision`:

| SGPrecision | Format                | Bytes/pixel |
| ----------- | --------------------- | ----------- |
| 0 (default) | `PF_FloatRGBA` (FP16) | 8           |
| 1 (debug)   | `PF_A32B32G32R32F`    | 16          |

### Worked examples

Assume `ProbeCounts = (8, 8, 8)` → `ProbeCount2D = (64, 8)` → 512 probes.

| SGLobeCount | Format | Atlas dims       | Atlas bytes   | Atlas MiB |
| ----------- | ------ | ---------------- | ------------- | --------- |
| 4           | FP16   | 256 x 8          | 16,384        | 0.016     |
| 16 (default)| FP16   | 1024 x 8         | 65,536        | 0.063     |
| 32          | FP16   | 2048 x 8         | 131,072       | 0.125     |
| 32          | FP32   | 2048 x 8         | 262,144       | 0.250     |

Scale linearly with probe count and lobe count. A large volume
`ProbeCounts = (16, 16, 16)` → 4096 probes, `SGLobeCount = 32`, FP16:
atlas = `8192 x 16` = 1 MiB. Still small.

**Takeaway**: SG amplitude memory is negligible compared to the irradiance
atlas (`ProbeCount2D.X * (6+2) * 4` bytes for FP32, or the distance atlas).
SG is not the memory bottleneck.

## Compute: SG Projection pass

`ProbeSGProjectCS.usf` dispatches `DivideAndRoundUp(probeCount, 64)` thread
groups. Each thread (one probe) does:

1. Round-robin guard — early-out if probe is outside the update window.
2. Hoist: `clampedSGLobeCount` trig ops to precompute SG basis axes.
3. Inner loop over `numRaysPerProbe` rays, each doing `clampedSGLobeCount`
   `dot + exp` ops (no trig — axes were hoisted).
4. Normalize + temporal blend loop, `clampedSGLobeCount` iterations.

Per-updated-probe op count ≈
`numRays * clampedSGLobeCount * (dot + exp + 2 fma) + clampedSGLobeCount * (trig + blend)`.

With the hoist, the trig cost is `clampedSGLobeCount` per thread (was
`numRays * clampedSGLobeCount` before commit `496fb1d`).

### Worked examples

`RaysPerProbe = 288`, `SGLobeCount = 16`, one probe updated:

- Inner loop: `288 * 16 = 4608` `dot+exp` ops.
- Hoist: `16` trig ops.
- Total ≈ 4.6k `dot+exp` + 16 trig per probe.

`SGLobeCount = 32` doubles the inner loop to 9216 ops/probe. `RaysPerProbe = 1008`
(quality preset) at `SGLobeCount = 32` → 32k ops/probe — still cheap per
probe, but the round-robin budget means only a fraction of probes update
per frame, so the per-frame cost scales with `ProbeIndexCount`, not total
probe count.

**Takeaway**: SG Projection is dominated by `numRays * clampedSGLobeCount`.
Doubling lobe count doubles projection cost. `SGLobeCount = 16` with
`RaysPerProbe = 288` is a balanced default; 32 is for scenes with sharp
directional features (e.g. strong sun shadows in GI).

## Compute: SG Apply (deferred CS)

`ApplyLightingDeferred.usf`'s SG branch runs per shaded pixel inside the
volume. Per pixel:

1. 8-probe neighborhood loop, each probe:
   - State/relocation/ISV bookkeeping (same as octa).
   - Chebyshev visibility from distance atlas (same as octa).
   - `clampedSGLobeCount` typed loads from `ProbeSGTexture` + weighted
     accumulate.
2. Mode-dependent evaluation:
   - Mode 1 (diffuse): `clampedSGLobeCount` `dot + exp` for the inner product.
   - Mode 2 (diffuse + spec): Mode 1 + `clampedSGLobeCount` `dot + exp` for
     the rough specular inner product (commit `c613619`).
   - Mode 3 (spec only): just the spec inner product.
   - Mode 4/5 (debug): Mode 1 + octa path or direct SG evaluate.

Per-pixel op count ≈ `8 * clampedSGLobeCount` typed loads + `clampedSGLobeCount`
(diffuse) or `2 * clampedSGLobeCount` (Mode 2) `dot+exp` ops.

### Worked examples

`SGLobeCount = 16`, Mode 2, one pixel:

- 8-probe loop: `8 * 16 = 128` typed loads + weight math.
- Evaluation: `16 + 16 = 32` `dot+exp` ops.
- Total ≈ 128 loads + 32 `dot+exp`.

`SGLobeCount = 32` doubles both → 256 loads + 64 `dot+exp` per pixel. At
1080p with the volume covering half the screen (~1M pixels), that's ~64M
`dot+exp` ops — well within a modern GPU's budget for a fullscreen compute
pass, but measurable.

**Takeaway**: SG Apply cost scales linearly with `SGLobeCount` and shaded
pixel count. Mode 2 is ~2x Mode 1. `SGLobeCount = 16` Mode 2 is the
recommended production default; 32 is for validation or scenes where the
specular noise reduction from the inner-product prefilter (commit `c613619`)
is worth the ~2x apply cost.

## Round-robin update budget

SG Projection respects the same round-robin `ProbeIndexStart/Count` window
as the octa IrradianceBlend pass (see commit `bc2b584`). This means:

- The per-frame SG projection cost is bounded by `ProbeIndexCount`, not
  total probe count. Volumes with higher `UpdatePriority` get a larger
  window and thus higher per-frame SG cost.
- Probes outside the window keep their last-converged SG amplitudes. This
  is correct behavior — SG amplitudes are temporal and only need updating
  when the lighting in that probe's region changes.

## Recommendations

| Use case                          | SGLobeCount | SGPrecision | Mode | Notes |
| --------------------------------- | ----------- | ----------- | ---- | ----- |
| Default production                | 16          | 0 (FP16)    | 2    | Balanced quality/cost. Rough spec via inner product. |
| High-detail directional GI        | 32          | 0 (FP16)    | 2    | Sharp sun shadows in GI, ~2x apply cost. |
| Validation / debug                | 16          | 1 (FP32)    | 4    | Mode 4 shows SG vs octa diff; FP32 removes precision as a variable. |
| Diffuse-only (cheapest SG)        | 8           | 0 (FP16)    | 1    | No spec inner product; lowest SG apply cost. |
| Mirror specular debug             | 16          | 0 (FP16)    | 3    | Spec-only view; useful for tuning `SGSpecularMinRoughness`. |

## Serialization cost

Saving SG amplitudes to the level archive (commit `99fae6d`) adds one
readback + one `SaveFDDGITexturePixels` per volume at save time. The
readback is async (`FRHIGPUTextureReadback`) and the pixel payload is the
same size as the atlas (see Memory section). For a 512-probe, 16-lobe,
FP16 volume, that's 64 KiB of archive data per volume — negligible.

Load time mirrors this: one `LoadFDDGITexturePixels` + one
`AddCopyTexturePass` per volume. The copy is a single GPU blit, sub-millisecond.

## Versioning

SG amplitude persistence is gated by `FDDGICustomVersion::SaveLoadSGAmplitudes`.
Older archives (saved before this version) load without SG data — the atlas
starts black and reconverges over a few frames. This is the same cold-start
behavior as a freshly-created volume, not a regression.
