//
//  File: PixelFormat.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKPixelFormat.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			struct PixelFormat
			{
				static DXGI_FORMAT From(DKPixelFormat);
				static DKPixelFormat To(DXGI_FORMAT);
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL

