//
//  File: RenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKRenderCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class RenderCommandEncoder : public DKRenderCommandEncoder
			{
			public:
				RenderCommandEncoder(ID3D12GraphicsCommandList*, CommandBuffer*, const DKRenderPassDescriptor&);
				~RenderCommandEncoder();


				void EndEncoding() override;
				DKCommandBuffer* Buffer() override;

				ComPtr<ID3D12GraphicsCommandList> commandList;
				DKObject<CommandBuffer> commandBuffer;

			private:
				DKArray<D3D12_RESOURCE_BARRIER> rtBarriers;
				DKRenderPassDescriptor renderPassDesc;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
