//
//  File: Texture.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "Texture.h"
#include "PixelFormat.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

Texture::Texture(ID3D12Resource* r, ID3D12DescriptorHeap* h, D3D12_CPU_DESCRIPTOR_HANDLE d)
	: resource(r)
	, descHeap(h)
	, descHandle(d)
{
}

Texture::~Texture(void)
{
}

uint32_t Texture::Width(void)
{
	return resource->GetDesc().Width;
}

uint32_t Texture::Height(void)
{
	return resource->GetDesc().Height;
}

uint32_t Texture::Depth(void)
{
	return resource->GetDesc().DepthOrArraySize;
}

uint32_t Texture::MipmapCount(void)
{
	return resource->GetDesc().MipLevels;
}

DKTexture::Type Texture::TextureType(void)
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

DKPixelFormat Texture::PixelFormat(void)
{
	return PixelFormat::To(resource->GetDesc().Format);
}

#endif //#if DKGL_USE_DIRECT3D
