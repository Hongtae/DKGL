//
//  File: BlitCommandEncoder.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#import <Metal/Metal.h>

#include "../../DKBlitCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class BlitCommandEncoder : public DKBlitCommandEncoder
			{
			public:
				BlitCommandEncoder(CommandBuffer*, id<MTLBlitCommandEncoder>);
				~BlitCommandEncoder(void);

				void EndEncoding(void) override;
				DKCommandBuffer* Buffer(void) override;

			private:
				DKObject<CommandBuffer> buffer;
				id<MTLBlitCommandEncoder> encoder;
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
