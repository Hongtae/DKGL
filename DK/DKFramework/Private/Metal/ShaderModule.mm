//
//  File: ShaderModule.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "ShaderModule.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ShaderModule::ShaderModule(DKGraphicsDevice* dev, id<MTLLibrary> lib, id<MTLFunction> func)
: library(nil)
, entryPoint(nil)
, device(dev)
{
    library = [lib retain];
    entryPoint = [func retain];
}

ShaderModule::~ShaderModule(void)
{
    [library release];
    [entryPoint release];
}

#endif //#if DKGL_ENABLE_METAL
