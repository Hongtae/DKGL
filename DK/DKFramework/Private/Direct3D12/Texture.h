//
//  File: Texture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include "d3d12_headers.h"

#include "../../DKTexture.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class Texture : public DKTexture
			{
			public:
				Texture(ID3D12Resource*, ID3D12DescriptorHeap*, D3D12_CPU_DESCRIPTOR_HANDLE);
				~Texture(void);

				uint32_t Width(void) override;
				uint32_t Height(void) override;
				uint32_t Depth(void) override;
				uint32_t MipmapCount(void) override;

				Type TextureType(void) override;
				DKPixelFormat PixelFormat(void) override;

				ComPtr<ID3D12Resource> resource;
				ComPtr<ID3D12DescriptorHeap> descHeap;
				D3D12_CPU_DESCRIPTOR_HANDLE descHandle;
			};
		}
	}
}

#endif //#if DKGL_USE_DIRECT3D
