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

ShaderModule::ShaderModule(DKGraphicsDevice* d, VkShaderModule s, const void* data, size_t size, DKShader::StageType st)
	: device(d)
	, module(s)
	, pushConstantLayout({})
{
	switch (st)
	{
	case DKShader::StageType::Vertex:
		stage = VK_SHADER_STAGE_VERTEX_BIT; break;
	case DKShader::StageType::Fragment:
		stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
	case DKShader::StageType::Compute:
		stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
	default:
		DKASSERT_DEBUG(0);
		break;
	}

	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

	spirv_cross::Compiler compiler(reinterpret_cast<const uint32_t*>(data), size / sizeof(uint32_t));
	auto active = compiler.get_active_interface_variables();
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

		if (layout.Count() <= set)
		{
			layout.Resize(set - layout.Count() + 1);
		}
		// get item count! (array size)
		uint32_t count = 1;
		if (spType.array.size() > 0)
		{
			for (auto i : spType.array)
				count *= i;
		}

		DescriptorSetLayout& descriptorSet = layout.Value(set);
		DescriptorSetLayout::Binding descriptor = { name, binding, count };
		descriptorSet.bindings.Add(descriptor);
	};

	auto GetResource = [&compiler, &active](const spirv_cross::Resource& resource, bool writable)->DKShaderResource
	{
		DKShaderResource out = {};
		out.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		out.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		out.name = compiler.get_name(resource.id).c_str();
		out.stride = compiler.get_decoration(resource.id, spv::DecorationArrayStride);
		out.writable = writable;
		out.enabled = active.find(resource.id) != active.end();

		const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
		out.count = 1;
		for (auto n : type.array)
			out.count = out.count * n;

		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Image:
		case spirv_cross::SPIRType::SampledImage:
			out.type = DKShaderResource::TypeTexture;
			break;
		case spirv_cross::SPIRType::Sampler:
			out.type = DKShaderResource::TypeSampler;
			break;
		case spirv_cross::SPIRType::Struct:
			out.type = DKShaderResource::TypeBuffer;
			break;
		default:
			DKASSERT_DESC_DEBUG(0, "Should implement this!");
			DKLogE("ERROR: Unsupported SPIR-V type!");
		}

		if (out.type == DKShaderResource::TypeBuffer)
		{
			struct GetStructMembers
			{
				DKShaderResource& base; // buffer resource (base)
				spirv_cross::Compiler& compiler;
				auto operator () (const DKString& name, const spirv_cross::SPIRType& spType) -> DKShaderResourceStruct
				{
					DKShaderResourceStruct rst = {};
					// get struct members!
					rst.members.Reserve(spType.member_types.size());
					for (uint32_t i = 0; i < spType.member_types.size(); ++i)
					{
						DKShaderResourceStructMember member;
						uint32_t type = spType.member_types.at(i);
						const spirv_cross::SPIRType& memberType = compiler.get_type(type);
						member.dataType = ShaderDataTypeFromSPIRType(memberType);
						DKASSERT_DEBUG(member.dataType != DKShaderDataType::Unknown);
						DKASSERT_DEBUG(member.dataType != DKShaderDataType::None);

						if (member.dataType == DKShaderDataType::Struct)
						{
							member.typeInfoKey = DKString(name).Append(".").Append(member.name);
							GetStructMembers getMembers = { base, compiler };
							DKShaderResourceStruct memberStruct = getMembers(member.typeInfoKey, memberType);
							base.structTypeMemberMap.Update(member.typeInfoKey, memberStruct);
						}

						member.count = 1;
						for (auto n : spType.array)
							member.count = member.count * n;

						member.name = compiler.get_member_name(spType.self, i).c_str();
						member.offset = compiler.type_struct_member_offset(spType, i);
						if (member.count > 1)
							member.stride = compiler.type_struct_member_array_stride(spType, i);
						//	member.size = (memberType.width >> 3) * memberType.vecsize * memberType.columns;
						member.size = compiler.get_declared_struct_member_size(spType, i);
						DKASSERT_DEBUG(member.size > 0);

						rst.members.Add(member);
					}
					rst.members.ShrinkToFit();
					return rst;
				}
			};
			out.typeInfoKey = out.name;
			GetStructMembers getMembers = { out, compiler };
			DKShaderResourceStruct rst = getMembers(out.typeInfoKey, compiler.get_type(resource.base_type_id));
			out.structTypeMemberMap.Update(out.typeInfoKey, std::move(rst));
		}
		return out;
	};

	// https://github.com/KhronosGroup/SPIRV-Cross/wiki/Reflection-API-user-guide
	// uniform_buffers
	for (const spirv_cross::Resource& resource : resources.uniform_buffers)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
		this->resources.Add(GetResource(resource, false));
	}
	// storage_buffers
	for (const spirv_cross::Resource& resource : resources.storage_buffers)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
		this->resources.Add(GetResource(resource, true));
	}
	// storage_images
	for (const spirv_cross::Resource& resource : resources.storage_images)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
		this->resources.Add(GetResource(resource, true));
	}
	// sampled_images (sampler2D)
	for (const spirv_cross::Resource& resource : resources.sampled_images)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
		this->resources.Add(GetResource(resource, false));
	}
	// separate_images
	for (const spirv_cross::Resource& resource : resources.separate_images)
	{
		const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);
		if (spType.image.dim == spv::DimBuffer)
		{
			GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
		}
		else
		{
			GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
		}
		this->resources.Add(GetResource(resource, false));
	}
	// separate_samplers
	for (const spirv_cross::Resource& resource : resources.separate_samplers)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_SAMPLER - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
		this->resources.Add(GetResource(resource, false));
	}
	// stage inputs
	this->stageInputAttributes.Reserve(resources.stage_inputs.size());
	for (const spirv_cross::Resource& resource : resources.stage_inputs)
	{
		uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
		DKStringU8 name = "";
		if (resource.name.size() > 0)
			name = resource.name.c_str();
		else
			name = compiler.get_fallback_name(resource.id).c_str();

		const spirv_cross::SPIRType& spType = compiler.get_type(resource.type_id);

		DKShaderDataType dataType = ShaderDataTypeFromSPIRType(spType);
		DKASSERT_DEBUG(dataType != DKShaderDataType::Unknown);

		// get item count! (array size)
		uint32_t count = 1;
		if (spType.array.size() > 0)
		{
			for (auto i : spType.array)
				count *= i;
		}

		DKShaderAttribute attr = {};
		attr.location = location;
		attr.name = name;
		attr.type = dataType;
		attr.active = true;

		this->stageInputAttributes.Add(attr);
	}

	// get pushConstant range.
	if (resources.push_constant_buffers.size() > 0)
	{
		const spirv_cross::Resource& resource = resources.push_constant_buffers[0];
		std::vector<spirv_cross::BufferRange> ranges = compiler.get_active_buffer_ranges(resource.id);
		this->pushConstantLayout.members.Reserve(ranges.size());

		uint32_t pushConstantOffset = (uint32_t)ranges[0].offset;
		uint32_t pushConstantSize = 0;

		for (spirv_cross::BufferRange &range : ranges)
		{
			// get range.
			PushConstantLayout::Member member;
			member.name = compiler.get_member_name(resource.id, range.index).c_str();
			member.offset = range.offset;
			member.size = range.range;
			this->pushConstantLayout.members.Add(std::move(member));

			pushConstantSize = (member.offset - pushConstantOffset) + member.size;
		}

		if (pushConstantSize % 4)	// size must be a multiple of 4
		{
			pushConstantSize += 4 - (pushConstantSize % 4);
		}

		DKASSERT_DEBUG((pushConstantOffset % 4) == 0);
		DKASSERT_DEBUG((pushConstantSize % 4) == 0);
		DKASSERT_DEBUG(pushConstantSize > 0);

		DKASSERT_DEBUG(pushConstantOffset < dev->properties.limits.maxPushConstantsSize);
		DKASSERT_DEBUG((pushConstantOffset + pushConstantSize) < dev->properties.limits.maxPushConstantsSize);

		this->pushConstantLayout.offset = pushConstantOffset;
		this->pushConstantLayout.size = pushConstantSize;

		if (resource.name.size() > 0)
			this->pushConstantLayout.name = resource.name.c_str();
		else
			this->pushConstantLayout.name = compiler.get_fallback_name(resource.id).c_str();

		const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);
	}

	// get module entry points
	std::vector<spirv_cross::EntryPoint> entryPoints = compiler.get_entry_points_and_stages();
	for (spirv_cross::EntryPoint& ep : entryPoints)
	{
		functionNames.Add(ep.name.c_str());
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
