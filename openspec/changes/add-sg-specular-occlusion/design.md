## Context

The 8-probe blending loop in `ApplyVolumeLightingContribution` computes Chebyshev visibility weights based on the probe-to-surface direction. This visibility is baked into the `blendedAmps` array, which feeds both SG diffuse and SG specular. However, specular visibility should be evaluated along the reflection direction R, not the probe direction — a probe may be visible from the surface while R is occluded (or vice versa), causing specular highlights to leak through occluded geometry.

## Goals / Non-Goals

**Goals:**
- Add reflection-direction visibility for SG specular using the DDGI distance field.
- Use the highest-weight (argmax) probe from the 8-probe blend to query the distance field along R.
- Apply surface-to-probe offset correction for distance accuracy.
- Use a roughness-modulated power function so smooth surfaces are more sensitive to occlusion than rough surfaces.
- Default enabled, with CVar to disable and a strength multiplier.
- Pass the occlusion as a scalar into `RTXGISGSpecularIBL` (option 3: internal parameter, not external multiply).

**Non-Goals:**
- Do not query all 8 probes along R (argmax is sufficient; 8× cost not justified).
- Do not replace the existing probe-direction Chebyshev in the blend loop (it stays for diffuse).
- Do not compute bent normals or directional AO (scalar occlusion along R is sufficient).
- Do not modify the SG diffuse path.

## Decisions

### Decision: Argmax probe for R-direction query

The argmax-weight probe from the 8-probe blend is the closest/most-contributing probe. Querying its distance field along R gives a good approximation of the reflection-direction visibility without querying all 8 probes. The distance field changes smoothly between adjacent probes, so using the argmax probe is a reasonable single-sample approximation.

Alternatives considered:
- Base probe (`baseProbeCoords`): rejected — not necessarily the closest; argmax is more representative.
- All 8 probes weighted: rejected — 8× the cost for marginal accuracy gain.

### Decision: Exponential distance decay + roughness power (Approach A)

The distance field gives the mean distance to geometry along R from the probe. There is no "expected ray length" for specular (unlike diffuse Chebyshev which has the surface-to-probe distance as a baseline). Approach A converts distance to visibility via exponential decay, then modulates by a roughness-controlled power:

```
distVisibility = 1.0 - exp(-correctedMeanDist / probeGridSpacing * strength)
NdotR = saturate(dot(N, R))
power = lerp(16.0, 1.0, roughness)  // smooth: sharp falloff; rough: gentle falloff
specOcc = saturate(pow(distVisibility * NdotR, power))
```

`probeGridSpacing` normalizes the distance to be scale-independent. `strength` is a CVar multiplier.

Alternatives considered:
- UE5 `GetSpecularOcclusion(AO, BentNormal, R, Roughness)`: rejected — requires a precomputed AO scalar, which DDGI does not provide.
- Chebyshev with estimated ray length: rejected — the "expected ray length" is a guess; exponential decay is more robust without a baseline.

### Decision: Surface-to-probe offset correction

The distance field gives distances from the probe, not from the surface. Projecting the surface-to-probe vector onto R and subtracting gives a corrected distance from the surface along R:

```
correctedMeanDist = max(meanDist_R - dot(normalize(probeWorldPos - biasedWorldPosition), R), 0.0)
```

This adds ~3 ALU instructions and improves accuracy when the probe is far from the surface.

### Decision: Option 3 integration (scalar parameter into RTXGISGSpecularIBL)

`specOcc` is computed in `ApplyVolumeLightingContribution` (which has access to the distance field and probe data) and passed as a `float` into `RTXGISGSpecularIBL`. This keeps `SGLighting.ush` decoupled from DDGI infrastructure while allowing the occlusion to be applied inside the BRDF function.

Alternatives considered:
- External multiply (option 1): rejected — less flexible for future changes to application point.
- Internal with full distance field data (option 2): rejected — couples `SGLighting.ush` to DDGI textures and probe indexing.

## Risks / Trade-offs

- **Single-probe approximation**: Using argmax probe for R-direction visibility may miss cases where the argmax probe's distance field doesn't represent the reflection path well. Mitigation: distance field is smooth between probes; argmax is the best single-probe choice.
- **No variance term**: The distance field stores mean + variance. The current formula uses only mean. Variance could be used for a Chebyshev-style soft falloff, but without a baseline distance, the formula is speculative. Mean-only is simpler and sufficient for the "kill leaks" goal.
- **Strength tuning**: The `strength` CVar multiplier may need per-scene tuning. Default `1.0` should work for typical probe grid spacings; the `probeGridSpacing` normalization makes it scale-independent.

## Migration Plan

1. In `ApplyVolumeLightingContribution`, add `argmaxProbeIndex` and `argmaxWeight` tracking variables, updated inside the 8-probe loop.
2. After the loop, add a CVar-guarded block that computes `specOcc`:
   - Compute `R = reflect(-CameraDirection, Normal)`
   - Get argmax probe world position (with relocation offset if enabled)
   - Query `ProbeDistance` along `-R` using octahedral coordinates
   - Apply offset correction
   - Compute `specOcc` via the exponential + power formula
3. Add `specOcc` as a parameter to `RTXGISGSpecularIBL` and multiply the return value.
4. Add CVars `r.RTXGI.DDGI.SG.SpecularOcclusion` (default `1`) and `r.RTXGI.DDGI.SG.SpecularOcclusionStrength` (default `1.0`).
5. When CVar is off, pass `1.0` as `specOcc`.

Rollback: `r.RTXGI.DDGI.SG.SpecularOcclusion 0` disables the occlusion (passes `1.0`).

## Implementation Notes

- `ProbeDistance` is already bound in `ApplyVolumeLightingContribution` as a function parameter.
- `DDGIGetOctahedralCoordinates`, `DDGIGetProbeUV`, and `DDGIGetProbeIndex` are already used in the 8-probe loop — reuse the same helpers.
- `probeGridSpacing` is available from the `Volume` struct (`Volume.probeGridSpacing`).
- The argmax probe's world position may need `ProbeOffsets` if relocation is enabled (same pattern as the existing loop).
- `RTXGISGSpecularIBL` signature change: add `float specOcc` as the last parameter. All call sites pass either the computed `specOcc` or `1.0`.
