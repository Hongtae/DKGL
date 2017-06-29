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
	DKASSERT_DEBUG(lib != nil);
	DKASSERT_DEBUG(func != nil);

    library = [lib retain];
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
    [library release];
    [function release];
}

DKObject<DKShaderFunction> ShaderFunction::CreateSpecializedFunction(const DKShaderSpecialization* values, size_t numValues) const
{
	if (values && numValues > 0)
	{
		@autoreleasepool {
			NSError* error = nil;
			NSString* funcName = function.name;
			MTLFunctionConstantValues* constantValues = [[[MTLFunctionConstantValues alloc] init] autorelease];
			for (size_t i = 0; i < numValues; ++i)
			{
				const DKShaderSpecialization& sp = values[i];
				[constantValues setConstantValues:sp.data
											 type:ShaderDataType::From(sp.type)
										withRange:NSMakeRange(sp.offset, sp.size)];
			}

			id<MTLFunction> func = [library newFunctionWithName:funcName
												 constantValues:constantValues
														  error:&error];
			if (error)
			{
				NSLog(@"MTLLibrary newFunction error!: %@", error.localizedDescription);
			}
			if (func)
			{
				DKObject<DKGraphicsDevice> dev = this->device;
				return DKOBJECT_NEW ShaderFunction(dev, library, func);
			}
		}
	}
	return NULL;
}

#endif //#if DKGL_ENABLE_METAL
