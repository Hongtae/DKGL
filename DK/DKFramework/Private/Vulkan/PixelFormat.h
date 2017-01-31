//
//  File: PixelFormat.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKPixelFormat.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			struct PixelFormat
			{
				static VkFormat From(DKPixelFormat);
				static DKPixelFormat To(VkFormat);
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
