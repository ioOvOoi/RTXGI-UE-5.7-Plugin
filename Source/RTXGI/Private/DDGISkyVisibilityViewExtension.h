#pragma once

#include "SceneViewExtension.h"

/**
 * Writes DDGI sky visibility into GBufferC.a (GBufferAO) after the deferred base pass.
 * Requires Allow Static Lighting = Off so GBufferC.a is AO.
 */
class FDDGISkyVisibilityViewExtension : public FWorldSceneViewExtension
{
public:
	FDDGISkyVisibilityViewExtension(const FAutoRegister& AutoReg, UWorld* InWorld);

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}

	virtual void PostRenderBasePassDeferred_RenderThread(
		FRDGBuilder& GraphBuilder,
		FSceneView& InView,
		const FRenderTargetBindingSlots& RenderTargets,
		TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) override;
};
