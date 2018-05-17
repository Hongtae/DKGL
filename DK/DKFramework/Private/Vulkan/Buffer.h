//
//  File: Buffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2018 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKGpuBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class Buffer : public DKGpuBuffer
			{
			public:
				Buffer(DKGraphicsDevice*, VkBuffer, VkBufferView, VkDeviceMemory, VkMemoryType, size_t);
				~Buffer(void);

				void* Lock(size_t offset, size_t length) override;
				void Unlock(void) override;

				DKSpinLock lock;
				void* mapped;
				size_t length;
				VkBuffer buffer;
				VkBufferView view;
				VkDeviceMemory memory;
				VkMemoryType memoryType;
				DKObject<DKGraphicsDevice> device;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_VULKAN
