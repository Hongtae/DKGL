//
//  File: ComputeCommandEncoder.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKComputeCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework::Private::Metal
{
	class ComputeCommandEncoder : public DKComputeCommandEncoder
	{
	public:
		ComputeCommandEncoder(CommandBuffer*);
		~ComputeCommandEncoder();

		// DKCommandEncoder overrides
		void EndEncoding() override;
		bool IsCompleted() const override { return reusableEncoder == nullptr; }
		DKCommandBuffer* Buffer() override { return buffer; }

		// DKComputeCommandEncoder

	private:
		using EncoderCommand = DKFunctionSignature<void(id<MTLComputeCommandEncoder>)>;
		struct ReusableEncoder : public ReusableCommandEncoder
		{
			bool EncodeBuffer(id<MTLCommandBuffer> buffer) override
			{
				id<MTLComputeCommandEncoder> encoder = [buffer computeCommandEncoder];
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
