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
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ShaderFunction::ShaderFunction(DKGraphicsDevice* d, VkShaderModule s, const void* data, size_t size, DKShader::StageType st, const DKStringU8& entry)
	: device(d)
	, module(s)
	, entryPoint(entry)
	, parent(NULL)
	, specializationData(NULL)
	, specializationInfo({ 0 })
{
	switch (st)
	{
	case DKShader::StageType::Vertex:
		stage = VK_SHADER_STAGE_VERTEX_BIT; break;
	case DKShader::StageType::TessellationControl:
		stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
	case DKShader::StageType::TessellationEvaluation:
		stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
	case DKShader::StageType::Geometry:
		stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
	case DKShader::StageType::Fragment:
		stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
	case DKShader::StageType::Compute:
		stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
	default:
		DKASSERT_DEBUG(0);
		break;
	}

	// SPIRV-Cross test!
	spirv_cross::Compiler comp(reinterpret_cast<const uint32_t*>(data), size / sizeof(uint32_t));

	// The SPIR-V is now parsed, and we can perform reflection on it.
	spirv_cross::ShaderResources resources = comp.get_shader_resources();

	// Get all sampled images in the shader.
	for (auto &resource : resources.sampled_images)
	{
		unsigned set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		DKLog("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

		// Modify the decoration to prepare it for GLSL.
		comp.unset_decoration(resource.id, spv::DecorationDescriptorSet);

		// Some arbitrary remapping if we want.
		comp.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
	}

}

ShaderFunction::ShaderFunction(ShaderFunction* p, const DKShaderSpecialization* values, size_t numValues)
	: parent(p)
	, specializationData(NULL)
	, specializationInfo({ 0 })
{
	DKASSERT_DEBUG(parent != NULL);

	device = parent->device;
	module = parent->module;
	entryPoint = parent->entryPoint;
	stage = parent->stage;
}

ShaderFunction::~ShaderFunction(void)
{
	if (parent == NULL)
	{
		GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
		vkDestroyShaderModule(dev->device, module, 0);
	}

	if (specializationData)
		DKFree(specializationData);
}

DKObject<DKShaderFunction> ShaderFunction::CreateSpecializedFunction(const DKShaderSpecialization* values, size_t numValues) const
{
	const ShaderFunction* parentObject = this->parent;
	if (parentObject == NULL)
		parentObject = this;

	if (values && numValues > 0)
	{
		// TODO: verify values with SPIR-V Cross reflection


		DKObject<ShaderFunction> func = DKOBJECT_NEW ShaderFunction(const_cast<ShaderFunction*>(parentObject), values, numValues);
		return func.SafeCast<DKShaderFunction>();
	}
	return NULL;
}

#endif //#if DKGL_ENABLE_VULKAN
