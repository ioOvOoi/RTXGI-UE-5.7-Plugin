## ADDED Requirements

### Requirement: Screen-space NDF filtering for SG specular
The system SHALL apply isotropic NDF filtering to material roughness before SG specular evaluation in the deferred DDGI compute shader pass, to reduce specular aliasing on high-curvature surfaces.

#### Scenario: Flat surfaces are unaffected
- **WHEN** the screen-space normal derivatives are near zero (flat surface)
- **THEN** the NDF filtering adds approximately zero roughness and the SG specular result is unchanged

#### Scenario: High-curvature surfaces get anti-aliased specular
- **WHEN** the screen-space normal derivatives are significant (high-curvature surface)
- **THEN** the NDF filtering increases the effective roughness, broadening the SG specular highlight and reducing aliasing

#### Scenario: Filtering is disabled via CVar
- **WHEN** `r.RTXGI.DDGI.SG.NDFFiltering` is set to `0`
- **THEN** the renderer skips normal-derivative computation and uses raw GBuffer roughness for SG specular

#### Scenario: Filtering is enabled by default
- **WHEN** no CVar override is set
- **THEN** `r.RTXGI.DDGI.SG.NDFFiltering` defaults to `1` (enabled)

### Requirement: Compute-shader normal derivatives via manual pixel loads
The system SHALL compute screen-space normal derivatives in the compute shader by loading adjacent pixels from the already-bound normal texture, since compute shaders do not have hardware derivative instructions.

#### Scenario: Adjacent pixels are loaded for derivatives
- **WHEN** NDF filtering is enabled and the current pixel is not at the screen edge
- **THEN** the renderer loads `NormalTexture` at `pixel + (1,0)` and `pixel + (0,1)` to compute `dndu` and `dndv`

#### Scenario: Screen-edge pixels are clamped
- **WHEN** the current pixel is at the screen edge
- **THEN** the renderer clamps the adjacent pixel coordinates to the view size to avoid out-of-bounds texture loads

### Requirement: No additional SRV bindings
The system SHALL reuse the already-bound `NormalTexture` for NDF filtering and SHALL NOT add new shader resource views.

#### Scenario: Existing normal texture is reused
- **WHEN** NDF filtering computes normal derivatives
- **THEN** the renderer uses the existing `NormalTexture` binding without adding new SRV slots

### Requirement: VSGL MIT license attribution
The system SHALL include MIT license attribution for the ported `IsotropicNDFFiltering` function.

#### Scenario: Ported function carries copyright notice
- **WHEN** `IsotropicNDFFiltering` is added to `ApplyLightingDeferred.usf`
- **THEN** the source includes the VSGL MIT copyright notice (Copyright (c) Yusuke Tokuyoshi)
