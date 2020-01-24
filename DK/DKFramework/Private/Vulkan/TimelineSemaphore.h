//
//  File: TimelineSemaphore.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKGpuResource.h"

namespace DKFramework::Private::Vulkan
{
    class TimelineSemaphore : public DKGpuSemaphore
    {
    public:
        TimelineSemaphore(DKGraphicsDevice*, VkSemaphore);
        ~TimelineSemaphore();

        DKObject<DKGraphicsDevice> device;
        VkSemaphore semaphore;
    };
}

#endif //#if DKGL_ENABLE_VULKAN
