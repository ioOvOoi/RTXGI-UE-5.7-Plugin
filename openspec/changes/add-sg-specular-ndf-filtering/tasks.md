## 1. Port IsotropicNDFFiltering and add CVar

- [ ] 1.1 Add `IsotropicNDFFiltering(dndu, dndv, roughness)` to `ApplyLightingDeferred.usf`, ported from VSGL `NDFFiltering.hlsli`. Include MIT license attribution.
- [ ] 1.2 Add CVar `r.RTXGI.DDGI.SG.NDFFiltering` (bool, default `1`, `ECVF_RenderThreadSafe`) in `DDGIVolumeComponent.cpp`.
- [ ] 1.3 Read the CVar on the render thread in `RenderDiffuseIndirectLight_RenderThread` and pass it to the shader parameters struct.

## 2. Integrate into MainCS

- [ ] 2.1 After reading `Normal` and `MaterialRoughness` in `MainCS`, add a CVar-guarded block that:
  - Loads `N0 = DecodeNormal(NormalTexture.Load(int3(GBufferNearestPixelIndex, 0)).xyz)` (already loaded, reuse)
  - Loads `N1` at `min(GBufferNearestPixelIndex + int2(1,0), View.ViewSize - 1)`
  - Loads `N2` at `min(GBufferNearestPixelIndex + int2(0,1), View.ViewSize - 1)`
  - Computes `dndu = N1 - N0`, `dndv = N2 - N0`
  - Applies `MaterialRoughness = IsotropicNDFFiltering(dndu, dndv, MaterialRoughness).x`
- [ ] 2.2 Verify the filtered `MaterialRoughness` is passed into all `VOLUME_ENTRY` calls that feed `ApplyVolumeLightingContribution`.

## 3. Validate

- [ ] 3.1 Build the plugin shader and confirm no compile errors.
- [ ] 3.2 Visual validation: high-curvature surface (sphere) shows no specular flickering when camera moves, with CVar enabled.
- [ ] 3.3 Visual validation: flat surface (wall) shows no visible difference with CVar enabled vs disabled (self-adapting no-op).
- [ ] 3.4 Confirm `r.RTXGI.DDGI.SG.NDFFiltering 0` disables filtering and restores raw GBuffer roughness behavior.
- [ ] 3.5 Confirm no out-of-bounds texture loads at screen edges (check with shader debugging or visual inspection at screen borders).
