#!/usr/bin/env python3
"""
Fit GGX NDF as sum of 3 SGs with FIXED sharpness ratios.
Only amplitudes are fitted (linear NNLS), sharpness ratios are fixed constants.
This avoids the polynomial fitting problem — only 3 amplitude polynomials are needed.
"""
import numpy as np
from scipy.optimize import nnls

def ggx_ndf(cos_theta, alpha):
    a2 = alpha * alpha
    return a2 / (np.pi * ((a2 - 1.0) * cos_theta**2 + 1.0)**2)

def fit_amplitudes(alpha, sharpness_multipliers=(4.0, 1.0, 0.25), n_samples=512):
    """Fix sharpness = base * multiplier, solve for amplitudes via NNLS."""
    cos_theta = np.linspace(0.001, 1.0, n_samples)
    target = ggx_ndf(cos_theta, alpha)
    base = max(1.0 / (alpha * alpha) - 1.0, 2.0)
    lambdas = [base * m for m in sharpness_multipliers]
    # Clamp minimum sharpness to 2.0
    lambdas = [max(l, 2.0) for l in lambdas]
    # Build design matrix
    A_design = np.column_stack([np.exp(l * (cos_theta - 1.0)) for l in lambdas])
    amplitudes, _ = nnls(A_design, target)
    # Compute error
    approx = A_design @ amplitudes
    max_rel_err = np.max(np.abs(approx - target) / (target + 1e-10))
    return amplitudes, lambdas, max_rel_err

def poly_eval(coeffs, x):
    result = 0.0
    for c in coeffs:
        result = result * x + c
    return result

def main():
    roughness_range = np.arange(0.15, 0.61, 0.01)
    results = []
    
    print("Rough | A0       | A1       | A2       | l0       | l1       | l2       | MaxErr")
    print("-" * 90)
    
    for r in roughness_range:
        alpha = r * r
        amps, lams, err = fit_amplitudes(alpha)
        results.append({'r': r, 'alpha': alpha, 'A': amps, 'L': lams, 'err': err})
        print(f"{r:5.2f} | {amps[0]:8.4f} | {amps[1]:8.4f} | {amps[2]:8.4f} | {lams[0]:8.2f} | {lams[1]:8.2f} | {lams[2]:8.2f} | {err:.4f}")
    
    # Fit amplitude polynomials (degree 3 in roughness)
    rs = np.array([r['r'] for r in results])
    polyA = []
    for j in range(3):
        A_vals = np.array([r['A'][j] for r in results])
        polyA.append(np.polyfit(rs, A_vals, 3))
    
    # Verify
    max_poly_err = 0
    for r in results:
        for j in range(3):
            p = poly_eval(polyA[j], r['r'])
            max_poly_err = max(max_poly_err, abs(p - r['A'][j]))
    
    max_raw = max(r['err'] for r in results)
    print(f"\n// HLSL: 3-SG GGX NDF fit (fixed sharpness ratios 4:1:0.25)")
    print(f"// Active range: roughness [0.15, 0.60]")
    print(f"// Max raw 3-SG vs GGX NDF relative error: {max_raw:.4f}")
    print(f"// Max polynomial vs raw amplitude error: {max_poly_err:.6f}")
    print()
    
    for j in range(3):
        c = polyA[j]
        print(f"// A{j}(roughness) = {c[0]:.8e}*r^3 + {c[1]:.8e}*r^2 + {c[2]:.8e}*r + {c[3]:.8e}")
        print(f"static const float GGX3SGPoly_A{j}[4] = {{ {c[0]:.8e}f, {c[1]:.8e}f, {c[2]:.8e}f, {c[3]:.8e}f }};")
    print()
    print(f"// Sharpness multipliers (fixed, applied to base NDF sharpness 1/alpha^2 - 1)")
    print(f"static const float3 GGX3SGSharpnessMul = float3(4.0f, 1.0f, 0.25f);")

if __name__ == "__main__":
    main()