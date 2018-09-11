//
//  File: SwapChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKWindow.h"
#include "CommandQueue.h"
#include "Texture.h"

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

		void SetColorPixelFormat(DKPixelFormat) override;
		void SetDepthStencilPixelFormat(DKPixelFormat) override;
		DKRenderPassDescriptor CurrentRenderPassDescriptor() override;

		bool Present() override;

		DKPixelFormat ColorPixelFormat() const override;
		DKPixelFormat DepthStencilPixelFormat() const override;

		bool enableVSync;
		VkSurfaceFormatKHR surfaceFormat;
		VkSurfaceKHR surface;
		VkSwapchainKHR swapchain;
		DKArray<VkSurfaceFormatKHR> availableSurfaceFormats;

		VkSemaphore presentCompleteSemaphore;
		VkSemaphore renderCompleteSemaphore;


		DKArray<DKObject<Texture>> renderTargets;

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
