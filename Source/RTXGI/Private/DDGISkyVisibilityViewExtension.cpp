#include "DDGISkyVisibilityViewExtension.h"
#include "DDGIVolumeComponent.h"
#include "DDGIUtilities.h"
#include "DDGIRenderHelpers.h"
#include "LegacyEngineCompat.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "SceneRendering.h"
#include "SceneTextures.h"
#include "SceneView.h"
#include "ShaderParameterStruct.h"
#include "SystemTextures.h"

DECLARE_GPU_STAT_NAMED(RTXGI_SkyVisibility, TEXT("RTXGI Sky Visibility"));

namespace
{
	// ponytail: 固定 4 槽位绑 SRV，改上限需同步 usf/参数结构；超过则按密度截断。若常 >4 再升到 c_RTXGI_DDGI_MAX_SHADING_VOLUMES
	static constexpr int32 GMaxSkyVisVolumes = 4;

	static FRDGTextureRef RegisterOrBlack(FRDGBuilder& GraphBuilder, const TRefCountPtr<IPooledRenderTarget>& Texture)
	{
		if (Texture.IsValid())
		{
			return GraphBuilder.RegisterExternalTexture(Texture);
		}
		return GraphBuilder.RegisterExternalTexture(GSystemTextures.BlackDummy);
	}


	static bool IsSkyVisibilityEnabled()
	{
		static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RTXGI.DDGI.SkyVisibility"));
		return CVar && CVar->GetBool();
	}

	static float GetSkyVisibilityResolutionScale()
	{
		static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RTXGI.DDGI.SkyVisibility.ResolutionScale"));
		return CVar ? FMath::Clamp(CVar->GetFloat(), 0.25f, 1.0f) : 0.75f;
	}

	static int32 GetSkyVisibilitySampleCount()
	{
		static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RTXGI.DDGI.SkyVisibility.SampleCount"));
		const int32 Raw = CVar ? CVar->GetInt() : 8;
		if (Raw <= 1) return 1;
		if (Raw <= 4) return 4;
		if (Raw <= 8) return 8;
		return 16;
	}

	// 中远距 soft 区间（UE cm）。命中距离在 Near~Far 之间从 0 平滑到 1，专打 DFAO 够不着的大厅/峡谷尺度
	static float GetSoftNear()
	{
		static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RTXGI.DDGI.SkyVisibility.SoftNear"));
		return CVar ? FMath::Max(1.0f, CVar->GetFloat()) : 300.0f;
	}

	static float GetSoftFar()
	{
		static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RTXGI.DDGI.SkyVisibility.SoftFar"));
		return CVar ? FMath::Max(2.0f, CVar->GetFloat()) : 8000.0f;
	}

	static float GetWorldUpBias()
	{
		static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RTXGI.DDGI.SkyVisibility.WorldUpBias"));
		return CVar ? FMath::Clamp(CVar->GetFloat(), 0.0f, 1.0f) : 0.35f;
	}

	static float GetSoftLeak()
	{
		static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RTXGI.DDGI.SkyVisibility.Leak"));
		return CVar ? FMath::Clamp(CVar->GetFloat(), 0.0f, 1.0f) : 0.2f;
	}
}

BEGIN_SHADER_PARAMETER_STRUCT(FSkyVisibilityCSParameters, )
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SkyVis_SceneDepth)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SkyVis_GBufferA)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SkyVis_GBufferB)
	SHADER_PARAMETER_SAMPLER(SamplerState, PointClampSampler)
	SHADER_PARAMETER_SAMPLER(SamplerState, LinearClampSampler)
	SHADER_PARAMETER(FIntPoint, ViewOffset)
	SHADER_PARAMETER(FIntPoint, DispatchSize)
	SHADER_PARAMETER(FIntPoint, FullViewSize)
	SHADER_PARAMETER(int32, SampleCount)
	SHADER_PARAMETER(float, SoftNear)
	SHADER_PARAMETER(float, SoftFar)
	SHADER_PARAMETER(float, WorldUpBias)
	SHADER_PARAMETER(float, SoftLeakFloor)
	SHADER_PARAMETER(int32, NumVolumes)

	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Volume_0_ProbeDistance)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Volume_0_ProbeOffsets)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<uint>, Volume_0_ProbeStates)
	SHADER_PARAMETER(FVector3f, Volume_0_Position)
	SHADER_PARAMETER(FVector4f, Volume_0_Rotation)
	SHADER_PARAMETER(FVector3f, Volume_0_Radius)
	SHADER_PARAMETER(FVector3f, Volume_0_ProbeGridSpacing)
	SHADER_PARAMETER(FIntVector, Volume_0_ProbeGridCounts)
	SHADER_PARAMETER(int32, Volume_0_ProbeNumDistanceTexels)
	SHADER_PARAMETER(FIntVector, Volume_0_ProbeScrollOffsets)
	SHADER_PARAMETER(float, Volume_0_SkyVisibilityIntensity)
	SHADER_PARAMETER(float, Volume_0_SkyLightLeak)

	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Volume_1_ProbeDistance)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Volume_1_ProbeOffsets)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<uint>, Volume_1_ProbeStates)
	SHADER_PARAMETER(FVector3f, Volume_1_Position)
	SHADER_PARAMETER(FVector4f, Volume_1_Rotation)
	SHADER_PARAMETER(FVector3f, Volume_1_Radius)
	SHADER_PARAMETER(FVector3f, Volume_1_ProbeGridSpacing)
	SHADER_PARAMETER(FIntVector, Volume_1_ProbeGridCounts)
	SHADER_PARAMETER(int32, Volume_1_ProbeNumDistanceTexels)
	SHADER_PARAMETER(FIntVector, Volume_1_ProbeScrollOffsets)
	SHADER_PARAMETER(float, Volume_1_SkyVisibilityIntensity)
	SHADER_PARAMETER(float, Volume_1_SkyLightLeak)

	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Volume_2_ProbeDistance)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Volume_2_ProbeOffsets)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<uint>, Volume_2_ProbeStates)
	SHADER_PARAMETER(FVector3f, Volume_2_Position)
	SHADER_PARAMETER(FVector4f, Volume_2_Rotation)
	SHADER_PARAMETER(FVector3f, Volume_2_Radius)
	SHADER_PARAMETER(FVector3f, Volume_2_ProbeGridSpacing)
	SHADER_PARAMETER(FIntVector, Volume_2_ProbeGridCounts)
	SHADER_PARAMETER(int32, Volume_2_ProbeNumDistanceTexels)
	SHADER_PARAMETER(FIntVector, Volume_2_ProbeScrollOffsets)
	SHADER_PARAMETER(float, Volume_2_SkyVisibilityIntensity)
	SHADER_PARAMETER(float, Volume_2_SkyLightLeak)

	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Volume_3_ProbeDistance)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Volume_3_ProbeOffsets)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<uint>, Volume_3_ProbeStates)
	SHADER_PARAMETER(FVector3f, Volume_3_Position)
	SHADER_PARAMETER(FVector4f, Volume_3_Rotation)
	SHADER_PARAMETER(FVector3f, Volume_3_Radius)
	SHADER_PARAMETER(FVector3f, Volume_3_ProbeGridSpacing)
	SHADER_PARAMETER(FIntVector, Volume_3_ProbeGridCounts)
	SHADER_PARAMETER(int32, Volume_3_ProbeNumDistanceTexels)
	SHADER_PARAMETER(FIntVector, Volume_3_ProbeScrollOffsets)
	SHADER_PARAMETER(float, Volume_3_SkyVisibilityIntensity)
	SHADER_PARAMETER(float, Volume_3_SkyLightLeak)

	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, SkyVisOutput)
END_SHADER_PARAMETER_STRUCT()

class FSkyVisibilityCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSkyVisibilityCS);
	SHADER_USE_PARAMETER_STRUCT(FSkyVisibilityCS, FGlobalShader);
	using FParameters = FSkyVisibilityCSParameters;

	class FEnableRelocation : SHADER_PERMUTATION_BOOL("RTXGI_DDGI_PROBE_RELOCATION");
	class FEnableScrolling : SHADER_PERMUTATION_BOOL("RTXGI_DDGI_INFINITE_SCROLLING_VOLUME");
	using FPermutationDomain = TShaderPermutationDomain<FEnableRelocation, FEnableScrolling>;

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("RTXGI_DDGI_PROBE_CLASSIFICATION"), FDDGIVolumeSceneProxy::FComponentData::c_RTXGI_DDGI_PROBE_CLASSIFICATION ? 1 : 0);
		OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
#if ENGINE_MAJOR_VERSION < 5
		OutEnvironment.SetDefine(TEXT("UE4_COMPAT"), 1);
#else
		OutEnvironment.SetDefine(TEXT("UE4_COMPAT"), 0);
#endif
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, GMaxRHIFeatureLevel);
	}
};
IMPLEMENT_GLOBAL_SHADER(FSkyVisibilityCS, "/Plugin/RTXGI/Private/SkyVisibilityCS.usf", "MainCS", SF_Compute);

BEGIN_SHADER_PARAMETER_STRUCT(FSkyVisibilityCompositeCSParameters, )
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SkyVisTexture)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SkyVis_GBufferB)
	SHADER_PARAMETER_SAMPLER(SamplerState, LinearClampSampler)
	SHADER_PARAMETER_SAMPLER(SamplerState, PointClampSampler)
	SHADER_PARAMETER(FIntPoint, ViewOffset)
	SHADER_PARAMETER(FIntPoint, ViewSize)
	SHADER_PARAMETER(FIntPoint, SkyVisSize)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SkyVis_GBufferCUAV)
END_SHADER_PARAMETER_STRUCT()

class FSkyVisibilityCompositeCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSkyVisibilityCompositeCS);
	SHADER_USE_PARAMETER_STRUCT(FSkyVisibilityCompositeCS, FGlobalShader);
	using FParameters = FSkyVisibilityCompositeCSParameters;

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, GMaxRHIFeatureLevel);
	}
};
IMPLEMENT_GLOBAL_SHADER(FSkyVisibilityCompositeCS, "/Plugin/RTXGI/Private/SkyVisibilityCompositeCS.usf", "MainCS", SF_Compute);

FDDGISkyVisibilityViewExtension::FDDGISkyVisibilityViewExtension(const FAutoRegister& AutoReg, UWorld* InWorld)
	: FWorldSceneViewExtension(AutoReg, InWorld)
{
}

void FDDGISkyVisibilityViewExtension::PostRenderBasePassDeferred_RenderThread(
	FRDGBuilder& GraphBuilder,
	FSceneView& InView,
	const FRenderTargetBindingSlots& RenderTargets,
	TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTexturesUB)
{
	if (!IsSkyVisibilityEnabled() || !InView.bIsViewInfo || !InView.Family || !InView.Family->Scene)
	{
		return;
	}

	// 引擎保证 BasePass（含本 hook）完成后再跑 DiffuseIndirect；同帧 GBufferC.a 写后读依赖引擎 pass 序
	FViewInfo& View = static_cast<FViewInfo&>(InView);
	const FSceneInterface* Scene = View.Family->Scene;

	struct FProxyEntry
	{
		FDDGIVolumeSceneProxy* Proxy = nullptr;
		float Density = 0.0f;
		FVector4f Rotation = FVector4f(0, 0, 0, 1);
		FVector3f Scale = FVector3f(1, 1, 1);
	};

	TArray<FProxyEntry, TInlineAllocator<GMaxSkyVisVolumes>> Volumes;
	for (FDDGIVolumeSceneProxy* Proxy : FDDGIVolumeSceneProxy::AllProxiesReadyForRender_RenderThread)
	{
		// 与 ApplyLighting 列表对齐：本 Scene、启用 volume、有 distance、sky vis intensity>0
		if (!Proxy || Proxy->OwningScene != Scene || !Proxy->ComponentData.EnableVolume || !Proxy->ProbesDistance.IsValid())
		{
			continue;
		}
		if (Proxy->ComponentData.SkyVisibilityIntensity <= 0.0f)
		{
			continue;
		}

		const FVector3f Scale = Proxy->ComponentData.Transform.GetScale3D();
		// UE volume 世界尺寸 ≈ Scale*200；密度仅用于排序 densest-first
		const FVector3f WorldSize = Scale * 200.0f;
		const float VolumeVolume = FMath::Max(WorldSize.X * WorldSize.Y * WorldSize.Z, KINDA_SMALL_NUMBER);
		const float ProbeCount = float(Proxy->ComponentData.ProbeCounts.X * Proxy->ComponentData.ProbeCounts.Y * Proxy->ComponentData.ProbeCounts.Z);
		const FQuat4f Rot = Proxy->ComponentData.Transform.GetRotation();

		Volumes.Add(FProxyEntry{
			Proxy,
			ProbeCount / VolumeVolume,
			FVector4f(Rot.X, Rot.Y, Rot.Z, Rot.W),
			Scale
		});
	}

	if (Volumes.Num() == 0)
	{
		return;
	}

	Volumes.Sort([](const FProxyEntry& A, const FProxyEntry& B) { return A.Density > B.Density; });
	if (Volumes.Num() > GMaxSkyVisVolumes)
	{
		Volumes.SetNum(GMaxSkyVisVolumes, EAllowShrinking::No);
	}

	// Require matching relocation/scrolling flags across bound volumes (same as ApplyLighting)
	const bool bRelocation = Volumes[0].Proxy->ComponentData.EnableProbeRelocation;
	const bool bScrolling = Volumes[0].Proxy->ComponentData.EnableProbeScrolling;
	for (int32 i = Volumes.Num() - 1; i >= 0; --i)
	{
		const auto& CD = Volumes[i].Proxy->ComponentData;
		if (CD.EnableProbeRelocation != bRelocation || CD.EnableProbeScrolling != bScrolling)
		{
			Volumes.RemoveAtSwap(i, EAllowShrinking::No);
		}
	}
	if (Volumes.Num() == 0)
	{
		return;
	}

	// UE5.7：FSceneTextures::Get(GraphBuilder) 已移除；从 ViewFamily 取
	const FViewFamilyInfo* ViewFamilyInfo = static_cast<const FViewFamilyInfo*>(View.Family);
	const FSceneTextures* SceneTexturesPtr = ViewFamilyInfo ? ViewFamilyInfo->GetSceneTexturesChecked() : nullptr;
	if (!SceneTexturesPtr)
	{
		return;
	}
	const FSceneTextures& SceneTextures = *SceneTexturesPtr;
	FRDGTextureRef GBufferA = SceneTextures.GBufferA;
	FRDGTextureRef GBufferB = SceneTextures.GBufferB;
	FRDGTextureRef GBufferC = SceneTextures.GBufferC;
	FRDGTextureRef SceneDepth = SceneTextures.Depth.Target;
	if (!GBufferA || !GBufferB || !GBufferC || !SceneDepth)
	{
		return;
	}

	const FIntPoint FullViewSize = View.ViewRect.Size();
	const float ResScale = GetSkyVisibilityResolutionScale();
	const FIntPoint DispatchSize(
		FMath::Max(1, FMath::CeilToInt(FullViewSize.X * ResScale)),
		FMath::Max(1, FMath::CeilToInt(FullViewSize.Y * ResScale)));

	FRDGTextureDesc SkyVisDesc = FRDGTextureDesc::Create2D(
		DispatchSize,
		PF_R16F,
		FClearValueBinding::None,
		TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef SkyVisRT = GraphBuilder.CreateTexture(SkyVisDesc, TEXT("DDGI.SkyVisibility"));

	RDG_EVENT_SCOPE(GraphBuilder, "RTXGI_SkyVisibility");
	RDG_GPU_STAT_SCOPE(GraphBuilder, RTXGI_SkyVisibility);

	{
		FSkyVisibilityCS::FPermutationDomain PermutationVector;
		PermutationVector.Set<FSkyVisibilityCS::FEnableRelocation>(bRelocation);
		PermutationVector.Set<FSkyVisibilityCS::FEnableScrolling>(bScrolling);
		TShaderMapRef<FSkyVisibilityCS> ComputeShader(GetGlobalShaderMap(View.GetFeatureLevel()), PermutationVector);

		FSkyVisibilityCSParameters* PassParameters = GraphBuilder.AllocParameters<FSkyVisibilityCSParameters>();
		PassParameters->View = View.ViewUniformBuffer;
		PassParameters->SkyVis_SceneDepth = SceneDepth;
		PassParameters->SkyVis_GBufferA = GBufferA;
		PassParameters->SkyVis_GBufferB = GBufferB;
		PassParameters->PointClampSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParameters->LinearClampSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParameters->ViewOffset = View.ViewRect.Min;
		PassParameters->DispatchSize = DispatchSize;
		PassParameters->FullViewSize = FullViewSize;
		PassParameters->SampleCount = GetSkyVisibilitySampleCount();
		PassParameters->SoftNear = GetSoftNear();
		PassParameters->SoftFar = FMath::Max(GetSoftFar(), GetSoftNear() + 1.0f);
		PassParameters->WorldUpBias = GetWorldUpBias();
		// 全局泄露地板；per-volume SkyLightLeak 在 shader 里 max(Floor, VolLeak)
		PassParameters->SoftLeakFloor = GetSoftLeak();
		PassParameters->NumVolumes = Volumes.Num();
		PassParameters->SkyVisOutput = GraphBuilder.CreateUAV(SkyVisRT);

		FRDGTextureRef Black = GraphBuilder.RegisterExternalTexture(GSystemTextures.BlackDummy);
		FRDGTextureRef StatesActiveDummy = DDGICreateActiveProbeStatesDummy(GraphBuilder);

		auto BindVolume = [&](int32 Index, FDDGIVolumeSceneProxy* Proxy, const FVector4f& Rotation, const FVector3f& Scale)
		{
			const FVector3f VolumeSize = Scale * 200.0f;
			FVector3f ProbeGridSpacing;
			ProbeGridSpacing.X = VolumeSize.X / float(Proxy->ComponentData.ProbeCounts.X);
			ProbeGridSpacing.Y = VolumeSize.Y / float(Proxy->ComponentData.ProbeCounts.Y);
			ProbeGridSpacing.Z = VolumeSize.Z / float(Proxy->ComponentData.ProbeCounts.Z);
			const FVector3f Radius = Scale * 100.0f;
			const FRDGTextureRef Distance = GraphBuilder.RegisterExternalTexture(Proxy->ProbesDistance);
			const FRDGTextureRef Offsets = RegisterOrBlack(GraphBuilder, Proxy->ProbesOffsets);
			const FRDGTextureRef States = Proxy->ProbesStates.IsValid()
				? GraphBuilder.RegisterExternalTexture(Proxy->ProbesStates)
				: StatesActiveDummy;

#define BIND_VOLUME_SLOT(N) \
			if (Index == N) \
			{ \
				PassParameters->Volume_##N##_ProbeDistance = Distance; \
				PassParameters->Volume_##N##_ProbeOffsets = Offsets; \
				PassParameters->Volume_##N##_ProbeStates = States; \
				PassParameters->Volume_##N##_Position = Proxy->ComponentData.Origin; \
				PassParameters->Volume_##N##_Rotation = Rotation; \
				PassParameters->Volume_##N##_Radius = Radius; \
				PassParameters->Volume_##N##_ProbeGridSpacing = ProbeGridSpacing; \
				PassParameters->Volume_##N##_ProbeGridCounts = Proxy->ComponentData.ProbeCounts; \
				PassParameters->Volume_##N##_ProbeNumDistanceTexels = FDDGIVolumeSceneProxy::FComponentData::c_NumTexelsDistance; \
				PassParameters->Volume_##N##_ProbeScrollOffsets = Proxy->ComponentData.ProbeScrollOffsets; \
				PassParameters->Volume_##N##_SkyVisibilityIntensity = Proxy->ComponentData.SkyVisibilityIntensity; \
				PassParameters->Volume_##N##_SkyLightLeak = FMath::Clamp(Proxy->ComponentData.SkyLightLeak, 0.0f, 1.0f); \
			}
			BIND_VOLUME_SLOT(0)
			BIND_VOLUME_SLOT(1)
			BIND_VOLUME_SLOT(2)
			BIND_VOLUME_SLOT(3)
#undef BIND_VOLUME_SLOT
		};

		for (int32 i = 0; i < Volumes.Num(); ++i)
		{
			BindVolume(i, Volumes[i].Proxy, Volumes[i].Rotation, Volumes[i].Scale);
		}
		// Unused slots: black textures + zero intensity (NumVolumes gates evaluation)
		for (int32 i = Volumes.Num(); i < GMaxSkyVisVolumes; ++i)
		{
#define ZERO_SLOT(N) \
			if (i == N) \
			{ \
				PassParameters->Volume_##N##_ProbeDistance = Black; \
				PassParameters->Volume_##N##_ProbeOffsets = Black; \
				PassParameters->Volume_##N##_ProbeStates = StatesActiveDummy; \
				PassParameters->Volume_##N##_Position = FVector3f::ZeroVector; \
				PassParameters->Volume_##N##_Rotation = FVector4f(0, 0, 0, 1); \
				PassParameters->Volume_##N##_Radius = FVector3f::ZeroVector; \
				PassParameters->Volume_##N##_ProbeGridSpacing = FVector3f(1, 1, 1); \
				PassParameters->Volume_##N##_ProbeGridCounts = FIntVector(1, 1, 1); \
				PassParameters->Volume_##N##_ProbeNumDistanceTexels = 1; \
				PassParameters->Volume_##N##_ProbeScrollOffsets = FIntVector::ZeroValue; \
				PassParameters->Volume_##N##_SkyVisibilityIntensity = 0.0f; \
				PassParameters->Volume_##N##_SkyLightLeak = 0.0f; \
			}
			ZERO_SLOT(0) ZERO_SLOT(1) ZERO_SLOT(2) ZERO_SLOT(3)
#undef ZERO_SLOT
		}

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("DDGI_SkyVisibility_Compute %dx%d", DispatchSize.X, DispatchSize.Y),
			ComputeShader,
			PassParameters,
			FComputeShaderUtils::GetGroupCount(DispatchSize, FIntPoint(8, 8)));
	}

	{
		TShaderMapRef<FSkyVisibilityCompositeCS> ComputeShader(GetGlobalShaderMap(View.GetFeatureLevel()));
		FSkyVisibilityCompositeCSParameters* PassParameters = GraphBuilder.AllocParameters<FSkyVisibilityCompositeCSParameters>();
		PassParameters->View = View.ViewUniformBuffer;
		PassParameters->SkyVisTexture = SkyVisRT;
		PassParameters->SkyVis_GBufferB = GBufferB;
		PassParameters->LinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParameters->PointClampSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParameters->ViewOffset = View.ViewRect.Min;
		PassParameters->ViewSize = FullViewSize;
		PassParameters->SkyVisSize = DispatchSize;
		PassParameters->SkyVis_GBufferCUAV = GraphBuilder.CreateUAV(GBufferC);

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("DDGI_SkyVisibility_Composite %dx%d", FullViewSize.X, FullViewSize.Y),
			ComputeShader,
			PassParameters,
			FComputeShaderUtils::GetGroupCount(FullViewSize, FIntPoint(8, 8)));
	}
}
