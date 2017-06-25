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
#include "ShaderDataType.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ShaderFunction::ShaderFunction(DKGraphicsDevice* dev, id<MTLLibrary> lib, id<MTLFunction> func)
: library(nil)
, function(nil)
, device(dev)
{
    library = [lib retain];
    function = [func retain];

	vertexAttributes.Reserve(function.vertexAttributes.count);
	for (MTLVertexAttribute* inAttr in function.vertexAttributes)
	{
		DKVertexAttribute attr;
		attr.name = DKString((const char*)inAttr.name.UTF8String);
		attr.index = (uint32_t)inAttr.attributeIndex;
		attr.type = ShaderDataType::To(inAttr.attributeType);
		attr.active = inAttr.active;
		attr.patchControlPointData = inAttr.patchControlPointData;
		attr.patchData = inAttr.patchData;
		vertexAttributes.Add(attr);
	}
	stageInputAttributes.Reserve(function.stageInputAttributes.count);
	for (MTLAttribute* inAttr in function.stageInputAttributes)
	{
		DKVertexAttribute attr;
		attr.name = DKString((const char*)inAttr.name.UTF8String);
		attr.index = (uint32_t)inAttr.attributeIndex;
		attr.type = ShaderDataType::To(inAttr.attributeType);
		attr.active = inAttr.active;
		attr.patchControlPointData = inAttr.patchControlPointData;
		attr.patchData = inAttr.patchData;
		stageInputAttributes.Add(attr);
	}
}

ShaderFunction::~ShaderFunction(void)
{
    [library release];
    [function release];
}

#endif //#if DKGL_ENABLE_METAL
