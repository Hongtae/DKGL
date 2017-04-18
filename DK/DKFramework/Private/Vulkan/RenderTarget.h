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
#include "TextureBaseT.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class RenderTarget : public TextureBaseT<DKRenderTarget>
			{
			public:
				RenderTarget(DKGraphicsDevice*, VkImageView, VkImage, const VkImageCreateInfo*);
				~RenderTarget(void);

				DKObject<DKGraphicsDevice> device;
				VkImageView imageView;

				VkSemaphore signalSemaphore;
			};
		}
	}
}

#endif //#if DKGL_USE_VULKAN
