//
//  File: SwapChain.h
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
#include "RenderTarget.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class SwapChain : public DKSwapChain
			{
			public:
				SwapChain(CommandQueue*, DKWindow*);
				~SwapChain(void);

				bool Setup(void);

				bool Present(void) override;

				bool Update(void);

				bool enableVSync;
				VkSurfaceFormatKHR surfaceFormat;
				VkSurfaceKHR surface;
				VkSwapchainKHR swapchain;

				DKArray<DKObject<RenderTarget>> renderTargets;

				DKObject<DKWindow> window;
				DKObject<CommandQueue> queue;

			private:
				void OnWindowEvent(const DKWindow::WindowEvent&);
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
