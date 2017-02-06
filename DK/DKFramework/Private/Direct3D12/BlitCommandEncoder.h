//
//  File: BlitCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include "d3d12_headers.h"

#include "../../DKBlitCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class BlitCommandEncoder : public DKBlitCommandEncoder
			{
			public:
				BlitCommandEncoder(ID3D12GraphicsCommandList*, CommandBuffer*);
				~BlitCommandEncoder(void);


				void EndEncoding(void) override;
				DKCommandBuffer* Buffer(void) override;

				ComPtr<ID3D12GraphicsCommandList> commandList;
				DKObject<CommandBuffer> commandBuffer;
			};
		}
	}
}

#endif //#if DKGL_USE_DIRECT3D
