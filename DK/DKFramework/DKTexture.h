//
//  File: DKTexture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	class DKTexture
	{
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
	};
}
