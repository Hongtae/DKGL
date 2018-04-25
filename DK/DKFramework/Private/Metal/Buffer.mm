//
//  File: Buffer.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "Buffer.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

Buffer::Buffer(DKGraphicsDevice* d, id<MTLBuffer> b)
: device(d)
, buffer(b)
{
}

Buffer::~Buffer(void)
{
	//GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	[buffer release];
}

#endif //#if DKGL_ENABLE_METAL
