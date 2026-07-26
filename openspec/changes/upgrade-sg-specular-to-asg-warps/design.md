## Context

`RTXGISGSpecularIBL` currently approximates the GGX BRDF lobe as a single isotropic SG centered at `reflect(-V, N)` with sharpness `2/α²`. At grazing angles the real GGX lobe (1) stretches anisotropically along the view direction and (2) shifts its peak toward the view direction due to the visible-normal distribution. The isotropic SG captures neither effect, producing round highlights and incorrect peak positions on low-angle surfaces.

## Goals / Non-Goals

**Goals:**
- Replace the isotropic SG BRDF lobe with an anisotropic ASG lobe.
- Use `GGXDominantVisibleNormal` for the reflection axis so the lobe center follows the VNDF.
- Use the Tokuyoshi–Harada NDF sharpness `1/α² - 1` instead of the Wang sharpness `2/α²`.
- Use the numerically stable `logAmplitude` form for the ASG∩SG product integral.
- Preserve the existing split-sum `EnvBRDFApprox` DF term and `horizon` term.
- Keep SG diffuse unchanged.

**Non-Goals:**
- Do not add multi-lobe (3-SG) NDF fitting (separate change `add-sg-specular-multi-lobe-ggx-fit`).
- Do not add specular occlusion (separate change `add-sg-specular-occlusion`).
- Do not add NDF filtering / specular AA (separate change `add-sg-specular-ndf-filtering`).
- Do not add roughness culling for cubemap fallback (separate change `add-sg-specular-roughness-cull`).
- Do not change the SG radiance basis (Fibonacci distribution, shared sharpness) or probe projection.

## Decisions

### Decision: Port VSGL ASG code (MIT) instead of MJP ASG warp

VSGL's `ASGReflectionLobe` + `ASGProductIntegral` + `GGXDominantVisibleNormal` are MIT-licensed and more accurate than MJP's `WarpDistributionASG` because:
- VSGL uses `GGXDominantVisibleNormal` for the reflection axis (lobe center follows VNDF); MJP uses pure `reflect(-view, N)`.
- VSGL uses NDF sharpness `1/α² - 1` (Tokuyoshi–Harada 2019); MJP uses `2/α²` (Wang 2009).
- VSGL uses `logAmplitude` for numerical stability at high sharpness; MJP uses direct amplitude.

Alternatives considered:
- MJP `WarpDistributionASG` (DXRPathTracer, MIT): rejected — misses VNDF shift, less stable.
- Single-SG Wang fit (status quo): rejected — no anisotropy, no VNDF shift.

### Decision: Keep EnvBRDFApprox split-sum DF term

The ASG upgrade changes the prefiltered-radiance half of the split-sum (D term). The `EnvBRDFApprox(F0, roughness, NoV)` term (G×F/(4·NoV·Ni) approximation) remains unchanged. This keeps the change focused on lobe shape.

### Decision: No new CVars

This is a quality replacement of the existing specular path. The existing `SGLightingMode` CVar (mode 0 = octa fallback, mode 2 = SG diffuse + specular, mode 3 = specular debug) already controls whether SG specular is active. No new toggle is needed — if the ASG path regresses, `SGLightingMode=0` falls back to octa.

## Risks / Trade-offs

- **ALU cost increase**: ASG∩SG product integral is slightly more expensive than SG∩SG (extra `rsqrt` + 2D basis evaluation). Estimated ~2× per-lobe ALU, but lobe count N is unchanged. Acceptable for a quality upgrade.
- **Visual regression**: The ASG path will change highlight shape on all surfaces. Visual validation against the previous single-SG path is required. Rollback = `SGLightingMode=0` or revert shader.
- **License compliance**: VSGL is MIT. Must include copyright notice in ported code. MIT permits commercial use with attribution.
- **Numerical edge cases**: `GGXDominantVisibleNormal` has a branch for `wi.z < 0` (below-surface view). Must port the numerically stable form, not a naive version.

## Migration Plan

1. Add `GGXDominantVisibleNormal` to `SGLighting.ush` (ported from VSGL `GGX.hlsli`).
2. Add `ASGReflectionLobe` and `ASGProductIntegral` to `SGLighting.ush` (ported from VSGL `AnisotropicSphericalGaussian.hlsli` + `SphericalGaussian.hlsli`).
3. Rewrite `RTXGISGSpecularIBL` to use `ASGReflectionLobe` for lobe construction and `ASGProductIntegral` for the inner-product loop.
4. Keep the `for k in 0..N` loop structure — only the per-iteration inner product changes from SG∩SG to ASG∩SG.
5. Validate against scenes with grazing-angle surfaces (floors, walls) at roughness 0.3–0.7.
6. Include VSGL MIT license header in the ported section.

Rollback strategy:
- `r.RTXGI.DDGI.SG.LightingMode 0` returns to octa diffuse fallback (no SG specular).
- Revert `SGLighting.ush` to restore the previous single-SG path.

## Implementation Notes

- VSGL source files (MIT, Copyright (c) Yusuke Tokuyoshi):
  - `VSGL/Shaders/GGX.hlsli` — `GGXDominantVisibleNormal`
  - `VSGL/Shaders/SphericalGaussian.hlsli` — `SGReflectionLobe`, `SGProduct`, `SGApproxProductIntegral`
  - `VSGL/Shaders/AnisotropicSphericalGaussian.hlsli` — `ASGReflectionLobe`, `ASGProductIntegral`, `ASGEvaluate`
- The existing `RTXGI_SG_UNIFORM_SHARPNESS = 3.0` (radiance basis sharpness) is consumed by `ASGProductIntegral` as the SG sharpness input — no change to the radiance basis.
- `GGXDominantVisibleNormal` needs `wi` (view direction toward surface) and `roughness` (perceptual → α = roughness²). Both are already available in `RTXGISGSpecularIBL`.

## Open Questions

- Should the `horizon = saturate(dot(N, R))` term use `R` from the ASG axis or from the pure reflection vector? (Likely ASG axis, since that is where the lobe is centered.)
