//
//  File: ShaderFunction.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKShaderFunction.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
    namespace Private
    {
        namespace Metal
        {
            class ShaderFunction : public DKShaderFunction
            {
            public:
                ShaderFunction(DKGraphicsDevice*, id<MTLLibrary>, id<MTLFunction>);
                ~ShaderFunction(void);

				const DKArray<DKVertexAttribute> VertexAttributes(void) const override { return vertexAttributes;}
				const DKArray<DKShaderAttribute> StageInputAttributes(void) const override { return stageInputAttributes;}

                DKGraphicsDevice* Device(void) override { return device; }

                id<MTLLibrary> library;
                id<MTLFunction> function;
                DKObject<DKGraphicsDevice> device;

				DKArray<DKVertexAttribute> vertexAttributes;
				DKArray<DKShaderAttribute> stageInputAttributes;
            };
        }
    }
}

#endif //#if DKGL_ENABLE_METAL
