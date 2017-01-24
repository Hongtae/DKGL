//
//  File: SwapChain.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#include <TargetConditionals.h>
#import <MetalKit/MTKView.h>

#include "SwapChain.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

SwapChain::SwapChain(CommandQueue* q, DKWindow* w)
: queue(q)
, window(w)
, metalLayer(nil)
{
	window->AddEventHandler(this, DKFunction(this, &SwapChain::OnWindowEvent), nullptr, nullptr);

}

SwapChain::~SwapChain(void)
{
	window->RemoveEventHandler(this);

	[metalLayer release];
}

bool SwapChain::Setup(void)
{
	id handle = (id)window->PlatformHandle();
#if TARGET_OS_IPHONE
	if ([handle isKindOfClass:[UIView class]])
	{
		UIView* view = (UIView*)handle;
		Class layerClass = [[view class] layerClass];
		if ([layerClass isSubclassOfClass:[CAMetalLayer class]])
		{
			id layer = view.layer;
			if ([layer isKindOfClass:[CAMetalLayer class]])
			{
				this->metalLayer = [(CAMetalLayer*)layer retain];
			}
		}
	}
#else
	if ([handle isKindOfClass:[NSView class]])
	{
		NSView* view = (NSView*)handle;
		[view setWantsLayer:YES];

		this->metalLayer = [[CAMetalLayer layer] retain];

		[view setLayer:this->metalLayer];
		NSWindow* window = view.window;
		if (window)
			this->metalLayer.contentsScale = window.backingScaleFactor;
		this->metalLayer.bounds = NSRectToCGRect(view.bounds);
	}
#endif

	if (this->metalLayer)
	{
		this->metalLayer.device = this->queue->queue.device;
		this->metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
#if 0
		CGSize drawableSize = this->metalLayer.drawableSize;
		BOOL framebufferOnly = this->metalLayer.framebufferOnly;
		CGSize preferredFrameSize = this->metalLayer.preferredFrameSize;
		BOOL presentsWithTransaction = this->metalLayer.presentsWithTransaction;

		id<CAMetalDrawable> drawable = [this->metalLayer nextDrawable];
		if (drawable)
		{
			id<MTLTexture> texture = drawable.texture;
			NSUInteger width = texture.width;
			NSUInteger height = texture.height;
			BOOL framebufferOnly = texture.framebufferOnly;
		}
#endif
		return true;
	}
	return false;
}

bool SwapChain::Present(void)
{
	return false;
}


void SwapChain::OnWindowEvent(const DKWindow::WindowEvent& e)
{
	if (e.type == DKWindow::WindowEvent::WindowResized)
	{
		//this->Update();
	}
}

#endif //#if DKGL_USE_METAL
