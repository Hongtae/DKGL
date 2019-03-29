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

namespace DKFramework
{
	class DKSwapChain
	{
	public:
		virtual ~DKSwapChain() {}

		virtual DKPixelFormat ColorPixelFormat() const = 0;
		virtual DKPixelFormat DepthStencilPixelFormat() const = 0;
		virtual void SetColorPixelFormat(DKPixelFormat) = 0;
		virtual void SetDepthStencilPixelFormat(DKPixelFormat) = 0;
		virtual DKRenderPassDescriptor CurrentRenderPassDescriptor() = 0;

		virtual bool Present() = 0;
	};
}
