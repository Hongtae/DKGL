//
//  File: ShaderFunction.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "../../../Libs/SPIRV-Cross/src/spirv_cross.hpp"
#include "Extensions.h"
#include "ShaderFunction.h"
#include "ShaderModule.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ShaderFunction::ShaderFunction(DKShaderModule* m, const DKStringU8& name, const DKShaderSpecialization* values, size_t numValues)
	: module(m)
	, functionName(name)
	, specializationData(NULL)
	, specializationInfo({ 0 })
{
	DKASSERT_DEBUG(module.SafeCast<ShaderModule>() != NULL);
}

ShaderFunction::~ShaderFunction(void)
{
	if (specializationData)
		DKFree(specializationData);
}


#endif //#if DKGL_ENABLE_VULKAN
