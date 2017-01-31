//
//  File: DKTexture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKPixelFormat.h"

namespace DKFramework
{
	class DKTexture
	{
	public:
		enum Type
		{
			Type1D,
			Type1DArray,
			Type2D,
			Type2DArray,
			Type2DMultisample,
			TypeCube,
			Type3D,
		};
		enum Usage
		{
			UsageUnknown,
			UsageShaderRead,
			UsageShaderWrite,
			UsageRenderTarget,
			UsagePixelFormatView,
		};

		virtual ~DKTexture(void) {}

		virtual uint32_t Width(void) = 0;
		virtual uint32_t Height(void) = 0;
		virtual uint32_t Depth(void) = 0;
		virtual uint32_t MipmapCount(void) = 0;

		virtual Type TextureType(void) = 0;
		virtual Usage TextureUsage(void) = 0;
		virtual DKPixelFormat PixelFormat(void) = 0;
	};
}
