//
//  File: SwapChain.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

#include "SwapChain.h"
#include "Texture.h"
#include "Event.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

SwapChain::SwapChain(CommandQueue* q, DKWindow* w)
: queue(q)
, window(w)
, metalLayer(nil)
, currentDrawable(nil)
, pixelFormat(DKPixelFormat::BGRA8Unorm)
{
	window->AddEventHandler(this, DKFunction(this, &SwapChain::OnWindowEvent), nullptr, nullptr);
}

SwapChain::~SwapChain()
{
	window->RemoveEventHandler(this);

	renderPassDescriptor.colorAttachments.Clear();
	renderPassDescriptor.depthStencilAttachment.renderTarget = nullptr;

	[currentDrawable release];
	[metalLayer release];
}

bool SwapChain::Setup()
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

		void (^setupView)() = ^() {
			this->metalLayer = [[CAMetalLayer layer] retain];
			view.wantsLayer = YES;
			view.layer = this->metalLayer;
			[view setLayer:this->metalLayer];

			NSWindow* window = view.window;
			if (window)
				this->metalLayer.contentsScale = window.backingScaleFactor;
			this->metalLayer.frame = NSRectToCGRect(view.bounds);
		};

		if ([NSThread isMainThread])
		{
			setupView();
		}
		else
		{
			dispatch_sync(dispatch_get_main_queue(), setupView);
		}
	}
#endif

	if (this->metalLayer)
	{
		this->metalLayer.device = this->queue->queue.device;
		this->metalLayer.pixelFormat = Metal::PixelFormat(pixelFormat);

		return true;
	}
	return false;
}


void SwapChain::SetPixelFormat(DKPixelFormat pf)
{
	switch (pf)
	{
		case DKPixelFormat::BGRA8Unorm:
		case DKPixelFormat::BGRA8Unorm_sRGB:
		case DKPixelFormat::RGBA16Float:
			break;
		default:	// invalid format!
			return;
			break;
	}

	if (pixelFormat != pf)
	{
		pixelFormat = pf;
		if (currentDrawable)
			[currentDrawable release];
		currentDrawable = nil;
	}
}

DKPixelFormat SwapChain::PixelFormat() const 
{
    return pixelFormat; 
}

DKRenderPassDescriptor SwapChain::CurrentRenderPassDescriptor()
{
	if (currentDrawable == nil)
	{
		@autoreleasepool
		{
			SetupFrame();
		}
	}

	return renderPassDescriptor;
}

size_t SwapChain::MaximumBufferCount() const 
{
    return this->metalLayer.maximumDrawableCount;
}

bool SwapChain::Present(DKGpuEvent** waitEvents, size_t numEvents)
{
    if (numEvents)
    {
        @autoreleasepool
        {
            id<MTLCommandBuffer> buffer = [queue->queue commandBuffer];

            for (size_t i = 0; i < numEvents; ++i)
            {
                DKASSERT_DEBUG(dynamic_cast<Event*>(waitEvents[i]));
                Event* event = static_cast<Event*>(waitEvents[i]);

                [buffer encodeWaitForEvent:event->event
                                     value:event->NextWaitValue()];
            }

            if (currentDrawable)
                [buffer presentDrawable:currentDrawable];
            [buffer commit];
        }
        return currentDrawable != nil;
    }
    else if (currentDrawable)
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

void SwapChain::SetupFrame()
{
	if (currentDrawable == nil)
	{
		renderPassDescriptor.colorAttachments.Clear();
		renderPassDescriptor.depthStencilAttachment.renderTarget = nullptr;

		this->metalLayer.pixelFormat = Metal::PixelFormat(pixelFormat);

        CGRect frame = this->metalLayer.frame;
        DKASSERT_DEBUG(frame.size.width > 0 && frame.size.height > 0);

		id<CAMetalDrawable> drawable = [this->metalLayer nextDrawable];
		if (drawable)
			currentDrawable = [drawable retain];
	}

	if (currentDrawable)
	{
		renderPassDescriptor.colorAttachments.Clear();
		renderPassDescriptor.depthStencilAttachment.renderTarget = nullptr;

		// setup color-attachment
		id<MTLTexture> texture = currentDrawable.texture;
		DKObject<Texture> renderTarget = DKOBJECT_NEW Texture(queue->Device(), texture);

		DKRenderPassColorAttachmentDescriptor colorAttachment;
		colorAttachment.renderTarget = renderTarget.SafeCast<DKTexture>();
		colorAttachment.clearColor = DKColor(0,0,0,0);
		colorAttachment.loadAction = colorAttachment.LoadActionClear;
		colorAttachment.storeAction = colorAttachment.StoreActionStore; // default for color-attachment

		renderPassDescriptor.colorAttachments.Clear();
		renderPassDescriptor.colorAttachments.Add(colorAttachment);

        // no depth stencil attachment
		renderPassDescriptor.depthStencilAttachment.renderTarget = nullptr;
		renderPassDescriptor.depthStencilAttachment.clearDepth = 1.0;	// default clear-depth value
		renderPassDescriptor.depthStencilAttachment.clearStencil = 0;	// default clear-stencil value
		renderPassDescriptor.depthStencilAttachment.loadAction = DKRenderPassAttachmentDescriptor::LoadActionClear;
		renderPassDescriptor.depthStencilAttachment.storeAction = DKRenderPassAttachmentDescriptor::StoreActionDontCare; // default for depth-stencil
	}
}

void SwapChain::OnWindowEvent(const DKWindow::WindowEvent& e)
{
	if (e.type == DKWindow::WindowEvent::WindowResized)
	{
		//this->Update();
	}
}

#endif //#if DKGL_ENABLE_METAL
