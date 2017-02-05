//
//  File: RenderCommandEncoder.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#import <Metal/Metal.h>

#include "../../DKRenderCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class RenderCommandEncoder : public DKRenderCommandEncoder
			{
			public:
				RenderCommandEncoder(CommandBuffer*, id<MTLRenderCommandEncoder>);
				~RenderCommandEncoder(void);

				void EndEncoding(void) override;
				DKCommandBuffer* Buffer(void) override;

			private:
				DKObject<CommandBuffer> buffer;
				id<MTLRenderCommandEncoder> encoder;
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
