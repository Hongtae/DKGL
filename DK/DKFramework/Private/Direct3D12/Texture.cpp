//
//  File: Texture.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12

#include "Texture.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

Texture::Texture(DKGraphicsDevice* dev, ID3D12Resource* res, D3D12_CPU_DESCRIPTOR_HANDLE srv, D3D12_CPU_DESCRIPTOR_HANDLE rtv)
	: resource(res)
	, device(dev)
	, srvHandle(srv)
    , rtvHandle(rtv)
{
	DKASSERT_DEBUG((resource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) == 0);
}

Texture::~Texture()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	if (srvHandle.ptr)
		dev->ReleaseDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srvHandle);
    if (rtvHandle.ptr)
        dev->ReleaseDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvHandle);
}

#endif //#if DKGL_ENABLE_DIRECT3D12
