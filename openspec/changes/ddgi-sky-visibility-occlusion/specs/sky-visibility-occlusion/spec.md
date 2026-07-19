## ADDED Requirements

### Requirement: Sky visibility from probe distance texture
系统 SHALL 提供 `DDGIGetVolumeSkyVisibility` HLSL 函数，从 DDGI 探针 distance texture 计算 sky visibility。函数 SHALL 对 8 邻域探针覆盖三线性插值，每个探针沿 N 个半球均匀方向采样 distance texture，`distance` 值大于 1e10f 时判定为天空可见（visibility=1），否则判定为遮挡（visibility=0）。N 的默认值 SHALL 为 8，通过 `r.RTXGI.DDGI.SkyVisibility.SampleCount` CVar 可调（1/4/8/16）。函数 SHALL 返回归一化的 sky visibility 标量 [0,1]。

#### Scenario: 大厅地板朝天顶开放
- **WHEN** 着色点在开阔大厅地面，法线朝天顶，上方无天花板遮挡
- **THEN** 8 邻域探针沿上半球查 distance，多数方向 `distance > 1e10f`，sky visibility 接近 1.0

#### Scenario: 大厅天花板下变暗
- **WHEN** 着色点在天花板下方，法线朝上，上方有近距天花板
- **THEN** sky visibility 接近 0

#### Scenario: 大厅远墙远处变暗
- **WHEN** 着色点在大厅远墙附近，法线水平朝天光开口反方向
- **THEN** 沿上半球积分部分方向有限 distance，sky visibility 降低

#### Scenario: SampleCount CVar 调节方向数
- **WHEN** SampleCount=1 **THEN** 只沿法线方向，墙面判断不准确
- **WHEN** SampleCount=8 **THEN** 半球积分，墙面/斜面判断准确

### Requirement: Sky visibility written to GBufferAO
系统 SHALL 通过 `FDDGISkyVisibilityViewExtension` 在 `PostRenderBasePassDeferred_RenderThread` 计算 sky visibility 并写入 **GBufferC.a（GBufferAO）**。写入语义 SHALL 为 min——对 GBufferC 做 float4 RMW，只改 `.a`。当 `ResolutionScale < 1` 时 SHALL 先写中间低分辨率 RT，再经全分辨率 pass upsample/`min` 写回；SHALL NOT 用半分辨率 dispatch 直接 scatter-write 全分辨率 GBuffer。Allow Static Lighting SHALL 为 Off。跳过 Unlit；禁止同资源 SRV+UAV。

#### Scenario: GBufferAO min 语义保护材质 AO
- **WHEN** 材质 AO=0.3，DDGI skyVis=0.7
- **THEN** 写入 min(0.3, 0.7)=0.3

#### Scenario: GBufferAO 写入启用 SkyLight 大面积遮蔽
- **WHEN** SkyVisibility 开启且 Allow Static Lighting Off
- **THEN** SkyLight 采样到 DDGI sky visibility，大厅大面积变暗，无 DFAO 斑点

#### Scenario: ResolutionScale 控制精度与性能
- **WHEN** ResolutionScale=0.25 **THEN** 低分辨率算 skyVis，经 upsample 写回，开销低
- **WHEN** ResolutionScale=1.0 **THEN** 可全分辨率直写，精度最高

### Requirement: Sky visibility multiplied into DDGI irradiance output
`ApplyVolumeLightingContribution` SHALL 在返回前：`LightWeight.rgb *= lerp(1.0, gBufferAO, SkyVisibilityIntensity)`，其中 Intensity 为**全局** CVar。统一作用于 octa+SG。从 **GBufferC.a** 读取，不重复计算。per-volume intensity 仅在 ViewExtension writer 生效。

#### Scenario: 天光关闭时间接光不变
- **WHEN** SkyVisibility=0 **THEN** 不写 AO，reader 乘 1，间接光不变

#### Scenario: 两条路径统一乘
- **WHEN** SG 模式开启且 SkyVisibility 开 **THEN** SG 与 octa 同样被调制

### Requirement: Chebyshev visibility floor weakening
`r.RTXGI.DDGI.ChebyshevFloor` 默认 0.5。`DDGIGetVolumeIrradiance` 与 ApplyLighting SG 路径使用 CVar。`ProbeUpdateRGS.usf` 传固定 0.05。

#### Scenario: CVar 0.05 恢复原始行为
#### Scenario: 默认 0.5 弱化，sky visibility 接管大面积遮挡
#### Scenario: 探针更新路径不受弱化影响

### Requirement: Per-volume sky visibility intensity
`FComponentData.SkyVisibilityIntensity` 默认 1.0。ViewExtension 按密度排序，最密 volume 的 intensity 主导 writer。ApplyLighting SHALL NOT 再乘 per-volume intensity。

#### Scenario: 近处小 volume 遮挡强 / 默认 1.0 时仅全局 CVar 调节 reader

### Requirement: ViewExtension lifecycle and volume lookup
`UDDGISkyVisibilitySubsystem` Initialize 创建 `FDDGISkyVisibilityViewExtension`，Deinitialize 清理。从 `AllProxiesReadyForRender_RenderThread` 用 `OwningScene == View.Family->Scene` 过滤。

#### Scenario: World 卸载自动清理 / PIE 多世界安全
