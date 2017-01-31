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

RenderTarget::RenderTarget(ID3D12Resource* r)
	: resource(r)
{
}

RenderTarget::~RenderTarget(void)
{
}

#endif //#if DKGL_USE_DIRECT3D
