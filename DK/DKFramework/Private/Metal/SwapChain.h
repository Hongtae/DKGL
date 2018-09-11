//
//  File: SwapChain.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "../../DKWindow.h"
#include "../../DKSwapChain.h"
#include "CommandQueue.h"

namespace DKFramework::Private::Metal
{
	class SwapChain : public DKSwapChain
	{
	public:
		SwapChain(CommandQueue*, DKWindow*);
		~SwapChain();

		bool Setup();

		void SetColorPixelFormat(DKPixelFormat) override;
		void SetDepthStencilPixelFormat(DKPixelFormat) override;

		DKPixelFormat ColorPixelFormat() const override { return colorPixelFormat; }
		DKPixelFormat DepthStencilPixelFormat() const override { return depthStencilPixelFormat; }


		DKRenderPassDescriptor CurrentRenderPassDescriptor() override;

		bool Present() override;

		void SetupFrame();

		DKObject<DKWindow> window;
		DKObject<CommandQueue> queue;

		CAMetalLayer* metalLayer;

		DKPixelFormat colorPixelFormat;
		DKPixelFormat depthStencilPixelFormat;
		id<CAMetalDrawable> currentDrawable;
		DKRenderPassDescriptor renderPassDescriptor;

		void OnWindowEvent(const DKWindow::WindowEvent&);
	};
}
#endif //#if DKGL_ENABLE_METAL
