//
//  File: CopyCommandEncoder.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKCopyCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework::Private::Metal
{
	class CopyCommandEncoder : public DKCopyCommandEncoder
	{
	public:
        CopyCommandEncoder(class CommandBuffer*);
		~CopyCommandEncoder();

		// DKCommandEncoder overrides
		void EndEncoding() override;
		bool IsCompleted() const override { return reusableEncoder == nullptr; }
		DKCommandBuffer* CommandBuffer() override { return commandBuffer; }

		// DKCopyCommandEncoder

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
		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_METAL
