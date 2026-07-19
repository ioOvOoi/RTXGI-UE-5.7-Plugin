#include "DDGISkyVisibilitySubsystem.h"
#include "DDGISkyVisibilityViewExtension.h"
#include "SceneViewExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DDGISkyVisibilitySubsystem)

void UDDGISkyVisibilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ViewExtension = FSceneViewExtensions::NewExtension<FDDGISkyVisibilityViewExtension>(GetWorld());
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
