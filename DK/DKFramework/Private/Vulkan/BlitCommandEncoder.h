//
//  File: BlitCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKBlitCommandEncoder.h"

#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class BlitCommandEncoder : public DKBlitCommandEncoder
			{
			public:
				BlitCommandEncoder(CommandBuffer*);
				~BlitCommandEncoder(void);
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
