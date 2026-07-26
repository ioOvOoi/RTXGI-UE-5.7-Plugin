## Why

After the ASG warp upgrade (`upgrade-sg-specular-to-asg-warps`), the SG specular BRDF lobe is a single ASG approximating the GGX NDF. A single SG/ASG lobe cannot capture the GGX NDF's characteristic narrow peak and long tail — the lobe is either too wide at the peak or too short at the tail. This is most visible at mid-range roughness (0.2–0.5), where the GGX shape has both a distinct peak and significant energy in the tails.

No publicly available, permissively-licensed 3-SG (or multi-SG) GGX NDF fitting exists. The Order 1886 team derived 3-SG coefficients internally but never published them. Wang et al. 2009 and Xu et al. 2013 fitted multi-SG to *measured* BRDF NDFs, not the standard parametric GGX. BakingLab and DXRPathTracer (MJP, MIT) use only the single-SG Wang fit.

This change derives our own 3-SG GGX NDF fit via offline Python non-negative least squares (NNLS) optimization, produces analytical polynomial coefficients as functions of roughness, and embeds them in `SGLighting.ush` as HLSL constants. The fit is applied as 3 co-axial ASG lobes (preserving the ASG anisotropy from the prior change), with each lobe independently warped and convolved with the radiance SG basis.

## What Changes

- Derive 3-SG GGX NDF fitting coefficients offline (Python NNLS):
  - Sample `D_GGX(h) = α²/(π·((α²-1)·cos²θ+1)²)` at 256 angles for roughness α ∈ [0.01, 1.0]
  - Fit 3 co-axial SG lobes: `D(h) ≈ Σ_j A_j(α)·exp(λ_j(α)·(cosθ-1))` with NNLS constraint `A_j ≥ 0`
  - Meta-fit each of the 6 coefficients (A_0, λ_0, A_1, λ_1, A_2, λ_2) as a polynomial of α
  - Output HLSL constant arrays (polynomial coefficients)
- Add `GGX3SGFit(roughness)` function to `SGLighting.ush` that evaluates the 6 analytical coefficients from the polynomial constants (pure ALU, no texture lookup).
- Extend `RTXGISGSpecularIBL` to optionally use 3 ASG lobes:
  - For each of 3 NDF SG lobes j: construct `NDF_j = SG(N, λ_j, A_j)`, warp to `ASG_j` via the ASG warp from the prior change, compute `ASGProductIntegral(ASG_j, radiance_SG_k)` for each k
  - Sum the 3×N contributions
  - Only active in the roughness range `[MultiLobeMin, MultiLobeMax]` (default 0.15–0.6); outside this range, use the single-ASG path from the prior change
- Add CVars:
  - `r.RTXGI.DDGI.SG.Specular.MultiLobe` (int, default `0` = disabled — enable after validating the ASG warp change)
  - `r.RTXGI.DDGI.SG.Specular.MultiLobeRange` (string "min,max", default "0.15,0.6")
- Keep `EnvBRDFApprox`, `horizon`, `specOcc`, and `sgSpecMask` (from other changes) unchanged.

## Capabilities

### New Capabilities

- `sg-specular-multi-lobe-ggx-fit`: 3-lobe ASG GGX NDF approximation for improved mid-range roughness specular shape, with analytical polynomial coefficients derived from offline NNLS fitting.

### Modified Capabilities

- `sg-radiance-ddgi-lighting`: SG rough specular can optionally use 3 co-axial ASG lobes (3× inner-product cost) in a configurable roughness range, falling back to single ASG outside that range.

## Impact

- `Shaders/Private/SDK/SGLighting.ush` — new `GGX3SGFit` function + polynomial constant arrays; `RTXGISGSpecularIBL` gains a 3-lobe branch.
- New CVars: `r.RTXGI.DDGI.SG.Specular.MultiLobe`, `r.RTXGI.DDGI.SG.Specular.MultiLobeRange`.
- Offline artifact: Python fitting script (not shipped, used to generate the polynomial constants).
- Cost: inner product N → 3N (3× ASG∩SG) when enabled and roughness is in range; unchanged otherwise.
- Depends on `upgrade-sg-specular-to-asg-warps` being completed first (uses the ASG warp infrastructure).
- No new SRV bindings (polynomial constants are HLSL compile-time constants).
