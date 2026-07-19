## Context

RTXGI DDGI 插件通过 `FGlobalIlluminationPluginDelegates::RenderDiffuseIndirectLight` 接入 UE5 渲染管线，在 lighting 阶段输出间接辐照度到 SceneColor。探针每帧追踪射线存 distance texture，`distance > 1e27f` 表示射线 miss 天空。当前辐照度 gather 里的 Chebyshev visibility（`Irradiance.ush:167-183`）用方差概率做软遮挡——分辨率受限于探针网格间距（通常 2m），对大厅大空间的天光大面积遮蔽不足。

UE5 的 SkyLight pass（`SkyLighting.usf:SkyLightDiffusePS`）读 `GBuffer.GBufferAO` 做天光遮挡——`SkyVisibility = min(SkyVisibility, min(GBuffer.GBufferAO, ScreenSpaceData.AmbientOcclusion))`。GBufferAO 在 `Allow Static Lighting` 关闭后映射为 **GBufferC.a（MRT index 3）**。DFAO 写入此通道但受 128³ SDF 体素上限，大网格上产生斑点噪点。

本项目是预编译安装版 UE 5.3/5.7（无引擎源码），所有改动必须在插件侧完成。

## Goals / Non-Goals

**Goals:**
- 从 DDGI distance texture 算 sky visibility（零额外 RT 追踪，纯采样）
- 写入 GBufferAO（GBufferC.a）让 UE5 SkyLight pass 自动消费
- sky visibility 同时乘到 DDGI 辐照度输出
- 弱化 Chebyshev visibility 让 sky visibility 接管大面积遮挡
- 全部通过 CVars + per-volume 属性控制，不改引擎源码

**Non-Goals:**
- 不替换 DDGI 辐照度 gather 核心逻辑
- 不做独立 RT AO pass
- 不影响探针更新射线追踪
- 不做 X2（per-probe sky visibility 新通道）

## Decisions

### D1：数据来源——复用 distance texture（X1）
探针 miss 已存 `distance = 1e27f`。沿方向查 distance，`> 1e10f` = 天空可见。X2 留 TODO。

### D2：应用机制——ViewExtension 写 AO + DDGI 读 AO（强度只生效一次）
`FWorldSceneViewExtension::PostRenderBasePassDeferred_RenderThread`（`UWorldSubsystem` 注册）写 **GBufferC.a**。
- **Writer**：`skyVis = lerp(1, computed, perVolumeIntensity)`，再 `min` 写入 GBufferC.a
- **Reader（ApplyLighting）**：`LightWeight.rgb *= lerp(1, GBufferAO, globalIntensity)`——全局强度只在 reader 生效
- ApplyLighting **不**绑定 per-volume intensity

### D3：多方向采样——N=8 半球均匀 + CVar
`DDGISphericalFibonacci`；SampleCount 1/4/8/16。

### D4：分辨率策略 (A)——禁止半分辨率 scatter 写全分辨率 GBuffer
低分辨率 CS → 中间 skyVis RT → 全分辨率 pass 采样/upsample，对 GBufferC 做 float4 RMW（只改 `.a`，`min`）。`ResolutionScale≈1` 时可走 (B) 全分辨率直写。

### D5：GBufferAO = GBufferC.a，min 语义
整 texel float4 RMW；跳过 Unlit；禁止同资源 SRV+UAV；Allow Static Lighting 必须 Off。

### D6：Chebyshev floor CVar 0.5；ProbeUpdate 固定 0.05

### D7：共享函数放 `Irradiance.ush`

### D8：`UWorldSubsystem` + `FWorldSceneViewExtension` 生命周期

## Risks / Trade-offs

- **[位置偏移]** ~1 探针间距；大尺度可接受
- **[Chebyshev 弱化]** CVar 可调回 0.05；ProbeUpdate 不受影响
- **[GBufferAO RMW]** 必须经中间 RT + 全分辨率 upsample；写错通道会破坏 BaseColor
- **[双暗化]** 天光与间接光各自读 AO 是设计意图
- **[Allow Static Lighting]** BREAKING：必须 Off
