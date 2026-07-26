## Context

RTXGI's deferred lighting pass accumulates (`+=`) SG specular into `SceneColor`. UE's ReflectionEnvironment also writes specular (from cubemaps and SSR) into `SceneColor`. When both are active, smooth surfaces receive double specular — the SG highlight叠加 the cubemap reflection.

The Order 1886 divided labor: SG specular for rough surfaces, cubemaps for smooth surfaces. This change applies the same division without reading cubemaps from the plugin — SG specular simply attenuates at low roughness.

## Goals / Non-Goals

**Goals:**
- Attenuate SG specular below a configurable roughness range to avoid double-specular with UE cubemaps.
- Keep the attenuation smooth (no hard pop at the boundary).
- Make the roughness range CVar-tunable for different project cubemap setups.
- Mark the Route B integration point (cubemap internal blend) with a TODO for future work.

**Non-Goals:**
- Do not read UE's ReflectionCapture cubemap atlas from the plugin (Route B — future work, marked with TODO).
- Do not disable or modify UE's reflection system.
- Do not attenuate SG diffuse (only specular is affected).
- Do not add a hard on/off toggle — the roughness range is the control.

## Decisions

### Decision: Route C (SG self-attenuation, no cubemap reading)

SG specular attenuates itself at low roughness. UE's cubemap reflection remains untouched. This avoids engine-internal texture access, SRV pressure, and probe selection complexity.

Alternatives considered:
- Route A (roughness-based division of labor with UE cubemaps): rejected — RTXGI cannot control UE cubemap enable/disable per-pixel.
- Route B (RTXGI reads cubemap atlas, internal blend): rejected for now — requires engine-internal API access, additional SRV, and probe selection. Marked as TODO for future work.

### Decision: Configurable roughness range via two CVars

`RoughnessCullMin` (default `0.2`) and `RoughnessCullMax` (default `0.5`) define the transition zone. Below `min`, SG specular is zero. Above `max`, SG specular is full strength. Between `min` and `max`, smooth linear transition.

Two separate CVars (not a single string) allow independent tuning and are simpler to set from console.

### Decision: TODO comment for Route B

A `// TODO (Route B):` comment at the `sgSpecMask` application point documents the future cubemap-internal-blend path, including what it would need (ReflectionCapture atlas SRV, probe selection, roughness blend).

## Risks / Trade-offs

- **No cubemap = no smooth specular**: If the project has no ReflectionCapture actors placed, smooth surfaces (roughness < 0.2) will have no indirect specular at all. This is expected — Route B would fix it. The TODO documents this.
- **CVar tuning**: Different projects have different cubemap quality. The default 0.2–0.5 range may need adjustment. Two CVars provide fine-grained control.
- **Transition visibility**: If `RoughnessCullMin` and `RoughnessCullMax` are set too close together, the transition may be visible as a hard line. Default 0.3-wide range is smooth enough.

## Migration Plan

1. Add CVars `r.RTXGI.DDGI.SG.Specular.RoughnessCullMin` (default `0.2`) and `RoughnessCullMax` (default `0.5`) in `DDGIVolumeComponent.cpp`.
2. Pass both values to the shader parameters struct.
3. In `RTXGISGSpecularIBL`, compute `sgSpecMask` and multiply the return value.
4. Add TODO comment for Route B.
5. Validate with UE ReflectionCapture actors present (smooth surfaces get cubemap, rough surfaces get SG).

Rollback: Set `RoughnessCullMin = 0.0` and `RoughnessCullMax = 0.0` to disable the mask (sgSpecMask = 1 everywhere).

## Implementation Notes

- The mask is applied after `specOcc` (from `add-sg-specular-occlusion`), so the final return is:
  ```
  return radiance * brdf * horizon * specOcc * sgSpecMask;
  ```
- The `roughness` used for the mask is the same material roughness passed to `RTXGISGSpecularIBL` (after NDF filtering if `add-sg-specular-ndf-filtering` is enabled).
- If both CVars are equal and > 0, the transition becomes a hard cutoff — avoid by keeping `max > min`.
