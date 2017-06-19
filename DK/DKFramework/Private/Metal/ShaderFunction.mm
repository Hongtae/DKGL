//
//  File: ShaderFunction.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "ShaderFunction.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ShaderFunction::ShaderFunction(DKGraphicsDevice* dev, id<MTLLibrary> lib, id<MTLFunction> func)
: library(nil)
, function(nil)
, device(dev)
{
    library = [lib retain];
    function = [func retain];
}

ShaderFunction::~ShaderFunction(void)
{
    [library release];
    [function release];
}

#endif //#if DKGL_ENABLE_METAL
