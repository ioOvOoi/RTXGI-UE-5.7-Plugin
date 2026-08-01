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

#include "CoreMinimal.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RendererInterface.h"

/**
 * 创建 1x1 R8_UINT 纹理并清 0。
 *
 * 为什么需要：
 * - 探针分类纹理是 Texture2D<uint>，语义 0=PROBE_STATE_ACTIVE，1=INACTIVE；
 * - 引擎 GSystemTextures.BlackDummy 是 float 格式，绑到 uint SRV 是未定义行为，SM6 可能读垃圾；
 * - 清 0 等价于“全部 ACTIVE”，在没有 states 纹理时跳过分类剔除是安全默认（与原 DDGI 无分类路径一致）。
 *
 * 为什么放 Private 头、inline：
 * - 依赖 RDG 私有 API，不能进 Public 头（否则 UObject 公开 include 链会炸编译）；
 * - 调用点少，inline 避免再多一个 .cpp 链接单元。
 */
inline FRDGTextureRef DDGICreateActiveProbeStatesDummy(FRDGBuilder& GraphBuilder)
{
	// 每 GraphBuilder 一份：RDG 帧末释放，无跨帧池化生命周期问题。
	FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		FIntPoint(1, 1),
		PF_R8_UINT,
		FClearValueBinding::None,
		TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef Tex = GraphBuilder.CreateTexture(Desc, TEXT("DDGI.ProbeStates.ActiveDummy"));

	// Texture UAV 清零必须用 FUintVector4；裸 0u 在部分 UE 版本无匹配重载。
	AddClearUAVPass(GraphBuilder, GraphBuilder.CreateUAV(Tex), FUintVector4(0, 0, 0, 0));
	return Tex;
}

/**
 * 有真实 ProbesStates 则注册 external；否则回退 ACTIVE dummy。
 * 统一 ApplyLighting / VolumeUpdate / Visualize / SkyVisibility 的绑定策略，避免路径间格式不一致。
 */
inline FRDGTextureRef DDGIRegisterProbeStatesOrActiveDummy(
	FRDGBuilder& GraphBuilder,
	const TRefCountPtr<IPooledRenderTarget>& ProbesStates)
{
	if (ProbesStates.IsValid())
	{
		return GraphBuilder.RegisterExternalTexture(ProbesStates);
	}
	return DDGICreateActiveProbeStatesDummy(GraphBuilder);
}
