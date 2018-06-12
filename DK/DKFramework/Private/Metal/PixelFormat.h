//
//  File: PixelFormat.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKPixelFormat.h"

namespace DKFramework::Private::Metal
{
	struct PixelFormat
	{
		static MTLPixelFormat From(DKPixelFormat);
		static DKPixelFormat To(MTLPixelFormat);
	};
}
#endif //#if DKGL_ENABLE_METAL

