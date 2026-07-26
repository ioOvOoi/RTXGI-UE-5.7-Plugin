/*
* Copyright (c) 2024, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "DDGIBakeDataAsset.h"
#include "DDGIVolumeComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DDGIBakeDataAsset)

void FDDGIBakeTexturePayload::FromTexturePixels(const FDDGITexturePixels& In)
{
	Desc.Width       = In.Desc.Width;
	Desc.Height      = In.Desc.Height;
	Desc.Stride      = In.Desc.Stride;
	Desc.PixelFormat = In.Desc.PixelFormat;
	Pixels           = In.Pixels;
}

void FDDGIBakeTexturePayload::ToTexturePixels(FDDGITexturePixels& Out) const
{
	Out.Desc.Width       = Desc.Width;
	Out.Desc.Height      = Desc.Height;
	Out.Desc.Stride      = Desc.Stride;
	Out.Desc.PixelFormat = Desc.PixelFormat;
	Out.Pixels           = Pixels;

	// ponytail: PendingReadback and Texture are transient GPU handles that must be
	// created fresh by the caller (e.g. via LoadFDDGITexturePixels or the equivalent
	// RHI texture creation path). They are intentionally left null here.
	Out.PendingReadback.Reset();
	Out.Texture = nullptr;
}
