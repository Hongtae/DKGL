//
//  File: CopyCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKCopyCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class CopyCommandEncoder : public DKCopyCommandEncoder
			{
			public:
                CopyCommandEncoder(ID3D12GraphicsCommandList*, class CommandBuffer*);
				~CopyCommandEncoder();


				void EndEncoding() override;
				DKCommandBuffer* CommandBuffer() override;
                bool IsCompleted() const override;

				ComPtr<ID3D12GraphicsCommandList> commandList;
				DKObject<class CommandBuffer> commandBuffer;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
