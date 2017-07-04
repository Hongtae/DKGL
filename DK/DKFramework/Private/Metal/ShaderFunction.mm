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
#include "ShaderModule.h"
#include "ShaderDataType.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ShaderFunction::ShaderFunction(DKShaderModule* sm, id<MTLFunction> func)
: module(sm)
, function(nil)
{
	DKASSERT_DEBUG(func != nil);

    function = [func retain];

	NSDictionary<NSString*, MTLFunctionConstant*>* constants = function.functionConstantsDictionary;
	NSLog(@"functionConstantsDictionary: %@", constants);
	[constants enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, MTLFunctionConstant * _Nonnull obj, BOOL * _Nonnull stop) {
		DKShaderFunction::Constant c;
		c.name = obj.name.UTF8String;
		c.type = ShaderDataType::To(obj.type);
		c.index = obj.index;
		c.required = obj.required;
		this->functionConstantsMap.Insert(DKString(key.UTF8String), c);
	}];

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
    [function release];
}

DKString ShaderFunction::FunctionName(void) const 
{
	DKString name;
	@autoreleasepool
	{
		name = function.name.UTF8String;
	}
	return name;
}

#endif //#if DKGL_ENABLE_METAL
