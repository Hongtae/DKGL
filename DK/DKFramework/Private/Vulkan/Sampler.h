//
//  File: Sampler.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKSampler.h"

namespace DKFramework::Private::Vulkan
{
    class Sampler : public DKSamplerState
    {
    public:
        Sampler(DKGraphicsDevice*, VkSampler);
        ~Sampler();

        DKObject<DKGraphicsDevice> device;
        VkSampler sampler;
    };
}

#endif //#if DKGL_ENABLE_VULKAN
