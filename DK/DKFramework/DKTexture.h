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
			TypeUnknown = 0,
			Type1D,
			Type1DArray,
			Type2D,
			Type2DArray,
			Type3D,
			TypeCube,
		};


		virtual ~DKTexture(void) {}

		virtual uint32_t Width(void) const = 0;
		virtual uint32_t Height(void) const = 0;
		virtual uint32_t Depth(void) const = 0;
		virtual uint32_t MipmapCount(void) const = 0;

		virtual Type TextureType(void) const = 0;
		virtual DKPixelFormat PixelFormat(void) const = 0;
	};
}
