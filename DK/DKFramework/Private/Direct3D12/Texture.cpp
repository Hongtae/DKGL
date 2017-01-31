//
//  File: Texture.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "Texture.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

Texture::Texture(DKGraphicsDevice* dev, ID3D12Resource* res, D3D12_CPU_DESCRIPTOR_HANDLE srv)
	: TextureBaseT(res)
	, device(dev)
	, srvHandle(srv)
{
}

Texture::~Texture(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	if (srvHandle.ptr)
		dev->ReleaseDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srvHandle);
}

#endif //#if DKGL_USE_DIRECT3D
