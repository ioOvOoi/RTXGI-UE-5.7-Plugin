## 1. Port VSGL helper functions into SGLighting.ush

- [ ] 1.1 Add `GGXDominantVisibleNormal(wi, roughness)` to `SGLighting.ush`, ported from VSGL `GGX.hlsli`. Include the numerically stable `wi.z < 0` branch.
- [ ] 1.2 Add `ASGReflectionLobe(dir, normal, roughness2)` to `SGLighting.ush`, ported from VSGL `AnisotropicSphericalGaussian.hlsli`. Use NDF sharpness `1/roughness2 - 1` and Jacobian-diagonal warp.
- [ ] 1.3 Add `ASGProductIntegral(asg, sg)` to `SGLighting.ush`, ported from VSGL `AnisotropicSphericalGaussian.hlsli`. Use `logAmplitude` form for numerical stability.
- [ ] 1.4 Add VSGL MIT license header comment block above the ported section.

## 2. Rewrite RTXGISGSpecularIBL to use ASG lobe

- [ ] 2.1 Replace the single-SG BRDF lobe construction (`specSharpness = 2/roughness²`, axis `= reflect(-V, N)`) with `ASGReflectionLobe(V, N, roughness²)`.
- [ ] 2.2 Replace the inner-product loop from SG∩SG closed form to `ASGProductIntegral(ASG_lobe, radiance_SG_lobe_k)` for each k in 0..N.
- [ ] 2.3 Keep `EnvBRDFApprox(F0, roughness, NoV)` DF term unchanged.
- [ ] 2.4 Update `horizon` term to use the ASG lobe axis (`BasisZ`) instead of pure reflection vector.
- [ ] 2.5 Verify: no new shader parameters or CVars introduced.

## 3. Validate

- [ ] 3.1 Build the plugin shader and confirm no compile errors.
- [ ] 3.2 Visual validation: grazing-angle surface (floor/wall) at roughness 0.3–0.7 shows stretched highlight, not circular.
- [ ] 3.3 Visual validation: high-roughness surface viewed at angle shows lobe center shifted toward view, not at pure reflection.
- [ ] 3.4 Visual validation: `LightingMode=3` (specular debug) shows the new ASG shape.
- [ ] 3.5 Confirm `LightingMode=0` (octa fallback) is unaffected.
- [ ] 3.6 Confirm SG diffuse path (`RTXGISGDiffuseIrradiance`) is unaffected.
