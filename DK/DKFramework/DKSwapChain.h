//
//  File: DKSwapChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKRenderPass.h"
#include "DKPixelFormat.h"
#include "DKGpuResource.h"

namespace DKFramework
{
    class DKCommandQueue;
	class DKSwapChain
	{
	public:
		virtual ~DKSwapChain() {}

		virtual DKPixelFormat PixelFormat() const = 0;
		virtual void SetPixelFormat(DKPixelFormat) = 0;

		virtual DKRenderPassDescriptor CurrentRenderPassDescriptor() = 0;
        virtual size_t MaximumBufferCount() const = 0;

        virtual DKCommandQueue* Queue() = 0;

		virtual bool Present(DKGpuEvent** waitEvents, size_t numEvents) = 0;

        bool Present() { return Present(nullptr, 0); }
	};
}
