## Context

The ASG warp change (`upgrade-sg-specular-to-asg-warps`) replaces the single isotropic SG BRDF lobe with a single ASG lobe using `GGXDominantVisibleNormal`. This fixes anisotropic stretching and lobe-center shift, but the NDF is still approximated by a single SG — which cannot match the GGX NDF's narrow peak + long tail shape. The error is most visible at mid-range roughness (0.2–0.5).

No publicly available 3-SG GGX NDF fit exists under a permissive license. This change derives our own fit and embeds it as analytical polynomial constants.

## Goals / Non-Goals

**Goals:**
- Derive 3-SG GGX NDF fitting coefficients offline (Python NNLS).
- Embed the coefficients as HLSL polynomial constants (no texture lookup, no SRV).
- Use 3 co-axial ASG lobes (preserving anisotropy from the ASG warp change).
- Constrain amplitudes to be non-negative (NNLS) to avoid negative-lobe dark spots.
- Only activate in a configurable roughness range; fall back to single ASG outside.
- Default disabled until the ASG warp change is visually validated.

**Non-Goals:**
- Do not derive an anisotropic (ASG) NDF fit — the 3-SG fit is for the isotropic NDF D(h); anisotropy comes from the ASG warp applied per-lobe at runtime.
- Do not fit the geometry (G) or Fresnel (F) terms — those remain `EnvBRDFApprox`.
- Do not ship the Python fitting script as a runtime artifact.
- Do not use a LUT texture — analytical polynomial only (SRV pressure constraint).

## Decisions

### Decision: 3 co-axial ASG lobes (not 3 isotropic SG)

Each of the 3 NDF SG lobes is independently warped to an ASG lobe at runtime, preserving the anisotropic stretching from the ASG warp change. Cost: 3N × ASG∩SG instead of 3N × SG∩SG, but ASG∩SG is only ~2× the ALU of SG∩SG, and the 3-lobe path is only active in a roughness sub-range.

Alternatives considered:
- 3 isotropic SG (cheaper, 3N × SG∩SG): rejected — loses grazing-angle anisotropy gained in the ASG warp change.
- 3 isotropic SG + dominantNormal (cheaper, keeps center shift, loses stretch): rejected — same reason.

### Decision: Analytical polynomial coefficients (not LUT)

The 6 coefficients (A_0, λ_0, A_1, λ_1, A_2, λ_2) are smooth functions of roughness. A 3–4 degree polynomial per coefficient provides sufficient accuracy. Polynomial constants are HLSL compile-time values — no texture, no SRV, no runtime parameter passing.

Alternatives considered:
- 1D LUT texture (16–32 entries): rejected — adds SRV pressure, conflicts with design.md constraint.
- Per-roughness table in a constant buffer: rejected — polynomial is cheaper and sufficient.

### Decision: NNLS constraint (A_j ≥ 0)

Non-negative least squares prevents negative amplitudes, which produce dark spots (negative lobes) in the specular highlight — the same artifact that motivated NNLS in MJP's BakingLab radiance fitting. The cost is a slightly "blurrier" fit (non-negative lobes can't cancel overshoot), but the visual result is cleaner.

### Decision: Roughness-range-gated activation

The 3-lobe path is only active when `roughness ∈ [MultiLobeMin, MultiLobeMax]` (default 0.15–0.6). Outside this range:
- Low roughness (< 0.15): NDF is already very narrow; single ASG is sufficient.
- High roughness (> 0.6): NDF is already very wide; single ASG is sufficient.

This avoids paying 3× cost across the full roughness range when the improvement is only visible in the mid-range.

### Decision: Default disabled

The 3-lobe path is gated by `r.RTXGI.DDGI.SG.Specular.MultiLobe` (default `0`). It should only be enabled after the ASG warp change (`upgrade-sg-specular-to-asg-warps`) is validated, to isolate which change provides which visual improvement.

## Risks / Trade-offs

- **3× inner-product cost**: In the active roughness range, the per-pixel specular cost triples. Mitigation: range-gating limits this to mid-roughness pixels only; CVar can disable.
- **Polynomial accuracy**: If the polynomial fit is insufficient at certain roughness values, the NDF approximation may have local errors. Mitigation: validate the polynomial against the ground-truth GGX NDF in the Python script before embedding.
- **NNLS blurriness**: Non-negative constraint makes the fit slightly blurrier than unconstrained least squares. Mitigation: 3 lobes provide enough degrees of freedom to capture both peak and tail even with the constraint.
- **Coupling with ASG warp**: This change depends on `upgrade-sg-specular-to-asg-warps` being completed. If the ASG warp is reverted, the 3-lobe path must also be disabled.

## Migration Plan

1. **Offline (Python, not shipped):**
   - Sample GGX NDF at 256 angles for roughness α ∈ [0.01, 1.0] (step 0.01).
   - Run NNLS fit for 3 SG lobes per roughness value.
   - Meta-fit each of the 6 coefficients as a 3–4 degree polynomial of α.
   - Output HLSL constant arrays.
2. **Shader:**
   - Add polynomial constant arrays to `SGLighting.ush`.
   - Add `GGX3SGFit(roughness)` function returning 3 (A, λ) pairs.
   - Add a 3-lobe branch in `RTXGISGSpecularIBL`, gated by CVar and roughness range.
3. **CVar:**
   - Add `r.RTXGI.DDGI.SG.Specular.MultiLobe` (default `0`).
   - Add `r.RTXGI.DDGI.SG.Specular.MultiLobeRange` (default "0.15,0.6").
4. **Validate:**
   - Compare 3-lobe vs single-ASG at roughness 0.2, 0.35, 0.5 against GGX ground truth.
   - Confirm smooth transition at the roughness range boundaries (no pop).

Rollback: `r.RTXGI.DDGI.SG.Specular.MultiLobe 0` restores single-ASG behavior.

## Implementation Notes

- The Python fitting script uses `scipy.optimize.nnls` for the per-roughness fit and `numpy.polyfit` for the polynomial meta-fit.
- GGX NDF: `D(h) = α² / (π · ((α²-1)·cos²θ + 1)²)` where `α = roughness²`.
- The 3 lobes should be ordered by sharpness: `λ_0 > λ_1 > λ_2` (narrow peak → mid → wide tail).
- The polynomial constants are embedded as `static const float` arrays in `SGLighting.ush`.
- The `GGX3SGFit` function evaluates 6 polynomials (6 × `mad` chains for degree-3), then returns the 3 (A, λ) pairs.
