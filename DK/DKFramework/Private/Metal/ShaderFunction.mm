//
//  File: ShaderFunction.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "ShaderFunction.h"
#include "ShaderModule.h"
#include "Types.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ShaderFunction::ShaderFunction(DKShaderModule* sm, id<MTLFunction> func, MTLSize wgSize, const DKString& spirvName)
: module(sm)
, function(nil)
, name(spirvName)
, workgroupSize(wgSize)
{
	DKASSERT_DEBUG(func != nil);

    function = [func retain];

	NSDictionary<NSString*, MTLFunctionConstant*>* constants = function.functionConstantsDictionary;
	NSLog(@"functionConstantsDictionary: %@", constants);
	[constants enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, MTLFunctionConstant * _Nonnull obj, BOOL * _Nonnull stop) {
		DKShaderFunction::Constant c;
		c.name = obj.name.UTF8String;
		c.type = ShaderDataType(obj.type);
		c.index = (uint32_t)obj.index;
		c.required = obj.required;
		this->functionConstantsMap.Insert(DKString(key.UTF8String), c);
	}];

	stageInputAttributes.Reserve(function.stageInputAttributes.count);
	for (MTLAttribute* inAttr in function.stageInputAttributes)
	{
		DKShaderAttribute attr;
		attr.name = DKString((const char*)inAttr.name.UTF8String);
		attr.location = (uint32_t)inAttr.attributeIndex;
		attr.type = ShaderDataType(inAttr.attributeType);
		attr.enabled = inAttr.active;
//        attr.patchControlPointData = inAttr.patchControlPointData;
//        attr.patchData = inAttr.patchData;
		stageInputAttributes.Add(attr);
	}
}

ShaderFunction::~ShaderFunction()
{
    [function release];
}

DKString ShaderFunction::FunctionName() const 
{
	return name;
}

DKShaderStage ShaderFunction::Stage() const
{
    switch (MTLFunctionType type = function.functionType; type)
    {
        case MTLFunctionTypeVertex:
            return DKShaderStage::Vertex;
        case MTLFunctionTypeFragment:
            return DKShaderStage::Fragment;
        case MTLFunctionTypeKernel:
            return DKShaderStage::Compute;
    }
    return DKShaderStage::Unknown;
}

#endif //#if DKGL_ENABLE_METAL
