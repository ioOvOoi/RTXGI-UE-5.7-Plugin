#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DDGISkyVisibilitySubsystem.generated.h"

class FDDGISkyVisibilityViewExtension;

/**
 * 每 World 持有 sky visibility 的 FWorldSceneViewExtension。
 * 为什么用 UWorldSubsystem：PIE 多世界时自动按 World 创建/销毁，避免全局 extension 串场景。
 */
UCLASS()
class UDDGISkyVisibilitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	// 显式允许创建，避免基类/项目钩子把子系统关掉导致 ViewExtension 永不挂上
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

private:
	TSharedPtr<FDDGISkyVisibilityViewExtension, ESPMode::ThreadSafe> ViewExtension;
};
