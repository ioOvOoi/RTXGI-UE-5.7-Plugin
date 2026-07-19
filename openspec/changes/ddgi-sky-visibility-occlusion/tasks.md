 1: ## 1. CVar 和 per-volume 属性基础设施
 2: 
 3: - [x] 1.1 在 `DDGIVolumeComponent.cpp` 新增 SkyVisibility / Intensity / ResolutionScale / SampleCount / ChebyshevFloor 5 个 CVar
 4: - [x] 1.2 在 `FComponentData` 新增 `SkyVisibilityIntensity`（默认 1.0），Details panel + proxy 同步（仅供 ViewExtension；不进入 ApplyLighting `VOLUME_ENTRY`）
 5: - [x] 1.3 在 `FApplyLightingDeferredShaderParameters` 新增 `SkyVisibilityEnable`、`SkyVisibilityIntensity`（全局）、`ChebyshevFloor`；移除误加到 apply 路径 `FVolumeData` 的 per-volume intensity；修正缩进
 6: 
 7: ## 2. shader 共享函数 DDGIGetVolumeSkyVisibility
 8: 
 9: - [x] 2.1 在 `Irradiance.ush` 新增 `DDGIGetVolumeSkyVisibility(...)`：8 邻域 × N 半球方向，`distance > 1e10f` = 天空，三线性归一化 [0,1]
10: - [x] 2.2 支持 classification + scrolling permutation，跳过 inactive probe
11: 
12: ## 3. DDGI pass 乘 sky visibility 到辐照度输出
13: 
14: - [x] 3.1 在 `ApplyLightingDeferred.usf` 声明 `SkyVisibilityEnable`、`SkyVisibilityIntensity`（全局）、`ChebyshevFloor`
15: - [x] 3.2 `return LightWeight` 前：读 **GBufferC.a**，`LightWeight.rgb *= lerp(1.0, gBufferAO, SkyVisibilityIntensity)`
16: - [x] 3.3 C++ `RenderDiffuseIndirectLight_RenderThread` 绑定上述参数
17: 
18: ## 4. Chebyshev floor 参数化
19: 
20: - [x] 4.1 `DDGIGetVolumeIrradiance` 增加 `ChebyshevFloor`；替换 hardcoded 0.05
21: - [x] 4.2 ApplyLightingDeferred SG 路径改用 uniform
22: - [x] 4.3 两处 `DDGIGetVolumeIrradiance` 传入 `ChebyshevFloor`
23: - [x] 4.4 `ProbeUpdateRGS.usf` 传固定 0.05
24: 
25: ## 5. SceneViewExtension compute pass
26: 
27: - [x] 5.1 `DDGISkyVisibilityViewExtension.h/.cpp`：`FWorldSceneViewExtension`，`PostRenderBasePassDeferred_RenderThread`
28: - [x] 5.2 `DDGISkyVisibilitySubsystem.h/.cpp`：`UWorldSubsystem`，`NewExtension` / `Reset`
29: - [x] 5.3 按 design D4 策略 (A)：低分辨率算 skyVis → 中间 RT；全分辨率对 **GBufferC** float4 RMW 只改 `.a` 做 min；密度排序 + per-volume intensity；跳过 Unlit；禁止同资源 SRV+UAV
30: - [x] 5.4 注册 RDG pass；绑定 GBufferC UAV + distance/offsets/states；CVar off 时尽早 return
31: - [x] 5.5 模块/Build 保证 subsystem 可注册
32: 
33: ## 6. 编译和验证
34: 
35: - [ ] 6.1 worktree 编译插件，shader 无报错
36: - [ ] 6.2 `SkyVisibility 0` → 辐照度不变
37: - [ ] 6.3 `SkyVisibility 1` + Allow Static Lighting Off → 大厅天光大面积变暗，无 DFAO 斑点
38: - [ ] 6.4 ChebyshevFloor 0.05 / 0.5 / 0.8
39: - [ ] 6.5 SampleCount 1 vs 8
40: - [ ] 6.6 ResolutionScale 0.25 / 0.5 / 1.0
