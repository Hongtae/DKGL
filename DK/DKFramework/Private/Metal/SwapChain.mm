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

	renderPassDescriptor.colorAttachments.Clear();
	renderPassDescriptor.depthStencilAttachment.renderTarget = nullptr;

	[currentDrawable release];
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


void SwapChain::SetColorPixelFormat(DKPixelFormat)
{
}

void SwapChain::SetDepthStencilPixelFormat(DKPixelFormat)
{
}

DKRenderPassDescriptor SwapChain::CurrentRenderPassDescriptor(void)
{
	if (currentDrawable == nil)
		SetupFrame();

	return renderPassDescriptor;
}

bool SwapChain::Present(void)
{
	if (currentDrawable)
	{
		@autoreleasepool
		{
			[currentDrawable present];
			[currentDrawable autorelease];
			currentDrawable = nil;
		}
		return true;
	}
	return false;
}

void SwapChain::SetupFrame(void)
{
	if (currentDrawable == nil)
	{
		renderPassDescriptor.colorAttachments.Clear();
		renderPassDescriptor.depthStencilAttachment.renderTarget = nullptr;

		id<CAMetalDrawable> drawable = [this->metalLayer nextDrawable];
		if (drawable)
			currentDrawable = [drawable retain];
	}

	if (currentDrawable)
	{
		renderPassDescriptor.colorAttachments.Clear();
		renderPassDescriptor.depthStencilAttachment.renderTarget = nullptr;

		id<MTLTexture> texture = currentDrawable.texture;
		DKObject<RenderTarget> renderTarget = DKOBJECT_NEW RenderTarget(texture, queue->Device());

		DKRenderPassColorAttachmentDescriptor colorAttachment;
		colorAttachment.renderTarget = renderTarget.SafeCast<DKRenderTarget>();
		colorAttachment.clearColor = DKColor(0,0,0,0);
		colorAttachment.loadAction = colorAttachment.LoadActionClear;
		colorAttachment.storeAction = colorAttachment.StoreActionStore;

		renderPassDescriptor.colorAttachments.Clear();
		renderPassDescriptor.colorAttachments.Add(colorAttachment);
		renderPassDescriptor.depthStencilAttachment.renderTarget = nullptr;
		renderPassDescriptor.depthStencilAttachment.loadAction = DKRenderPassAttachmentDescriptor::LoadActionClear;
		renderPassDescriptor.depthStencilAttachment.storeAction = DKRenderPassAttachmentDescriptor::StoreActionDontCare;
	}
}

void SwapChain::OnWindowEvent(const DKWindow::WindowEvent& e)
{
	if (e.type == DKWindow::WindowEvent::WindowResized)
	{
		//this->Update();
	}
}

#endif //#if DKGL_USE_METAL
