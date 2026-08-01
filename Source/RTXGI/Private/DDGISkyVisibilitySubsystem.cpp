#include "DDGISkyVisibilitySubsystem.h"
#include "DDGISkyVisibilityViewExtension.h"
#include "SceneViewExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DDGISkyVisibilitySubsystem)

void UDDGISkyVisibilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 在本 World 注册 ViewExtension；Deinitialize/World 销毁时 Reset，保证 PIE 不泄漏
	ViewExtension = FSceneViewExtensions::NewExtension<FDDGISkyVisibilityViewExtension>(GetWorld());
	UE_LOG(LogTemp, Log, TEXT("RTXGI: DDGISkyVisibilitySubsystem initialized for world %s"),
		*GetNameSafe(GetWorld()));
}

void UDDGISkyVisibilitySubsystem::Deinitialize()
{
	ViewExtension.Reset();
	Super::Deinitialize();
}

bool UDDGISkyVisibilitySubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// Game / PIE / Editor preview worlds; skip inactive types
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::PIE
		|| WorldType == EWorldType::Editor
		|| WorldType == EWorldType::GamePreview;
}
