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
#include "Subsystems/WorldSubsystem.h"
#include "DDGISkyVisibilitySubsystem.generated.h"

class FDDGISkyVisibilityViewExtension;

/**
 * 每 World 拥有一份 Sky Visibility 的 SceneViewExtension。
 *
 * 为什么用 UWorldSubsystem 而不是模块 Startup 全局注册：
 * - PIE 可同时存在 Editor World + PIE World，全局 extension 会读错场景的探针集合；
 * - World 销毁时 Subsystem 自动 Deinitialize，ViewExtension 与 World 生命周期绑定，避免悬挂。
 */
UCLASS()
class UDDGISkyVisibilitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/**
	 * 为什么强制 true：
	 * 部分项目/基类钩子可能默认不创建自定义 WorldSubsystem，会导致 ViewExtension 永不注册、功能“静默失效”。
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

private:
	/** 渲染侧 extension；必须 ThreadSafe，因创建在游戏线程、使用在渲染线程。 */
	TSharedPtr<FDDGISkyVisibilityViewExtension, ESPMode::ThreadSafe> ViewExtension;
};
