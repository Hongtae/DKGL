//
//  File: RenderTarget.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKRenderTarget.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class RenderTarget /*: public DKRenderTarget */
			{
			public:
				RenderTarget(DKGraphicsDevice*, VkImageView);
				~RenderTarget(void);

				DKObject<DKGraphicsDevice> device;
				VkImageView imageView;
			};
		}
	}
}

#endif //#if DKGL_USE_VULKAN
