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
		};


		virtual ~DKTexture(void) {}

		virtual uint32_t Width(void) = 0;
		virtual uint32_t Height(void) = 0;
		virtual uint32_t Depth(void) = 0;
		virtual uint32_t MipmapCount(void) = 0;

		virtual Type TextureType(void) = 0;
		virtual DKPixelFormat PixelFormat(void) = 0;
	};
}
