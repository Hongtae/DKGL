//
//  File: Buffer.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
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

void* Buffer::Contents()
{
    return [buffer contents];
}

void Buffer::Flush()
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    if (buffer.storageMode == MTLStorageModeManaged)
    {
        NSRange range = NSMakeRange(0, buffer.length);
        [buffer didModifyRange:range];
    }
#endif
}

size_t Buffer::Length() const
{
    return buffer.length;
}

#endif //#if DKGL_ENABLE_METAL
