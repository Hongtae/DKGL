//
//  File: RenderTarget.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include "d3d12_headers.h"

#include "../../DKRenderTarget.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class RenderTarget : public DKRenderTarget
			{
			public:
				RenderTarget(ID3D12Resource*, ID3D12DescriptorHeap*, D3D12_CPU_DESCRIPTOR_HANDLE);
				~RenderTarget(void);

				ComPtr<ID3D12Resource> resource;
				ComPtr<ID3D12DescriptorHeap> descHeap;
				D3D12_CPU_DESCRIPTOR_HANDLE descHandle;
			};
		}
	}
}

#endif //#if DKGL_USE_DIRECT3D
