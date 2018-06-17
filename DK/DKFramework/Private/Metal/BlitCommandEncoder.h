//
//  File: BlitCommandEncoder.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKBlitCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework::Private::Metal
{
	class BlitCommandEncoder : public DKBlitCommandEncoder
	{
	public:
		BlitCommandEncoder(CommandBuffer*);
		~BlitCommandEncoder(void);

		// DKCommandEncoder overrides
		void EndEncoding(void) override;
		bool IsCompleted(void) const override { return reusableEncoder == nullptr; }
		DKCommandBuffer* Buffer(void) override { return buffer; }

		// DKBlitCommandEncoder

	private:
		using EncoderCommand = DKFunctionSignature<void(id<MTLBlitCommandEncoder>)>;
		struct ReusableEncoder : public ReusableCommandEncoder
		{
			bool EncodeBuffer(id<MTLCommandBuffer> buffer) override
			{
				id<MTLBlitCommandEncoder> encoder = [buffer blitCommandEncoder];
				for (EncoderCommand* command : encoderCommands )
				{
					command->Invoke(encoder);
				}
				[encoder endEncoding];
				return true;
			}
			DKArray<DKObject<EncoderCommand>> encoderCommands;
		};
		DKObject<ReusableEncoder> reusableEncoder;
		DKObject<CommandBuffer> buffer;
	};
}
#endif //#if DKGL_ENABLE_METAL
