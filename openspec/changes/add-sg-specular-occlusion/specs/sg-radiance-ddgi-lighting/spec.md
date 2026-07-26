## ADDED Requirements

### Requirement: Reflection-direction specular occlusion
The system SHALL compute a scalar specular occlusion factor from the DDGI distance field along the reflection direction and apply it to SG rough specular output.

#### Scenario: Occluded reflection direction reduces specular
- **WHEN** the reflection direction R is blocked by nearby geometry (distance field reports short distance along R)
- **THEN** the SG specular output is attenuated by the specular occlusion factor

#### Scenario: Unobstructed reflection direction preserves specular
- **WHEN** the reflection direction R is unobstructed (distance field reports long distance along R)
- **THEN** the specular occlusion factor is approximately 1.0 and SG specular is unaffected

#### Scenario: Roughness modulates occlusion sensitivity
- **WHEN** the material roughness is low (smooth surface)
- **THEN** the occlusion factor has a sharper falloff (higher power), making smooth surfaces more sensitive to occlusion
- **WHEN** the material roughness is high (rough surface)
- **THEN** the occlusion factor has a gentler falloff (lower power), reflecting that a wide lobe is less affected by a single-direction occlusion

### Requirement: Argmax probe for distance field query
The system SHALL use the highest-weight probe from the 8-probe blend loop to query the distance field along the reflection direction.

#### Scenario: Argmax probe is tracked during blend
- **WHEN** the 8-probe blending loop computes weights for each probe
- **THEN** the loop tracks which probe has the highest weight and its index

#### Scenario: Argmax probe distance is queried along R
- **WHEN** the specular occlusion is computed after the blend loop
- **THEN** the renderer queries the argmax probe's distance field texture along the octahedral coordinates of `-R`

### Requirement: Surface-to-probe offset correction
The system SHALL correct the distance field reading by subtracting the surface-to-probe offset projected onto the reflection direction.

#### Scenario: Distance is corrected for probe position
- **WHEN** the mean distance along R is read from the argmax probe's distance field
- **THEN** the renderer subtracts `dot(normalize(probeWorldPos - biasedWorldPosition), R)` from the mean distance and clamps to non-negative

### Requirement: Specular occlusion CVar control
The system SHALL provide console variable control for specular occlusion enable/disable and strength.

#### Scenario: Occlusion is enabled by default
- **WHEN** no CVar override is set
- **THEN** `r.RTXGI.DDGI.SG.SpecularOcclusion` defaults to `1` (enabled)

#### Scenario: Occlusion is disabled
- **WHEN** `r.RTXGI.DDGI.SG.SpecularOcclusion` is set to `0`
- **THEN** the renderer passes `1.0` as the specular occlusion factor (no attenuation)

#### Scenario: Strength multiplier adjusts decay rate
- **WHEN** `r.RTXGI.DDGI.SG.SpecularOcclusionStrength` is set to a value other than `1.0`
- **THEN** the distance decay rate in the occlusion formula is scaled by that value

### Requirement: specOcc parameter in RTXGISGSpecularIBL
The system SHALL accept a scalar specular occlusion factor as a parameter to `RTXGISGSpecularIBL` and multiply it into the final result.

#### Scenario: specOcc modulates the final specular output
- **WHEN** `RTXGISGSpecularIBL` computes `radiance * brdf * horizon`
- **THEN** the result is multiplied by `specOcc` before returning

#### Scenario: specOcc = 1.0 is a no-op
- **WHEN** `specOcc` is `1.0` (occlusion disabled or unobstructed)
- **THEN** the final specular output is unchanged
