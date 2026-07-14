## Why

The current SG rough specular in `RTXGISGSpecularIBL` uses a single isotropic SG lobe (`specSharpness = 2/roughness²`) centered at `reflect(-V, N)`. This has two known deficiencies at grazing angles:

1. **No anisotropic stretching** — a real GGX BRDF lobe stretches along the view direction at grazing angles, but an isotropic SG produces a round highlight. This is visible as incorrect highlight shape on floors and walls viewed at low angles.
2. **Lobe center does not follow VNDF** — the lobe is centered at the pure reflection vector `reflect(-V, N)`, but the true GGX visible-normal distribution shifts its peak toward the view direction as roughness increases. The isotropic SG misses this shift.

The VSGL repository (Yusuke Tokuyoshi, MIT license) provides battle-tested, numerically stable ASG reflection-lobe construction and ASG∩SG product-integral code that addresses both issues. This change ports that code into `SGLighting.ush`, replacing the single-SG BRDF lobe with a single-ASG lobe using `GGXDominantVisibleNormal` for the reflection axis and the Tokuyoshi–Harada NDF sharpness formula.

## What Changes

- Port the following from VSGL (MIT) into `SGLighting.ush`:
  - `GGXDominantVisibleNormal(wi, roughness)` from `GGX.hlsli`
  - `SGReflectionLobe` / `ASGReflectionLobe` and `ASGProductIntegral` from `SphericalGaussian.hlsli` / `AnisotropicSphericalGaussian.hlsli`
  - Numerically stable `logAmplitude` form for the ASG product integral
- Replace the BRDF lobe construction inside `RTXGISGSpecularIBL`:
  - NDF sharpness: `2/α²` → `1/α² - 1` (Tokuyoshi–Harada 2019)
  - Reflection axis: `reflect(-V, N)` → `reflect(-wi, GGXDominantVisibleNormal(wi, roughness))`
  - Lobe shape: isotropic SG → ASG (anisotropic stretching via Jacobian diagonal)
  - Inner product: `SG∩SG` closed form → `ASGProductIntegral` (ASG∩SG)
- Keep the existing `EnvBRDFApprox` split-sum DF term, `horizon` term, and the overall `radiance * brdf * horizon` structure unchanged.
- Keep SG diffuse (`RTXGISGDiffuseIrradiance`) unchanged — it does not need anisotropy.
- Preserve all existing SG lighting modes (0–5), CVars, and volume panel controls.
- Include VSGL MIT license attribution in the ported shader sections.

## Capabilities

### Modified Capabilities

- `sg-radiance-ddgi-lighting`: SG rough specular BRDF lobe construction upgraded from single isotropic SG to single ASG with dominant-visible-normal axis and Tokuyoshi NDF sharpness. Inner product upgraded from SG∩SG to ASG∩SG. Grazing-angle highlight shape and lobe-center accuracy improved.

## Impact

- `Shaders/Private/SDK/SGLighting.ush` — `RTXGISGSpecularIBL` rewritten; new helper functions added (`GGXDominantVisibleNormal`, `ASGReflectionLobe`, `ASGProductIntegral`).
- No new CVars (this is a quality replacement of the existing specular path).
- No new SRV bindings (ASG construction is pure ALU from existing view/normal/roughness).
- Visual output of SG `LightingMode=2` and `LightingMode=3` will change (improved grazing-angle shape).
- No impact on SG diffuse, octa fallback, probe projection, or bake assets.
