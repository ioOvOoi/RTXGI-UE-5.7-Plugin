## 1. Add roughness cull CVars

- [ ] 1.1 Add `r.RTXGI.DDGI.SG.Specular.RoughnessCullMin` (float, default `0.2`, range 0–1, `ECVF_RenderThreadSafe`) in `DDGIVolumeComponent.cpp`.
- [ ] 1.2 Add `r.RTXGI.DDGI.SG.Specular.RoughnessCullMax` (float, default `0.5`, range 0–1, `ECVF_RenderThreadSafe`) in `DDGIVolumeComponent.cpp`.
- [ ] 1.3 Read both CVars on the render thread and pass to the shader parameters struct.

## 2. Add sgSpecMask to RTXGISGSpecularIBL

- [ ] 2.1 In `RTXGISGSpecularIBL` (`SGLighting.ush`), add the roughness mask parameters (`roughnessCullMin`, `roughnessCullMax`) to the function signature.
- [ ] 2.2 Compute `sgSpecMask = saturate((roughness - roughnessCullMin) / max(roughnessCullMax - roughnessCullMin, 1e-6))`.
- [ ] 2.3 Multiply the return value: `return radiance * brdf * horizon * specOcc * sgSpecMask;`
- [ ] 2.4 Add `// TODO (Route B): Read UE ReflectionCapture cubemap atlas and blend internally by roughness instead of self-attenuating. Requires engine-internal cubemap SRV binding, probe selection, and roughness-weighted blend. This would provide dynamic GI specular on smooth surfaces instead of yielding entirely to static cubemaps.` comment above the mask.
- [ ] 2.5 Update all call sites to pass the CVar values.

## 3. Validate

- [ ] 3.1 Build the plugin shader and confirm no compile errors.
- [ ] 3.2 Visual validation: smooth metallic surface (roughness < 0.2) with UE ReflectionCapture present shows cubemap reflection only (no SG specular double-bright).
- [ ] 3.3 Visual validation: rough surface (roughness > 0.5) shows SG specular at full strength (unchanged).
- [ ] 3.4 Visual validation: mid-range roughness (0.3) shows smooth transition, no hard line.
- [ ] 3.5 Confirm `RoughnessCullMin=0` and `RoughnessCullMax=0` disables the mask (SG specular at all roughness).
- [ ] 3.6 Confirm TODO comment is present in the shader source.
