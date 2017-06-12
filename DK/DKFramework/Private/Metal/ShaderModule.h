//
//  File: ShaderModule.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKShaderModule.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
    namespace Private
    {
        namespace Metal
        {
            class ShaderModule : public DKShaderModule
            {
            public:
                ShaderModule(DKGraphicsDevice*, id<MTLLibrary>, id<MTLFunction>);
                ~ShaderModule(void);

                DKGraphicsDevice* Device(void) override { return device; }

                id<MTLLibrary> library;
                id<MTLFunction> entryPoint;
                DKObject<DKGraphicsDevice> device;
            };
        }
    }
}

#endif //#if DKGL_ENABLE_METAL
