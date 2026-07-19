#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DDGISkyVisibilitySubsystem.generated.h"

class FDDGISkyVisibilityViewExtension;

/**
 * Owns the per-world sky visibility SceneViewExtension lifecycle.
 */
UCLASS()
class UDDGISkyVisibilitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	TSharedPtr<FDDGISkyVisibilityViewExtension, ESPMode::ThreadSafe> ViewExtension;
};
