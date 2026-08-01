## Why

DDGI 的辐照度 gather 内置的 Chebyshev visibility 是方差概率估计，给的是软遮挡，对中远距离大面积天光遮蔽（大厅远墙变暗、天花板下变暗）分辨率不足，且 SDF 方案（DFAO）在 128³ 体素上限下产生斑点噪点。需要用 DDGI 已有的探针 distance texture 算 sky visibility，无额外追踪成本、无 SDF 体素噪点，替代 DFAO 做大面积天光遮挡。

## What Changes

- 新增 `DDGIGetVolumeSkyVisibility` HLSL 函数到 `Irradiance.ush`：8 邻域探针 × N=8 方向半球均匀采样 distance texture，`distance > 1e10f` = 天空可见，三线性插值得 sky visibility
- `ApplyLightingDeferred.usf` 两条辐照度路径（octa + SG）在返回前统一乘 GBufferAO（全局 Intensity）
- 新增 `FDDGISkyVisibilityViewExtension` + `UDDGISkyVisibilitySubsystem`：低分辨率算 skyVis → 中间 RT → 全分辨率 float4 RMW 写入 **GBufferC.a**（min）
- 弱化 Chebyshev floor：`r.RTXGI.DDGI.ChebyshevFloor` 默认 0.5（探针更新保持 0.05）
- CVars：SkyVisibility / Intensity（全局，reader）/ ResolutionScale / SampleCount
- per-volume `SkyVisibilityIntensity` 在 `FComponentData`（仅 writer）
- **BREAKING**：关闭 `Allow Static Lighting` 以释放 GBufferC.a 作为 GBufferAO

## Capabilities

### New Capabilities
- `sky-visibility-occlusion`: 从 DDGI 探针 distance texture 计算 sky visibility，写入 GBufferAO 影响 SkyLight，同时乘到 DDGI 辐照度

### Modified Capabilities
<!-- 无现有 openspec/specs，留空 -->

## Impact

- **Shaders**：`Irradiance.ush`、`ApplyLightingDeferred.usf`、新 SkyVisibility CS
- **C++**：`DDGIVolumeComponent.cpp`、`DDGISkyVisibilityViewExtension`、`DDGISkyVisibilitySubsystem`
- **引擎依赖**：`FWorldSceneViewExtension`、`UWorldSubsystem`、GBufferC.a（Allow Static Lighting Off）
- **运行时**：4 CVar + 1 per-volume 属性；零额外 RT 追踪
