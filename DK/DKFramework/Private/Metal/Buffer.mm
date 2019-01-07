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
, buffer([b retain])
{
    DKASSERT_DEBUG(buffer);
}

Buffer::~Buffer()
{
	//GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	[buffer release];
}


void* Buffer::Lock(size_t offset, size_t size)
{
	NSUInteger length = buffer.length;
	if (offset < length)
	{
		if (buffer.contents)
		{
			return &reinterpret_cast<unsigned char*>(buffer.contents)[offset];
		}
		else
		{
			DKLogE("ERROR: DKGpuBuffer::Lock failed: %s", "Unaccessible storage mode");
		}
	}
	else
	{
		DKLogE("ERROR: DKGpuBuffer::Lock failed: %s", "Invalid offset");
	}
	return NULL;
}

void Buffer::Unlock()
{
}

#endif //#if DKGL_ENABLE_METAL
