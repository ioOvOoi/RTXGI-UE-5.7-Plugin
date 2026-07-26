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

namespace RTXGISG
{
	struct FLobe
	{
		float AxisX;
		float AxisY;
		float AxisZ;
		float Sharpness;
	};

	// Initial SG policy: fixed world-space axes, fixed uniform sharpness, per-probe RGB amplitudes only.
	// SG12 is the low-risk default for development. SG16 is the higher-quality target for rough specular fidelity.
	// Future work can introduce mixed wide/narrow sharpness bands, but per-probe fitted axes are intentionally forbidden
	// because they make probe interpolation unstable and can cause specular lobe shifting.
	static constexpr float UniformSharpness = 3.0f;

	static constexpr FLobe Basis12[] =
	{
		{  0.000000f,  0.525731f,  0.850651f, UniformSharpness },
		{  0.000000f, -0.525731f,  0.850651f, UniformSharpness },
		{  0.000000f,  0.525731f, -0.850651f, UniformSharpness },
		{  0.000000f, -0.525731f, -0.850651f, UniformSharpness },
		{  0.525731f,  0.850651f,  0.000000f, UniformSharpness },
		{ -0.525731f,  0.850651f,  0.000000f, UniformSharpness },
		{  0.525731f, -0.850651f,  0.000000f, UniformSharpness },
		{ -0.525731f, -0.850651f,  0.000000f, UniformSharpness },
		{  0.850651f,  0.000000f,  0.525731f, UniformSharpness },
		{  0.850651f,  0.000000f, -0.525731f, UniformSharpness },
		{ -0.850651f,  0.000000f,  0.525731f, UniformSharpness },
		{ -0.850651f,  0.000000f, -0.525731f, UniformSharpness },
	};

	static constexpr FLobe Basis16[] =
	{
		{  0.000000f,  1.000000f,  0.000000f, UniformSharpness },
		{  0.000000f, -1.000000f,  0.000000f, UniformSharpness },
		{  1.000000f,  0.000000f,  0.000000f, UniformSharpness },
		{ -1.000000f,  0.000000f,  0.000000f, UniformSharpness },
		{  0.000000f,  0.000000f,  1.000000f, UniformSharpness },
		{  0.000000f,  0.000000f, -1.000000f, UniformSharpness },
		{  0.577350f,  0.577350f,  0.577350f, UniformSharpness },
		{  0.577350f,  0.577350f, -0.577350f, UniformSharpness },
		{  0.577350f, -0.577350f,  0.577350f, UniformSharpness },
		{  0.577350f, -0.577350f, -0.577350f, UniformSharpness },
		{ -0.577350f,  0.577350f,  0.577350f, UniformSharpness },
		{ -0.577350f,  0.577350f, -0.577350f, UniformSharpness },
		{ -0.577350f, -0.577350f,  0.577350f, UniformSharpness },
		{ -0.577350f, -0.577350f, -0.577350f, UniformSharpness },
		{  0.707107f,  0.707107f,  0.000000f, UniformSharpness },
		{ -0.707107f, -0.707107f,  0.000000f, UniformSharpness },
	};

	static constexpr int DefaultLobeCount = 12;
	static constexpr int HighQualityLobeCount = 16;
}
