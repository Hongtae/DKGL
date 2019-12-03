//
//  File: Semaphore.h
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

#define DKGL_VULKAN_SEMAPHORE_AUTO_INCREMENTAL_TIMELINE  0

namespace DKFramework::Private::Vulkan
{
    class Semaphore : public DKGpuEvent
    {
#if DKGL_VULKAN_SEMAPHORE_AUTO_INCREMENTAL_TIMELINE
        DKAtomicNumber64 waitValue;
        DKAtomicNumber64 signalValue;
#endif
    public:
        Semaphore(DKGraphicsDevice*, VkSemaphore);
        ~Semaphore();

        DKObject<DKGraphicsDevice> device;
        VkSemaphore semaphore;

#if DKGL_VULKAN_SEMAPHORE_AUTO_INCREMENTAL_TIMELINE
        uint64_t NextWaitValue();
        uint64_t NextSignalValue();
#else
        uint64_t NextWaitValue() { return 0; }
        uint64_t NextSignalValue() { return 0; }
#endif
    };
}

#endif //#if DKGL_ENABLE_VULKAN
