/*
* Copyright (c) 2019-2022, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "DDGISkyVisibilitySubsystem.h"
#include "DDGISkyVisibilityViewExtension.h"
#include "SceneViewExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DDGISkyVisibilitySubsystem)

void UDDGISkyVisibilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 在本 World 上注册 ViewExtension。GetWorld() 在 Initialize 时已有效。
	// Deinitialize / World 销毁时 Reset，保证 PIE 进出关卡不泄漏 extension。
	ViewExtension = FSceneViewExtensions::NewExtension<FDDGISkyVisibilityViewExtension>(GetWorld());
}

void UDDGISkyVisibilitySubsystem::Deinitialize()
{
	// 先释放 extension，再 Super：避免销毁顺序上 extension 仍回调已死 World。
	ViewExtension.Reset();
	Super::Deinitialize();
}

bool UDDGISkyVisibilitySubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// 只要会真正渲染场景的 World：Game / PIE / Editor 视口 / GamePreview。
	// 跳过 Inactive 等不会跑 BasePass 的类型，减少无用注册。
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::PIE
		|| WorldType == EWorldType::Editor
		|| WorldType == EWorldType::GamePreview;
}
