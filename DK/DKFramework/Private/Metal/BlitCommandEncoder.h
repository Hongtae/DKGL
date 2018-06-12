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
	class BlitCommandEncoder : public DKBlitCommandEncoder, public ReusableCommandEncoder
	{
	public:
		BlitCommandEncoder(CommandBuffer*);
		~BlitCommandEncoder(void);

		// DKCommandEncoder overrides
		void EndEncoding(void) override;
		bool IsCompleted(void) const override { return buffer == nullptr; }
		DKCommandBuffer* Buffer(void) override { return buffer; }

		// DKBlitCommandEncoder


		// ReusableCommandEncoder overrides
		bool EncodeBuffer(id<MTLCommandBuffer>) override;
		void CompleteBuffer(void) override { buffer = nullptr; }

	private:
		DKObject<CommandBuffer> buffer;

		using EncoderCommand = DKFunctionSignature<void(id<MTLBlitCommandEncoder>)>;
		DKArray<DKObject<EncoderCommand>> encoderCommands;
	};
}
#endif //#if DKGL_ENABLE_METAL
