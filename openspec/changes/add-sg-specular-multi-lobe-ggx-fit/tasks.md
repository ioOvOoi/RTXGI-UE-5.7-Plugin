## 1. Offline Python fitting (not shipped)

- [ ] 1.1 Write Python script that samples `D_GGX(h) = α²/(π·((α²-1)·cos²θ+1)²)` at 256 angles θ ∈ [0, π/2] for α ∈ [0.01, 1.0] (step 0.01).
- [ ] 1.2 Run `scipy.optimize.nnls` to fit 3 SG lobes `D(h) ≈ Σ_j A_j·exp(λ_j·(cosθ-1))` per roughness value, with `A_j ≥ 0` constraint.
- [ ] 1.3 Meta-fit each of the 6 coefficients (A_0, λ_0, A_1, λ_1, A_2, λ_2) as a 3–4 degree polynomial of α using `numpy.polyfit`.
- [ ] 1.4 Validate the polynomial fit against ground-truth GGX NDF (max relative error < 5% in the active roughness range 0.15–0.6).
- [ ] 1.5 Output HLSL constant arrays (polynomial coefficients) ready to embed in `SGLighting.ush`.

## 2. Add GGX3SGFit and polynomial constants to SGLighting.ush

- [ ] 2.1 Add `static const float` polynomial coefficient arrays for the 6 coefficients (3 amplitudes + 3 sharpnesses).
- [ ] 2.2 Add `GGX3SGFit(roughness)` function that evaluates the 6 polynomials and returns 3 (A, λ) pairs.
- [ ] 2.3 Verify: pure ALU, no texture fetch, no SRV.

## 3. Add 3-lobe branch to RTXGISGSpecularIBL

- [ ] 3.1 Add CVar `r.RTXGI.DDGI.SG.Specular.MultiLobe` (int, default `0`, `ECVF_RenderThreadSafe`) in `DDGIVolumeComponent.cpp`.
- [ ] 3.2 Add CVar `r.RTXGI.DDGI.SG.Specular.MultiLobeRange` (string "min,max", default "0.15,0.6") in `DDGIVolumeComponent.cpp`.
- [ ] 3.3 Pass `MultiLobe` flag and range to the shader parameters struct.
- [ ] 3.4 In `RTXGISGSpecularIBL`, add a branch: if `MultiLobe` enabled and `roughness ∈ [min, max]`, loop over 3 NDF lobes, each warped to ASG and convolved via `ASGProductIntegral`; otherwise use single-ASG path.
- [ ] 3.5 Ensure the 3-lobe and single-lobe paths produce matching results at the range boundaries (no pop).

## 4. Validate

- [ ] 4.1 Build the plugin shader and confirm no compile errors.
- [ ] 4.2 Visual validation: roughness 0.2, 0.35, 0.5 surfaces show improved highlight shape (narrow peak + long tail) vs single-ASG.
- [ ] 4.3 Visual validation: no visible pop when roughness crosses 0.15 or 0.6 boundary.
- [ ] 4.4 Confirm `r.RTXGI.DDGI.SG.Specular.MultiLobe 0` restores single-ASG behavior.
- [ ] 4.5 Confirm no negative-lobe dark spots (NNLS constraint holds).
- [ ] 4.6 Performance check: 3-lobe path in range costs ~3× single-ASG; verify frame budget impact.
