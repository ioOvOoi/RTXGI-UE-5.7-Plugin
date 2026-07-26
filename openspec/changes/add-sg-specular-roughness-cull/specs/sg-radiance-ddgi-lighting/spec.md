## ADDED Requirements

### Requirement: Roughness-based SG specular attenuation
The system SHALL attenuate SG rough specular output at low roughness to avoid double-specular accumulation with UE's cubemap reflection system.

#### Scenario: Smooth surfaces yield to cubemap reflections
- **WHEN** the material roughness is below `r.RTXGI.DDGI.SG.Specular.RoughnessCullMin`
- **THEN** the SG specular output is zero, allowing UE's ReflectionCapture cubemaps to handle smooth-surface reflections without double-specular

#### Scenario: Rough surfaces receive full SG specular
- **WHEN** the material roughness is above `r.RTXGI.DDGI.SG.Specular.RoughnessCullMax`
- **THEN** the SG specular output is at full strength (no attenuation)

#### Scenario: Mid-range roughness transitions smoothly
- **WHEN** the material roughness is between `RoughnessCullMin` and `RoughnessCullMax`
- **THEN** the SG specular output transitions smoothly from zero to full strength without a visible pop

### Requirement: Configurable roughness cull range
The system SHALL provide two console variables to configure the roughness attenuation range.

#### Scenario: Default range is 0.2 to 0.5
- **WHEN** no CVar override is set
- **THEN** `RoughnessCullMin` defaults to `0.2` and `RoughnessCullMax` defaults to `0.5`

#### Scenario: Custom range is applied
- **WHEN** the user sets `r.RTXGI.DDGI.SG.Specular.RoughnessCullMin` and `RoughnessCullMax` to custom values
- **THEN** the attenuation range uses the custom values

#### Scenario: Attenuation is disabled
- **WHEN** both `RoughnessCullMin` and `RoughnessCullMax` are set to `0.0`
- **THEN** the SG specular mask is `1.0` everywhere (no attenuation, SG specular at all roughness values)

### Requirement: Route B TODO documentation
The system SHALL document the future Route B (cubemap internal blend) integration point with a TODO comment in the shader source.

#### Scenario: TODO comment marks the cubemap blend integration point
- **WHEN** the `sgSpecMask` is applied in `RTXGISGSpecularIBL`
- **THEN** a `// TODO (Route B):` comment describes the future work of reading UE's ReflectionCapture cubemap atlas and blending internally by roughness
