//
//  File: TextureBaseT.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include "d3d12_headers.h"

#include "PixelFormat.h"
#include "../../DKTexture.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			template <typename BaseClassT> struct TextureBaseT : public BaseClassT
			{
				uint32_t Width(void) override
				{
					return resource->GetDesc().Width;
				}
				uint32_t Height(void) override
				{
					return resource->GetDesc().Height;
				}
				uint32_t Depth(void) override
				{
					return resource->GetDesc().DepthOrArraySize;
				}
				uint32_t MipmapCount(void) override
				{
					return resource->GetDesc().MipLevels;
				}
				DKTexture::Type TextureType(void) override
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
				DKPixelFormat PixelFormat(void) override
				{
					return PixelFormat::To(resource->GetDesc().Format);
				}

				TextureBaseT(ID3D12Resource* r) : resource(r) {}
				ComPtr<ID3D12Resource> resource;
			};

			using Texture = TextureBaseT<DKTexture>;
		}
	}
}

#endif //#if DKGL_USE_METAL

