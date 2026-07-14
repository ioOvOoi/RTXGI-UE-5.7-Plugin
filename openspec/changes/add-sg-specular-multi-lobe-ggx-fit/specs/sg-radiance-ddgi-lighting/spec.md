## ADDED Requirements

### Requirement: 3-lobe ASG GGX NDF approximation
The system SHALL optionally evaluate SG rough specular using 3 co-axial ASG lobes whose NDF sharpness and amplitude are fitted to the GGX distribution, improving specular shape accuracy at mid-range roughness.

#### Scenario: 3-lobe path is disabled by default
- **WHEN** `r.RTXGI.DDGI.SG.Specular.MultiLobe` is set to `0` (default)
- **THEN** the renderer uses the single-ASG path from the ASG warp change

#### Scenario: 3-lobe path is enabled
- **WHEN** `r.RTXGI.DDGI.SG.Specular.MultiLobe` is set to `1`
- **AND** the material roughness is within the configured multi-lobe range
- **THEN** the renderer evaluates 3 co-axial ASG lobes, each independently warped and convolved with the radiance SG basis, and sums the 3×N contributions

#### Scenario: Roughness outside the multi-lobe range falls back to single ASG
- **WHEN** the 3-lobe path is enabled but the material roughness is outside `[MultiLobeMin, MultiLobeMax]`
- **THEN** the renderer uses the single-ASG path for that pixel

#### Scenario: Smooth transition at range boundaries
- **WHEN** the material roughness crosses the multi-lobe range boundary
- **THEN** the transition between single-ASG and 3-lobe-ASG does not produce a visible pop or discontinuity

### Requirement: Analytical polynomial NDF fitting coefficients
The system SHALL compute the 3-lobe NDF fitting coefficients (3 amplitudes + 3 sharpnesses) as analytical polynomial functions of roughness, embedded as HLSL compile-time constants.

#### Scenario: Coefficients are computed without texture lookup
- **WHEN** `GGX3SGFit(roughness)` is called in the shader
- **THEN** the 6 coefficients are evaluated from polynomial constants using only ALU operations (no texture fetch, no SRV binding)

#### Scenario: Amplitudes are non-negative
- **WHEN** the 3-lobe NDF fit is evaluated for any valid roughness
- **THEN** all 3 amplitudes `A_j` are non-negative (NNLS-constrained fit)

### Requirement: Configurable multi-lobe roughness range
The system SHALL allow the user to configure the roughness range in which the 3-lobe path is active.

#### Scenario: Default range covers mid-range roughness
- **WHEN** no CVar override is set for the multi-lobe range
- **THEN** the 3-lobe path is active for roughness in `[0.15, 0.6]`

#### Scenario: Custom range is applied
- **WHEN** `r.RTXGI.DDGI.SG.Specular.MultiLobeRange` is set to a custom "min,max" value
- **THEN** the 3-lobe path is active only within that custom roughness range

### Requirement: Offline NNLS fitting derivation
The system's 3-lobe NDF coefficients SHALL be derived from offline non-negative least squares fitting of the GGX NDF, not from any proprietary or copyrighted source.

#### Scenario: Coefficients are derived from public math
- **WHEN** the polynomial constants are generated
- **THEN** they are produced by fitting the public GGX NDF formula `α²/(π·((α²-1)·cos²θ+1)²)` using NNLS, with no third-party copyrighted coefficients
