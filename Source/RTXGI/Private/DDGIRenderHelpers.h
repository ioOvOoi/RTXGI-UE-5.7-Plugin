#pragma once

#include "CoreMinimal.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RendererInterface.h"

// 1x1 R8_UINT 清 0 (= PROBE_STATE_ACTIVE)。勿用 BlackDummy 绑 Texture2D<uint> ProbeStates。
inline FRDGTextureRef DDGICreateActiveProbeStatesDummy(FRDGBuilder& GraphBuilder)
{
	// ponytail: 每 GraphBuilder 一份 1x1，无跨帧生命周期问题
	FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		FIntPoint(1, 1),
		PF_R8_UINT,
		FClearValueBinding::None,
		TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef Tex = GraphBuilder.CreateTexture(Desc, TEXT("DDGI.ProbeStates.ActiveDummy"));
	// UE：Texture UAV 清零用 FUintVector4，不能传裸 0u
	AddClearUAVPass(GraphBuilder, GraphBuilder.CreateUAV(Tex), FUintVector4(0, 0, 0, 0));
	return Tex;
}

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
