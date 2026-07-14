## Why

The deferred DDGI lighting pass runs as a compute shader (`FApplyLightingDeferredShaderCS`). On high-curvature surfaces (spheres, cylinders, character faces), the interpolated normal changes rapidly in screen space, causing SG specular highlights to flicker and alias as the camera moves. UE's material-level specular AA may or may not have already filtered the roughness stored in GBufferB — the DDGI pass has no way to know, and in scenes where material specular AA is off, the SG specular path is unprotected.

The VSGL repository (Yusuke Tokuyoshi, MIT) provides `IsotropicNDFFiltering` — a compact, self-adapting formula that adds roughness proportional to screen-space normal variance. The formula is a no-op on flat surfaces (where normal derivatives are near zero) and only adds roughness where curvature is high, making it safe to enable by default.

## What Changes

- Port `IsotropicNDFFiltering(dndu, dndv, roughness)` from VSGL `NDFFiltering.hlsli` (MIT) into `ApplyLightingDeferred.usf`.
- In `MainCS`, before the volume loop, compute screen-space normal derivatives by manually loading adjacent pixels (compute shader has no `ddx`/`ddy`):
  - `N0 = DecodeNormal(NormalTexture.Load(GBufferPixel))`
  - `N1 = DecodeNormal(NormalTexture.Load(GBufferPixel + (1,0)))`
  - `N2 = DecodeNormal(NormalTexture.Load(GBufferPixel + (0,1)))`
  - `dndu = N1 - N0`, `dndv = N2 - N0`
  - Clamp pixel coordinates to view size to avoid out-of-bounds loads.
- Apply `MaterialRoughness = IsotropicNDFFiltering(dndu, dndv, MaterialRoughness).x` before passing roughness to `ApplyVolumeLightingContribution`.
- Add CVar `r.RTXGI.DDGI.SG.NDFFiltering` (default `1` = enabled). When disabled, skip the derivative computation and use raw GBuffer roughness.
- Include VSGL MIT license attribution in the ported section.

## Capabilities

### New Capabilities

- `sg-specular-ndf-filtering`: Screen-space NDF filtering for SG specular anti-aliasing in the deferred DDGI compute shader pass.

### Modified Capabilities

- `sg-radiance-ddgi-lighting`: Material roughness fed to SG specular is now optionally filtered by screen-space normal variance before entering the volume lighting loop.

## Impact

- `Shaders/Private/ApplyLightingDeferred.usf` — `MainCS` gains normal-derivative loads and NDF filtering call before the volume loop.
- New CVar: `r.RTXGI.DDGI.SG.NDFFiltering` (bool, default `1`).
- No new SRV bindings (`NormalTexture` is already bound).
- Cost: 2 extra `NormalTexture.Load` + decode + ~5 ALU instructions when enabled; zero when disabled.
- Edge pixels require coordinate clamping to avoid out-of-bounds texture loads.
- May double-filter roughness on materials that already have specular AA enabled in their material graph — acceptable because the formula is self-adapting (flat surfaces get ≈0 added roughness).
