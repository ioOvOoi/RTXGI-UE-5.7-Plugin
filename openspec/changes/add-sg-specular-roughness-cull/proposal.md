## Why

The RTXGI deferred lighting pass outputs SG specular via `+=` accumulation into `SceneColor`. UE's own reflection system (ReflectionEnvironment / ReflectionCapture cubemaps) also writes specular into `SceneColor`. When both are active, smooth/metallic surfaces receive **double specular** — the SG specular highlight叠加 the cubemap reflection, producing over-bright artifacts.

The Order 1886 solved this by clear division of labor: SG specular handles **rough surfaces** (diffuse specularity), cubemaps handle **smooth surfaces** (sharp reflections). This change applies the same division in the RTXGI plugin: SG specular is attenuated at low roughness, yielding to UE's cubemap reflections. No cubemap reading is required from the plugin side.

Route B (RTXGI reading UE's ReflectionCapture cubemap atlas and internally blending by roughness) was considered but rejected for this change because it requires engine-internal texture access, additional SRV bindings, and probe selection logic that conflicts with the SRV-pressure constraint. A TODO comment will mark the integration point for future Route B work.

## What Changes

- In `RTXGISGSpecularIBL`, multiply the final result by a roughness-based mask:
  ```
  float sgSpecMask = saturate((roughness - roughnessCullMin) / (roughnessCullMax - roughnessCullMin));
  return radiance * brdf * horizon * specOcc * sgSpecMask;
  ```
  - `roughness < roughnessCullMin`: SG specular = 0 (cubemap handles smooth reflections)
  - `roughnessCullMin ≤ roughness ≤ roughnessCullMax`: smooth transition
  - `roughness > roughnessCullMax`: SG specular at full strength (rough surfaces, cubemap can't help)
- Add CVars:
  - `r.RTXGI.DDGI.SG.Specular.RoughnessCullMin` (float, default `0.2`)
  - `r.RTXGI.DDGI.SG.Specular.RoughnessCullMax` (float, default `0.5`)
- Add a `TODO` comment at the mask application point documenting Route B (cubemap internal blend) as a future improvement.

## Capabilities

### New Capabilities

- `sg-specular-roughness-cull`: Roughness-based attenuation of SG specular at low roughness to avoid double-specular with UE's cubemap reflection system.

### Modified Capabilities

- `sg-radiance-ddgi-lighting`: SG rough specular is now attenuated below a configurable roughness range, yielding to UE's ReflectionCapture cubemaps for smooth-surface reflections.

## Impact

- `Shaders/Private/SDK/SGLighting.ush` — `RTXGISGSpecularIBL` gains a `sgSpecMask` multiplication at the return point.
- New CVars: `r.RTXGI.DDGI.SG.Specular.RoughnessCullMin` (default `0.2`), `r.RTXGI.DDGI.SG.Specular.RoughnessCullMax` (default `0.5`).
- Cost: ~3 ALU instructions (2 sub + 1 div + 1 saturate).
- No new SRV bindings.
- Visual: smooth/metallic surfaces (roughness < 0.2) lose SG specular; rough surfaces (roughness > 0.5) are unaffected.
- If UE ReflectionCapture is not placed in the scene, smooth surfaces will have no indirect specular at all — this is expected and documented (Route B would fix it by reading cubemaps internally).
- Orthogonal to `specOcc` from `add-sg-specular-occlusion` — both multiply the final result.
