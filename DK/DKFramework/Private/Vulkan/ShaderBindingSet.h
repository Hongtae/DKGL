//
//  File: ShaderBindingSet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKShaderBindingSet.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Vulkan
{
    class ShaderBindingSet : public DKShaderBindingSet
    {
    public:
        ShaderBindingSet();
        ~ShaderBindingSet();

    };
}
#endif //#if DKGL_ENABLE_VULKAN
