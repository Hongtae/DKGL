//
//  File: DKRenderPass.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKColor.h"
#include "DKTexture.h"

namespace DKFramework
{
	struct DKRenderPassAttachmentDescriptor
	{
		enum LoadAction
		{
			LoadActionDontCare = 0,
			LoadActionLoad,
			LoadActionClear,
		};
		enum StoreAction
		{
			StoreActionDontCare = 0,
			StoreActionStore,
		};

		DKObject<DKTexture> renderTarget;

		uint32_t mipmapLevel = 0;
		LoadAction loadAction = LoadActionDontCare;
		StoreAction storeAction = StoreActionDontCare;
	};

	struct DKRenderPassColorAttachmentDescriptor : public DKRenderPassAttachmentDescriptor
	{
		DKColor clearColor;
	};

	struct DKRenderPassDepthStencilAttachmentDescriptor : public DKRenderPassAttachmentDescriptor
	{
		float clearDepth = 1.0;
		uint32_t clearStencil = 0;
	};

	/**
	 @brief A render-pass descriptor
	 */
	struct DKRenderPassDescriptor
	{
		DKArray<DKRenderPassColorAttachmentDescriptor> colorAttachments;
		DKRenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;

		size_t numberOfActiveLayers = 0;
	};
}
