## Context

The deferred DDGI lighting pass (`FApplyLightingDeferredShaderCS`, `ApplyLightingDeferred.usf`) reads material roughness from GBufferB and passes it to `ApplyVolumeLightingContribution`, which feeds it into SG specular evaluation. On high-curvature surfaces, the interpolated normal changes rapidly across screen pixels, causing the specular lobe to point in different directions for neighboring pixels — this produces aliasing and temporal flickering in SG specular highlights.

Pixel shaders have `ddx`/`ddy` for free, but this pass is a compute shader — derivatives must be computed by manually loading adjacent pixels.

## Goals / Non-Goals

**Goals:**
- Add screen-space NDF filtering to SG specular roughness in the deferred compute pass.
- Make it self-adapting (no-op on flat surfaces, active only where curvature is high).
- Enable by default, with a CVar to disable if double-filtering with material specular AA is visually too aggressive.
- No new SRV bindings — reuse the already-bound `NormalTexture`.

**Non-Goals:**
- Do not filter roughness for UE's own reflection system (this pass only feeds the SG/DDGI path).
- Do not implement anisotropic NDF filtering (isotropic is sufficient for the SG specular quality target).
- Do not change the compute shader thread group size or dispatch dimensions.

## Decisions

### Decision: Port VSGL IsotropicNDFFiltering (MIT)

VSGL's `IsotropicNDFFiltering` is a 5-line function with a self-adapting formula: `kernelRoughness² = (1/2π)·(|dN/du|² + |dN/dv|²)`. When normal derivatives are near zero (flat surfaces), `kernelRoughness² ≈ 0` and no roughness is added. The formula is clamped to a maximum of `KAPPA = 0.18` to prevent over-blurring. This makes it safe to enable by default.

Alternatives considered:
- Custom heuristic threshold: rejected — VSGL formula is peer-reviewed (JCGT 2021) and self-adapting.
- UE material-level specular AA only: rejected — DDGI pass cannot detect whether material AA was applied, and scenes with material AA off are unprotected.

### Decision: Default enabled

The formula is self-adapting: flat surfaces get ≈0 added roughness, so default-on does not degrade flat-surface quality. Artists can disable via CVar if they observe over-blurring on materials that already have specular AA.

### Decision: Manual pixel loads for derivatives

Compute shaders have no `ddx`/`ddy`. Derivatives are computed by loading `GBufferPixel + (1,0)` and `GBufferPixel + (0,1)` from `NormalTexture` (already bound). Cost: 2 extra `Load` calls (unfiltered, cheap) + 2 `DecodeNormal` + 2 subtractions. Pixel coordinates are clamped to `View.ViewSize - 1` to avoid out-of-bounds.

## Risks / Trade-offs

- **Double filtering**: Materials with specular AA enabled in their material graph already filter roughness before writing to GBufferB. NDF filtering adds another pass. Risk: over-blurred highlights on high-curvature surfaces. Mitigation: self-adapting formula + CVar to disable.
- **Edge pixels**: `Load` with out-of-bounds coordinates returns zero in UE's HLSL, which decodes to a non-unit normal. Mitigation: clamp pixel coordinates to view size.
- **Cost on low-end hardware**: 2 extra texture loads per pixel. Mitigation: CVar-off path skips all derivative computation.

## Migration Plan

1. Add `IsotropicNDFFiltering` function to `ApplyLightingDeferred.usf` (ported from VSGL, with MIT attribution).
2. Add CVar `r.RTXGI.DDGI.SG.NDFFiltering` (bool, default `1`) in `DDGIVolumeComponent.cpp`.
3. In `MainCS`, after reading `Normal` and `MaterialRoughness`, add the derivative computation and filtering call, guarded by the CVar.
4. Clamp `GBufferNearestPixelIndex + offset` to `View.ViewSize - 1`.
5. Pass the filtered roughness into the volume loop (replacing `MaterialRoughness` in the `VOLUME_ENTRY` macro calls).

Rollback: `r.RTXGI.DDGI.SG.NDFFiltering 0` restores raw GBuffer roughness.

## Implementation Notes

- VSGL source: `VSGL/Shaders/NDFFiltering.hlsli` (MIT, Copyright (c) Yusuke Tokuyoshi).
- The CVar should be `ECVF_RenderThreadSafe` to match existing SG CVars.
- `GBufferNearestPixelIndex` is already computed in `MainCS` (line 313 of `ApplyLightingDeferred.usf`).
- `View.ViewSizeAndInvSize.xy` gives the view size for clamping.
