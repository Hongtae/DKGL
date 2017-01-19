//
//  File: Swapchain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKWindow.h"
#include "CommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class Swapchain : public DKDrawable
			{
			public:
				Swapchain(CommandQueue*, DKWindow*);
				~Swapchain(void);

				bool Setup(void);

				bool Present(void) override;

				bool Update(void);

				bool enableVSync;
				VkSurfaceFormatKHR surfaceFormat;
				VkSurfaceKHR surface;
				VkSwapchainKHR swapchain;

				DKObject<DKWindow> window;
				DKObject<CommandQueue> queue;

			private:

				struct SwapchainImageView
				{
					VkImage image = nullptr;
					VkImageView imageView = nullptr;
				};
				DKArray<SwapchainImageView> buffers;

				void OnWindowEvent(const DKWindow::WindowEvent&);
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
