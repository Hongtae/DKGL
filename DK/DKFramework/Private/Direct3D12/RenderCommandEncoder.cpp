//
//  File: RenderCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "RenderCommandEncoder.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

RenderCommandEncoder::RenderCommandEncoder(ID3D12CommandList* cm)
	: commandList(cm)
{
}

RenderCommandEncoder::~RenderCommandEncoder(void)
{
}

void RenderCommandEncoder::EndEncoding(void)
{
}

DKGraphicsDevice* RenderCommandEncoder::Device(void)
{
	return commandBuffer->Queue()->Device();
}

#endif //#if DKGL_USE_DIRECT3D
