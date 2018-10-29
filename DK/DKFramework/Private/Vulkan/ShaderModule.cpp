//
//  File: ShaderModule.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Types.h"
#include "ShaderModule.h"
#include "ShaderFunction.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ShaderModule::ShaderModule(DKGraphicsDevice* d, VkShaderModule m, const DKShader* s)
	: device(d)
	, module(m)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

    switch (s->Stage())
    {
    case DKShaderStage::Vertex:
        stage = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case DKShaderStage::TessellationControl:
        stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        break;
    case DKShaderStage::TessellationEvaluation:
        stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        break;
    case DKShaderStage::Geometry:
        stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        break;
    case DKShaderStage::Fragment:
        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    case DKShaderStage::Compute:
        stage = VK_SHADER_STAGE_COMPUTE_BIT;
        break;
    default:
        DKASSERT_DEBUG(0);
        break;
    }

    functionNames = s->FunctionNames();
    pushConstantLayouts = s->PushConstantBufferLayouts();
    descriptors = s->Descriptors();
    inputAttributes = s->InputAttributes();
    resources = s->Resources();
}

ShaderModule::~ShaderModule()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	vkDestroyShaderModule(dev->device, module, 0);
}

DKObject<DKShaderFunction> ShaderModule::CreateFunction(const DKString& name) const
{
	for (const DKString& fname : functionNames)
	{
		if (fname.Compare(name) == 0)
		{
			DKObject<ShaderFunction> func = DKOBJECT_NEW ShaderFunction(const_cast<ShaderModule*>(this), DKStringU8(name), 0, 0);
			return func.SafeCast<DKShaderFunction>();
		}
	}

	return NULL;
}

DKObject<DKShaderFunction> ShaderModule::CreateSpecializedFunction(const DKString& name, const DKShaderSpecialization* values, size_t numValues) const
{
	if (values && numValues > 0)
	{
		// TODO: verify values with SPIR-V Cross reflection


	}
	return NULL;
}

#endif //#if DKGL_ENABLE_VULKAN
