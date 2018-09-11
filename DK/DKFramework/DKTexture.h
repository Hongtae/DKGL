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
	struct DKTextureDescriptor
	{

	};

	class DKTexture
	{
	public:
		enum Type
		{
			TypeUnknown = 0,
			Type1D,
			Type1DArray,
			Type2D,
			Type2DArray,
			Type3D,
			TypeCube,
		};
		enum Usage
		{
			UsageUnknown = 0x0,
			UsageShaderRead = 0x1,
			UsageShaderWrite = 0x2,
			UsageRenderTarget = 0x4,
			UsagePixelFormatView = 0x10,
		};


		virtual ~DKTexture() {}

		virtual uint32_t Width() const = 0;
		virtual uint32_t Height() const = 0;
		virtual uint32_t Depth() const = 0;
		virtual uint32_t MipmapCount() const = 0;

		virtual Type TextureType() const = 0;
		virtual DKPixelFormat PixelFormat() const = 0;
	};
}
