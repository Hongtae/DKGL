//
//  File: DKBlendState.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	enum class DKBlendFactor
	{
		Zero,
		One,
		SourceColor,
		OneMinusSourceColor,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestinationColor,
		OneMinusDestinationColor,
		DestinationAlpha,
		OneMinusDestinationAlpha,
		SourceAlphaSaturated,
		BlendColor,
		OneMinusBlendColor,
		BlendAlpha,
		OneMinusBlendAlpha,
	};

	enum class DKBlendOperation
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max,
	};

	enum DKColorWriteMask : uint8_t
	{
		DKColorWriteMaskNone = 0,
		DKColorWriteMaskRed = 0x1 << 3,
		DKColorWriteMaskGreen = 0x1 << 2,
		DKColorWriteMaskBlue = 0x1 << 1,
		DKColorWriteMaskAlpha = 0x1 << 0,
		DKColorWriteMaskAll = 0xf
	};

	struct DKGL_API DKBlendState
	{
		bool enabled = false;

		DKBlendFactor sourceRGBBlendFactor = DKBlendFactor::One;
		DKBlendFactor sourceAlphaBlendFactor = DKBlendFactor::One;

		DKBlendFactor destinationRGBBlendFactor = DKBlendFactor::Zero;
		DKBlendFactor destinationAlphaBlendFactor = DKBlendFactor::Zero;

		DKBlendOperation rgbBlendOperation = DKBlendOperation::Add;
		DKBlendOperation alphaBlendOperation = DKBlendOperation::Add;

		DKColorWriteMask writeMask = DKColorWriteMaskAll;

		// preset
		static const DKBlendState	defaultOpaque;
		static const DKBlendState	defaultAlpha;
		static const DKBlendState	defaultMultiply;
		static const DKBlendState	defaultScreen;
		static const DKBlendState	defaultDarken;
		static const DKBlendState	defaultLighten;
		static const DKBlendState	defaultLinearBurn;
		static const DKBlendState	defaultLinearDodge;
	};
}
