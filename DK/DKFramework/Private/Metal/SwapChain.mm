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
, currentDrawable(nil)
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

		return true;
	}
	return false;
}

bool SwapChain::Present(void)
{
	@autoreleasepool
	{
		id<MTLCommandBuffer> cb = [this->queue->queue commandBuffer];
		[cb presentDrawable:this->currentDrawable];
		[this->currentDrawable autorelease];
		this->currentDrawable = nil;
	}
	return true;
}

DKObject<DKTexture> SwapChain::NextFrame(void)
{
	id<CAMetalDrawable> drawable = [this->metalLayer nextDrawable];
	if (drawable)
	{
		this->currentDrawable = [drawable retain];

		id<MTLTexture> texture = drawable.texture;
		DKObject<Texture> renderTarget = DKOBJECT_NEW Texture(texture);
		return renderTarget.SafeCast<DKTexture>();
	}
	return NULL;
}

void SwapChain::OnWindowEvent(const DKWindow::WindowEvent& e)
{
	if (e.type == DKWindow::WindowEvent::WindowResized)
	{
		//this->Update();
	}
}

#endif //#if DKGL_USE_METAL
