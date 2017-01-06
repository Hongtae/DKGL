//
//  File: DKSampler.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKDepthStencil.h"

namespace DKFramework
{
	class DKSamplerDescriptor
	{
		enum MinMagFilter
		{
			MinMagFilterNearest,
			MinMagFilterLinear,
		};
		enum MipFilter
		{
			MipFilterNotMipmapped,
			MipFilterNearest,
			MipFilterLinear,
		};
		enum AddressMode
		{
			AddressModeClampToEdge,
			AddressModeRepeat,
			AddressModeMirrorRepeat,
			AddressModeClampToZero,
		};
		enum BorderColor
		{
			BorderColorTransparentBlack,
			BorderColorOpaqueBlack,
			BorderColorOpaqueWhite
		};
	};

	class DKSamplerState
	{
	};
}
