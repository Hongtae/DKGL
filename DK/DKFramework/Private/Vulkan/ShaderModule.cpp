//
//  File: ShaderModule.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "../../../Libs/SPIRV-Cross/src/spirv_cross.hpp"
#include "Extensions.h"
#include "ShaderModule.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ShaderModule::ShaderModule(DKGraphicsDevice* d, VkShaderModule s, const void* data, size_t size, DKShader::StageType st, const DKStringU8& entry)
	: device(d)
	, module(s)
	, entryPoint(entry)
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
	spirv_cross::Compiler comp(reinterpret_cast<const uint32_t*>(data), size);

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

ShaderModule::~ShaderModule(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	vkDestroyShaderModule(dev->device, module, 0);
}

#endif //#if DKGL_ENABLE_VULKAN
