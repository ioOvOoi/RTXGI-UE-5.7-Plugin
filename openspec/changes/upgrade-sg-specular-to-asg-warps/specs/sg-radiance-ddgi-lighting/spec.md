## MODIFIED Requirements

### Requirement: ASG reflection lobe for SG rough specular
The system SHALL evaluate SG rough specular using an anisotropic spherical Gaussian (ASG) reflection lobe constructed from a dominant visible normal, instead of an isotropic SG centered at the pure reflection vector.

#### Scenario: Grazing-angle highlight stretches along view direction
- **WHEN** a surface is viewed at a grazing angle and SG rough specular is evaluated
- **THEN** the specular highlight is stretched anisotropically along the view direction rather than appearing circular

#### Scenario: Lobe center follows visible-normal distribution
- **WHEN** material roughness is high and the surface is viewed at an angle
- **THEN** the SG specular lobe center is shifted toward the view direction according to the GGX dominant visible normal, rather than remaining at the pure reflection vector

#### Scenario: NDF sharpness uses Tokuyoshi-Harada formula
- **WHEN** the ASG reflection lobe is constructed for a given roughness
- **THEN** the NDF sharpness is computed as `1/α² - 1` where `α = roughness²`, instead of the Wang formula `2/α²`

### Requirement: ASG-SG product integral for radiance evaluation
The system SHALL compute SG specular radiance using the ASG∩SG product integral (anisotropic convolution) instead of the isotropic SG∩SG inner product.

#### Scenario: Each radiance SG lobe is convolved with the ASG BRDF lobe
- **WHEN** SG specular radiance is accumulated over N radiance SG lobes
- **THEN** each radiance lobe is evaluated via `ASGProductIntegral(ASG_BRDF_lobe, radiance_SG_lobe)` using a numerically stable logAmplitude form

#### Scenario: Radiance basis sharpness is unchanged
- **WHEN** the ASG product integral consumes the radiance SG lobe sharpness
- **THEN** the radiance basis sharpness `RTXGI_SG_UNIFORM_SHARPNESS` remains the shared constant for the SG amplitude atlas

### Requirement: VSGL MIT license attribution
The system SHALL include MIT license attribution for ported VSGL code in the shader source files.

#### Scenario: Ported shader sections carry copyright notice
- **WHEN** VSGL-derived functions are added to `SGLighting.ush`
- **THEN** the source includes the VSGL MIT copyright notice (Copyright (c) Yusuke Tokuyoshi) and permission notice
