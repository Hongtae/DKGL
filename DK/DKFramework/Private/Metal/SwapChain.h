//
//  File: SwapChain.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "../../DKWindow.h"
#include "CommandQueue.h"
#include "RenderTarget.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class SwapChain : public DKSwapChain
			{
			public:
				SwapChain(CommandQueue*, DKWindow*);
				~SwapChain(void);

				bool Setup(void);

				bool Present(void) override;

				DKObject<DKRenderTarget> NextFrame(void);

				DKObject<DKWindow> window;
				DKObject<CommandQueue> queue;

				CAMetalLayer* metalLayer;

			private:
				id<CAMetalDrawable> currentDrawable;
				void OnWindowEvent(const DKWindow::WindowEvent&);
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
