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

	// uniform_buffers
	for (const spirv_cross::Resource& resource : resources.uniform_buffers)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
	}
	// storage_buffers
	for (const spirv_cross::Resource& resource : resources.storage_buffers)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
	}
	// storage_images
	for (const spirv_cross::Resource& resource : resources.storage_images)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
	}
	// sampled_images
	for (const spirv_cross::Resource& resource : resources.sampled_images)
	{
		GetLayout(resource, this->layouts[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]);
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

		const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);

		DKShaderDataType dataType = DKShaderDataType::None;

		// get item type
		switch (spType.basetype)
		{
		case spirv_cross::SPIRType::Void:
			dataType = DKShaderDataType::None;
			break;
		case spirv_cross::SPIRType::Struct:
			dataType = DKShaderDataType::Struct;
			break;
		case spirv_cross::SPIRType::Boolean:
			switch (spType.vecsize)
			{
			case 2:		dataType = DKShaderDataType::Bool2;		break;
			case 3:		dataType = DKShaderDataType::Bool3;		break;
			case 4:		dataType = DKShaderDataType::Bool4;		break;
			default:	dataType = DKShaderDataType::Bool;		break;
			}
			break;
		case spirv_cross::SPIRType::Char:
			switch (spType.vecsize)
			{
			case 2:		dataType = DKShaderDataType::Char2;		break;
			case 3:		dataType = DKShaderDataType::Char3;		break;
			case 4:		dataType = DKShaderDataType::Char4;		break;
			default:	dataType = DKShaderDataType::Char;		break;
			}
			break;
		case spirv_cross::SPIRType::Int:
		case spirv_cross::SPIRType::Int64:
			if (spType.width == 16)
			{
				switch (spType.vecsize)
				{
				case 2:		dataType = DKShaderDataType::Short2;	break;
				case 3:		dataType = DKShaderDataType::Short3;	break;
				case 4:		dataType = DKShaderDataType::Short4;	break;
				default:	dataType = DKShaderDataType::Short;		break;
				}
			}
			else if (spType.width == 32)
			{
				switch (spType.vecsize)
				{
				case 2:		dataType = DKShaderDataType::Int2;		break;
				case 3:		dataType = DKShaderDataType::Int3;		break;
				case 4:		dataType = DKShaderDataType::Int4;		break;
				default:	dataType = DKShaderDataType::Int;		break;
				}
			}
			else
			{
				DKLogE("ERROR: DKShaderModule Unsupported stage input attribute type! (Int %d bit)", spType.width);
			}
			break;
		case spirv_cross::SPIRType::UInt:
		case spirv_cross::SPIRType::UInt64:
			if (spType.width == 16)
			{
				switch (spType.vecsize)
				{
				case 2:		dataType = DKShaderDataType::UShort2;	break;
				case 3:		dataType = DKShaderDataType::UShort3;	break;
				case 4:		dataType = DKShaderDataType::UShort4;	break;
				default:	dataType = DKShaderDataType::UShort;	break;
				}
			}
			else if (spType.width == 32)
			{
				switch (spType.vecsize)
				{
				case 2:		dataType = DKShaderDataType::UInt2;		break;
				case 3:		dataType = DKShaderDataType::UInt3;		break;
				case 4:		dataType = DKShaderDataType::UInt4;		break;
				default:	dataType = DKShaderDataType::UInt;		break;
				}
			}
			else
			{
				DKLogE("ERROR: DKShaderModule Unsupported stage input attribute type! (Int %d bit)", spType.width);
			}
			break;
		case spirv_cross::SPIRType::Half:
		case spirv_cross::SPIRType::Float:
		case spirv_cross::SPIRType::Double:
			if (spType.width == 16)
			{
				switch (spType.vecsize)
				{
				case 2:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Half2x2;	break;
					case 3:		dataType = DKShaderDataType::Half2x3;	break;
					case 4:		dataType = DKShaderDataType::Half2x4;	break;
					default:	dataType = DKShaderDataType::Half2;		break;
					}
					break;
				case 3:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Half3x2;	break;
					case 3:		dataType = DKShaderDataType::Half3x3;	break;
					case 4:		dataType = DKShaderDataType::Half3x4;	break;
					default:	dataType = DKShaderDataType::Half3;		break;
					}
					break;
				case 4:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Half4x2;	break;
					case 3:		dataType = DKShaderDataType::Half4x3;	break;
					case 4:		dataType = DKShaderDataType::Half4x4;	break;
					default:	dataType = DKShaderDataType::Half4;		break;
					}
					break;
				default:
					dataType = DKShaderDataType::Half;		break;
				}
			}
			else if (spType.width == 32)
			{
				switch (spType.vecsize)
				{
				case 2:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Float2x2;	break;
					case 3:		dataType = DKShaderDataType::Float2x3;	break;
					case 4:		dataType = DKShaderDataType::Float2x4;	break;
					default:	dataType = DKShaderDataType::Float2;	break;
					}
					break;
				case 3:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Float3x2;	break;
					case 3:		dataType = DKShaderDataType::Float3x3;	break;
					case 4:		dataType = DKShaderDataType::Float3x4;	break;
					default:	dataType = DKShaderDataType::Float3;	break;
					}
					break;
				case 4:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Float4x2;	break;
					case 3:		dataType = DKShaderDataType::Float4x3;	break;
					case 4:		dataType = DKShaderDataType::Float4x4;	break;
					default:	dataType = DKShaderDataType::Float4;	break;
					}
					break;
				default:
					dataType = DKShaderDataType::Float;		break;
				}
			}
			else
			{
				DKLogE("ERROR: DKShaderModule Unsupported stage input attribute type! (Float %d bit)", spType.width);
			}
			break;
		default:
			DKLogE("ERROR: DKShaderModule Unsupported stage input attribute type!");
		}

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
