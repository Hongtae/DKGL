//
//  File: SwapChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKWindow.h"
#include "CommandQueue.h"
#include "ImageView.h"

namespace DKFramework::Private::Vulkan
{
	class SwapChain : public DKSwapChain
	{
	public:
		SwapChain(CommandQueue*, DKWindow*);
		~SwapChain();

		bool Setup();
		bool Update();
		void SetupFrame();

		void SetPixelFormat(DKPixelFormat) override;
		DKPixelFormat PixelFormat() const override;

		DKRenderPassDescriptor CurrentRenderPassDescriptor() override;
        size_t MaximumBufferCount() const override;

		bool Present(DKGpuEvent**, size_t) override;

        DKCommandQueue* Queue() override { return queue; }

		bool enableVSync;
		VkSurfaceFormatKHR surfaceFormat;
		VkSurfaceKHR surface;
		VkSwapchainKHR swapchain;
		DKArray<VkSurfaceFormatKHR> availableSurfaceFormats;

        VkSemaphore frameReadySemaphore;

        DKArray<DKObject<ImageView>> imageViews;

		DKObject<DKWindow> window;
		DKObject<CommandQueue> queue;


		DKSpinLock lock;
		bool deviceReset;	// recreate swapchain

		uint32_t frameIndex;
		DKRenderPassDescriptor renderPassDescriptor;
		void OnWindowEvent(const DKWindow::WindowEvent&);
	};
}
#endif //#if DKGL_ENABLE_VULKAN
