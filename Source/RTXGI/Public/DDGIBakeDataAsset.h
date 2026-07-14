/*
* Copyright (c) 2024, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Engine/DataAsset.h"

#include "DDGIBakeDataAsset.generated.h"

// ponytail: forward-declared to break circular include between DDGIBakeDataAsset
// and DDGIVolumeComponent. The full definition is only needed in the .cpp for
// ToTexturePixels/FromTexturePixels method bodies.
struct FDDGITexturePixels;

/** Serializable texture payload descriptor (subset of FDDGITexturePixels.Desc that survives serialization). */
USTRUCT()
struct FDDGIBakeTexturePayloadDesc
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Bake Texture")
	uint32 Width = 0;

	UPROPERTY(VisibleAnywhere, Category = "Bake Texture")
	uint32 Height = 0;

	UPROPERTY(VisibleAnywhere, Category = "Bake Texture")
	uint32 Stride = 0;

	UPROPERTY(VisibleAnywhere, Category = "Bake Texture")
	uint32 PixelFormat = 0;
};

/**
 * Serializable texture payload for bake assets.
 * Holds only the data that survives serialization (Desc + Pixels).
 * Transient GPU handles (PendingReadback, Texture) are intentionally excluded.
 * Use FromTexturePixels()/ToTexturePixels() to convert to/from runtime FDDGITexturePixels.
 */
USTRUCT()
struct FDDGIBakeTexturePayload
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Bake Texture")
	FDDGIBakeTexturePayloadDesc Desc;

	UPROPERTY(VisibleAnywhere, Category = "Bake Texture")
	TArray<uint8> Pixels;

	/** Populate from a runtime FDDGITexturePixels (captures Desc + Pixels, discards GPU handles). */
	void FromTexturePixels(const FDDGITexturePixels& In);

	/** Convert to runtime FDDGITexturePixels (copies Desc + Pixels; GPU handles remain null). */
	void ToTexturePixels(FDDGITexturePixels& Out) const;
};

/**
 * A cookable primary data asset holding a complete DDGI volume snapshot.
 * Stores 5 inline texture payloads (Irradiance, Distance, Offsets, States, SGAmplitudes)
 * plus bake configuration metadata.
 *
 * Saved under <MapName>/DDGIBakes/ for per-map bake management.
 */
UCLASS(BlueprintType)
class RTXGI_API UDDGIBakeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Texture payloads ---

	UPROPERTY(VisibleAnywhere, Category = "Bake Data")
	FDDGIBakeTexturePayload Irradiance;

	UPROPERTY(VisibleAnywhere, Category = "Bake Data")
	FDDGIBakeTexturePayload Distance;

	UPROPERTY(VisibleAnywhere, Category = "Bake Data")
	FDDGIBakeTexturePayload Offsets;

	UPROPERTY(VisibleAnywhere, Category = "Bake Data")
	FDDGIBakeTexturePayload States;

	UPROPERTY(VisibleAnywhere, Category = "Bake Data")
	FDDGIBakeTexturePayload SGAmplitudes;

	// --- Metadata for validation on load ---

	UPROPERTY(VisibleAnywhere, Category = "Bake Metadata")
	FIntVector ProbeCounts = FIntVector(0, 0, 0);

	UPROPERTY(VisibleAnywhere, Category = "Bake Metadata")
	int32 RaysPerProbe = 0;

	UPROPERTY(VisibleAnywhere, Category = "Bake Metadata")
	int32 SGLobeCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Bake Metadata")
	bool bEnableProbeRelocation = false;

	UPROPERTY(VisibleAnywhere, Category = "Bake Metadata")
	bool bEnableProbeScrolling = false;

	UPROPERTY(VisibleAnywhere, Category = "Bake Metadata")
	FString BakeName;
};
