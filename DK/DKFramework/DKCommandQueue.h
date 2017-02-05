//
//  File: DKCommandQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKRenderPass.h"
#include "DKCommandBuffer.h"

namespace DKFramework
{
	class DKGraphicsDevice;
	class DKWindow;

	class DKSwapChain
	{
	public:
		virtual ~DKSwapChain(void) {}

		virtual void SetColorPixelFormat(DKPixelFormat) = 0;
		virtual void SetDepthStencilPixelFormat(DKPixelFormat) = 0;
		virtual DKRenderPassDescriptor CurrentRenderPassDescriptor(void) = 0;

		virtual bool Present(void) = 0;
	};
	/// @brief GPU command queue
	class DKCommandQueue
	{
	public:
		virtual ~DKCommandQueue(void) {}

		virtual DKObject<DKCommandBuffer> CreateCommandBuffer(void) = 0;
		virtual DKObject<DKSwapChain> CreateSwapChain(DKWindow*) = 0;

		virtual DKGraphicsDevice* Device(void) = 0;
	};

	inline DKGraphicsDevice* DKCommandBuffer::Device(void)
	{
		return Queue()->Device();
	}
}
