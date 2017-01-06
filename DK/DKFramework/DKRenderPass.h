//
//  File: DKRenderPass.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"


namespace DKFramework
{
	class DKRenderPassAttachmentDescriptor
	{
	};

	class DKRenderPassColorAttachmentDescriptor : public DKRenderPassAttachmentDescriptor
	{
	};

	class DKRenderPassStencilAttachmentDescriptor : public DKRenderPassAttachmentDescriptor
	{
	};

	/**
	 @brief A render-pass descriptor
	 */
	class DKRenderPassDescriptor
	{
	};
}
