//
//  File: ShaderDataType.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKShaderFunction.h"

namespace DKFramework::Private::Metal
{
	struct ShaderDataType
	{
		static MTLDataType From(DKShaderDataType);
		static DKShaderDataType To(MTLDataType);
	};
}
#endif //#if DKGL_ENABLE_METAL

