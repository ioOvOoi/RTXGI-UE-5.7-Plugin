/*
* Copyright (c) 2019-2022, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#pragma once

#include "SceneViewExtension.h"

/**
 * 延迟 BasePass 之后写入 DDGI 天空可见度（中远距天光开阔度）。
 *
 * 为什么挂在 PostRenderBasePassDeferred：
 * - 此时 GBufferA/B/C 与 Depth 已就绪，可解码世界位置/法线/着色模型；
 * - 写 GBufferC.a（GBufferAO）后，引擎 SkyLight 与后续 DDGI ApplyLighting 都能读到；
 * - 比塞进探针更新更便宜：零额外 RT，只采样已有 distance 纹理。
 *
 * 前置条件：项目 Allow Static Lighting = Off，否则 GBufferC.a 不是 AO 通道。
 * 由 UDDGISkyVisibilitySubsystem 按 World 创建/销毁，避免 PIE 多世界串台。
 */
class FDDGISkyVisibilityViewExtension : public FWorldSceneViewExtension
{
public:
	FDDGISkyVisibilityViewExtension(const FAutoRegister& AutoReg, UWorld* InWorld);

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}

	/**
	 * 渲染线程：低分辨率算 skyVis → 全分辨率 composite min 写 GBufferC.a。
	 * 为什么两 pass：半分辨率禁止直接 scatter 写全分辨率 GBuffer（避免 RMW 撕裂与带宽浪费）。
	 */
	virtual void PostRenderBasePassDeferred_RenderThread(
		FRDGBuilder& GraphBuilder,
		FSceneView& InView,
		const FRenderTargetBindingSlots& RenderTargets,
		TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) override;
};
