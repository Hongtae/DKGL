//
//  File: ShaderFunction.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "ShaderModule.h"
#include "ShaderFunction.h"
#include "Types.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ShaderModule::ShaderModule(DKGraphicsDevice* dev, id<MTLLibrary> lib)
: library(nil)
, device(dev)
, workgroupSize({1,1,1})
, bindings({})
{
	DKASSERT_DEBUG(lib != nil);
	library = [lib retain];

	@autoreleasepool {
		NSArray<NSString*> *names = [library functionNames];
		this->functionNames.Reserve(names.count);

		for (NSString* name in names)
		{
			this->functionNames.Add(name.UTF8String);
		}
	}
}

ShaderModule::~ShaderModule()
{
	[library release];
}

DKObject<DKShaderFunction> ShaderModule::CreateFunction(const DKString& name) const
{
	@autoreleasepool {
		NSString* functionName = [NSString stringWithUTF8String:(const char*)DKStringU8(name)];
		id<MTLFunction> func = [library newFunctionWithName:functionName];
		if (func)
		{
			return DKOBJECT_NEW ShaderFunction(const_cast<ShaderModule*>(this), func, workgroupSize);
		}
	}
	return NULL;
}

DKObject<DKShaderFunction> ShaderModule::CreateSpecializedFunction(const DKString& name, const DKShaderSpecialization* values, size_t numValues) const
{
	if (values && numValues > 0)
	{
		@autoreleasepool {
			NSError* error = nil;
			NSString* funcName = [NSString stringWithUTF8String:(const char *)DKStringU8(name)];
			MTLFunctionConstantValues* constantValues = [[[MTLFunctionConstantValues alloc] init] autorelease];
			for (size_t i = 0; i < numValues; ++i)
			{
				const DKShaderSpecialization& sp = values[i];
				[constantValues setConstantValues:sp.data
											 type:ShaderDataType(sp.type)
										withRange:NSMakeRange(sp.index, sp.size)];
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
				return DKOBJECT_NEW ShaderFunction(const_cast<ShaderModule*>(this), func, workgroupSize);
			}
		}
	}
	return NULL;
}

#endif //#if DKGL_ENABLE_METAL
