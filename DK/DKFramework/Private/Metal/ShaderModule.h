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

#include "../../DKShaderModule.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Metal
{
	class ShaderModule : public DKShaderModule
	{
	public:
		ShaderModule(DKGraphicsDevice*, id<MTLLibrary>);
		~ShaderModule();

		const DKArray<DKString>& FunctionNames() const override { return functionNames; }

		DKObject<DKShaderFunction> CreateFunction(const DKString& name) const override;
		DKObject<DKShaderFunction> CreateSpecializedFunction(const DKString& name, const DKShaderSpecialization* values, size_t numValues) const override;

		DKGraphicsDevice* Device() override { return device; }

		id<MTLLibrary> library;
		DKObject<DKGraphicsDevice> device;
		DKArray<DKString> functionNames;
	};
}
#endif //#if DKGL_ENABLE_METAL
