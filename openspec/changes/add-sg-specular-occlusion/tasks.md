## 1. Add specular occlusion CVars

- [ ] 1.1 Add `r.RTXGI.DDGI.SG.SpecularOcclusion` (bool, default `1`, `ECVF_RenderThreadSafe`) in `DDGIVolumeComponent.cpp`.
- [ ] 1.2 Add `r.RTXGI.DDGI.SG.SpecularOcclusionStrength` (float, default `1.0`, `ECVF_RenderThreadSafe`) in `DDGIVolumeComponent.cpp`.
- [ ] 1.3 Read both CVars on the render thread and pass to the shader parameters struct.

## 2. Track argmax probe in the 8-probe blend loop

- [ ] 2.1 In `ApplyVolumeLightingContribution`, add variables `argmaxProbeIndex` and `argmaxWeight` before the 8-probe loop.
- [ ] 2.2 Inside the loop, after computing `weight`, update argmax if `weight > argmaxWeight`.
- [ ] 2.3 Also track the argmax probe's world position (with relocation offset if enabled) for the offset correction.

## 3. Compute specOcc after the blend loop

- [ ] 3.1 After the 8-probe loop, add a CVar-guarded block (only when `bSGSpecularEnabled` and `SpecularOcclusion` CVar is on):
  - Compute `R = reflect(-CameraDirection, Normal)`
  - Get argmax probe octahedral coordinates for `-R`
  - Compute `distUV_R` using `DDGIGetProbeUV` with the argmax probe index
  - Read `filteredDist_R = ProbeDistance.SampleLevel(LinearClamp, distUV_R, 0).rg`
  - Extract `meanDist_R = filteredDist_R.x`
  - Apply offset correction: `correctedMeanDist = max(meanDist_R - dot(normalize(argmaxProbeWorldPos - biasedWorldPosition), R), 0.0)`
  - Compute `distVisibility = 1.0 - exp(-correctedMeanDist / Volume.probeGridSpacing * strength)`
  - Compute `NdotR = saturate(dot(Normal, R))`
  - Compute `power = lerp(16.0, 1.0, roughness)`
  - Compute `specOcc = saturate(pow(distVisibility * NdotR, power))`
- [ ] 3.2 If CVar is off, set `specOcc = 1.0`.

## 4. Add specOcc parameter to RTXGISGSpecularIBL

- [ ] 4.1 Add `float specOcc` as the last parameter to `RTXGISGSpecularIBL` in `SGLighting.ush`.
- [ ] 4.2 Multiply the return value by `specOcc`: `return radiance * brdf * horizon * specOcc;`
- [ ] 4.3 Update all call sites of `RTXGISGSpecularIBL` in `ApplyLightingDeferred.usf` to pass the computed `specOcc` (or `1.0` if occlusion is disabled).

## 5. Validate

- [ ] 5.1 Build the plugin shader and confirm no compile errors.
- [ ] 5.2 Visual validation: surface under furniture / in corners / behind walls shows no SG specular leak.
- [ ] 5.3 Visual validation: unobstructed surfaces show no visible change with occlusion enabled.
- [ ] 5.4 Visual validation: smooth (low roughness) surfaces are more aggressively occluded than rough surfaces for the same geometry.
- [ ] 5.5 Confirm `r.RTXGI.DDGI.SG.SpecularOcclusion 0` disables occlusion (specOcc = 1.0, no change).
- [ ] 5.6 Confirm `r.RTXGI.DDGI.SG.SpecularOcclusionStrength` adjusts the falloff strength.
