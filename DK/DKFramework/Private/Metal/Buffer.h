//
//  File: Buffer.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKGpuBuffer.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Metal
{
	class Buffer : public DKGpuBuffer
	{
	public:
		Buffer(DKGraphicsDevice*, id<MTLBuffer>);
		~Buffer(void);

		void* Lock(size_t offset, size_t length) override;
		void Unlock(void) override;

		id<MTLBuffer> buffer;
		DKObject<DKGraphicsDevice> device;
	};
}
#endif //#if DKGL_ENABLE_METAL
