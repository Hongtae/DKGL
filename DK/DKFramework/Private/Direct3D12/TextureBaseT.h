//
//  File: TextureBaseT.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKTexture.h"
#include "PixelFormat.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			template <typename BaseClassT> struct TextureBaseT : public BaseClassT
			{
				static_assert(DKTypeConversionTest<BaseClassT, DKTexture>(), "BaseClassT must be DKTexture or DKTexture subclass");

				ComPtr<ID3D12Resource> resource;
				TextureBaseT(ID3D12Resource* tex) : resource(tex) {}

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
			};
		}
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
