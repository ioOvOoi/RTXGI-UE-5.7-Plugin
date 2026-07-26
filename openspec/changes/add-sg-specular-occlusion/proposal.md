## Why

The current 8-probe blending loop in `ApplyVolumeLightingContribution` applies Chebyshev visibility weights based on the **probe-to-surface direction** — "is the path from the surface to this probe occluded?" This protects SG diffuse correctly, but SG specular needs visibility along the **reflection direction R** — "is the path from the surface along R occluded?"

When a probe is visible from the surface but the reflection direction is blocked by nearby geometry, the SG specular highlight leaks through — producing "bright reflections in shadow" artifacts (e.g., specular highlights under furniture, in corners, behind walls). This is the same artifact The Order 1886 solved with dedicated reflection occlusion.

This change adds a reflection-direction visibility query using the DDGI distance field. After the 8-probe blend loop, the argmax-weight probe is queried along R to compute a scalar specular occlusion factor, which is passed into `RTXGISGSpecularIBL` as an additional parameter.

## What Changes

- In the 8-probe blending loop (`ApplyVolumeLightingContribution`), track the probe with the highest blend weight (`argmaxProbeIndex`, `argmaxWeight`).
- After the loop, if SG specular is active:
  - Compute reflection direction `R = reflect(-CameraDirection, Normal)`
  - Query the argmax probe's distance field along `-R` (octahedral coordinates)
  - Read `filteredDist_R = ProbeDistance.SampleLevel(LinearClamp, distUV_R, 0).rg`
  - Apply surface-to-probe offset correction: `correctedMeanDist = max(meanDist_R - dot(surfaceToProbe, R), 0)`
  - Compute specular occlusion using exponential distance decay + roughness-modulated power:
    ```
    distVisibility = 1.0 - exp(-correctedMeanDist / probeGridSpacing * strength)
    NdotR = saturate(dot(Normal, R))
    power = lerp(16.0, 1.0, roughness)
    specOcc = saturate(pow(distVisibility * NdotR, power))
    ```
  - Pass `specOcc` as a new `float` parameter to `RTXGISGSpecularIBL`
- In `RTXGISGSpecularIBL`, multiply the final result by `specOcc`:
  ```
  return radiance * brdf * horizon * specOcc;
  ```
- Add CVars:
  - `r.RTXGI.DDGI.SG.SpecularOcclusion` (bool, default `1` = enabled)
  - `r.RTXGI.DDGI.SG.SpecularOcclusionStrength` (float, default `1.0` — multiplier on the distance decay rate)
- When `SpecularOcclusion` is disabled, pass `1.0` as `specOcc` (no-op).

## Capabilities

### New Capabilities

- `sg-specular-occlusion`: Reflection-direction visibility occlusion for SG specular, computed from the DDGI distance field along the reflection vector using the highest-weight probe.

### Modified Capabilities

- `sg-radiance-ddgi-lighting`: SG rough specular is now modulated by a reflection-direction occlusion factor derived from the DDGI distance field, eliminating "bright reflection in shadow" artifacts.

## Impact

- `Shaders/Private/ApplyLightingDeferred.usf` — `ApplyVolumeLightingContribution` gains argmax tracking in the 8-probe loop and a post-loop specular occlusion computation block.
- `Shaders/Private/SDK/SGLighting.ush` — `RTXGISGSpecularIBL` gains a `specOcc` float parameter.
- New CVars: `r.RTXGI.DDGI.SG.SpecularOcclusion` (default `1`), `r.RTXGI.DDGI.SG.SpecularOcclusionStrength` (default `1.0`).
- Cost: 1 extra `ProbeDistance.SampleLevel` + argmax tracking (1 compare per probe in the loop) + ~10 ALU for the occlusion formula. `ProbeDistance` is already bound (no new SRV).
- The `specOcc` parameter is orthogonal to the `sgSpecMask` (roughness cull) from `add-sg-specular-roughness-cull` — both multiply the final result.
