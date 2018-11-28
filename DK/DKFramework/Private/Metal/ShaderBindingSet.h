//
//  File: ShaderBindingSet.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKShaderBindingSet.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Metal
{
    class ShaderBindingSet : public DKShaderBindingSet
    {
    public:
        ShaderBindingSet();
        ~ShaderBindingSet();

    };
}
#endif //#if DKGL_ENABLE_METAL
