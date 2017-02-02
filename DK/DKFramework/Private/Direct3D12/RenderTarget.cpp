//
//  File: RenderTarget.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "RenderTarget.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

RenderTarget::RenderTarget(GraphicsDevice* dev, ID3D12Resource* res, D3D12_CPU_DESCRIPTOR_HANDLE srv, D3D12_CPU_DESCRIPTOR_HANDLE rtv)
	: TextureBaseT(res)
	, device(dev)
	, srvHandle(srv)
	, rtvHandle(rtv)
{
}

RenderTarget::~RenderTarget(void)
{
	if (srvHandle.ptr)
		device->ReleaseDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srvHandle);
	if (rtvHandle.ptr)
		device->ReleaseDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvHandle);
}

#endif //#if DKGL_USE_DIRECT3D
