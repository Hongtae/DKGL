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
#include "ShaderFunction.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ShaderModule::ShaderModule(DKGraphicsDevice* d, VkShaderModule s, const void* data, size_t size, DKShader::StageType st)
	: device(d)
	, module(s)
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

	spirv_cross::Compiler compiler(reinterpret_cast<const uint32_t*>(data), size / sizeof(uint32_t));
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();
	 
	auto GetLayout = [&compiler](const spirv_cross::Resource& resource, DKArray<DescriptorSetLayout>& layout)
	{
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		DKStringU8 name = "";
		if (resource.name.size() > 0)
			name = resource.name.c_str();
		else
			name = compiler.get_fallback_name(resource.id).c_str();

		const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);

		DescriptorSetLayout& descriptorSet = layout.Value(set);
		DescriptorSetLayout::Binding descriptor = { name, binding, 0 };
	};

	// get pushConstant range.
	if (resources.push_constant_buffers.size() > 0)
	{
		const spirv_cross::Resource& resource = resources.push_constant_buffers[0];
		std::vector<spirv_cross::BufferRange> ranges = compiler.get_active_buffer_ranges(resource.id);
		this->pushConstantLayout.members.Reserve(ranges.size());
		for (spirv_cross::BufferRange &range : ranges)
		{
			// get range.
			PushConstantLayout::Member member;
			member.name = compiler.get_member_name(resource.id, range.index).c_str();
			member.offset = range.offset;
			member.size = range.range;
			this->pushConstantLayout.members.Add(std::move(member));
		}
		if (resource.name.size() > 0)
			this->pushConstantLayout.name = resource.name.c_str();
		else
			this->pushConstantLayout.name = compiler.get_fallback_name(resource.id).c_str();

		const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);
	}

	// Get all sampled images in the shader.
	for (auto &resource : resources.sampled_images)
	{
		unsigned set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		DKLog("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

	}
	std::vector<std::string> entryPointNames = compiler.get_entry_points();
	functionNames.Reserve(entryPointNames.size());
	for (std::string& name : entryPointNames)
	{
		functionNames.Add(name.c_str());
	}

	// specialization constants
	std::vector<spirv_cross::SpecializationConstant> spConsts = compiler.get_specialization_constants();
	for (spirv_cross::SpecializationConstant& sc : spConsts)
	{
		// 
	}
}

ShaderModule::~ShaderModule(void)
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
