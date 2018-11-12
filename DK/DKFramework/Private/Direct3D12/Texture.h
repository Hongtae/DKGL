//
//  File: Texture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKTexture.h"
#include "../../DKGraphicsDevice.h"
#include "PixelFormat.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class Texture : public DKTexture
			{
			public:
				Texture(DKGraphicsDevice*, ID3D12Resource*, D3D12_CPU_DESCRIPTOR_HANDLE srv, D3D12_CPU_DESCRIPTOR_HANDLE rtv);
				~Texture();

                uint32_t Width() const override
                {
                    return resource->GetDesc().Width;
                }

                uint32_t Height() const override
                {
                    return resource->GetDesc().Height;
                }

                uint32_t Depth() const override
                {
                    return resource->GetDesc().DepthOrArraySize;
                }

                uint32_t MipmapCount() const override
                {
                    return resource->GetDesc().MipLevels;
                }

                DKTexture::Type TextureType() const override
                {
                    D3D12_RESOURCE_DESC desc = resource->GetDesc();
                    switch (desc.Dimension)
                    {
                    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                        return (desc.DepthOrArraySize > 1) ? Type1DArray : Type1D;
                    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                        return (desc.DepthOrArraySize > 1) ? Type2DArray : Type2D;
                    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                        return Type3D;
                    }
                    return TypeUnknown;
                }

                DKPixelFormat PixelFormat() const override
                {
                    return PixelFormat::To(resource->GetDesc().Format);
                }

				DKObject<DKGraphicsDevice> device;
				D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
                ComPtr<ID3D12Resource> resource;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
