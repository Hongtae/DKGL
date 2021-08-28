//
//  File: DKShader.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKShader.h"
#include "../../Libs/SPIRV-Cross/spirv_cross.hpp"
#include "../../Libs/SPIRV-Cross/spirv_common.hpp"

namespace DKFramework::Private
{
    inline DKShaderDataType ShaderDataTypeFromSPIRType(const spirv_cross::SPIRType& spType)
    {
        DKShaderDataType dataType = DKShaderDataType::Unknown;
        // get item type
        switch (spType.basetype)
        {
        case spirv_cross::SPIRType::Void:
            dataType = DKShaderDataType::None;
            break;
        case spirv_cross::SPIRType::Struct:
            dataType = DKShaderDataType::Struct;
            break;
        case spirv_cross::SPIRType::Image:
        case spirv_cross::SPIRType::SampledImage:
            dataType = DKShaderDataType::Texture;
            break;
        case spirv_cross::SPIRType::Sampler:
            dataType = DKShaderDataType::Sampler;
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
                DKLogE("ERROR: DKShader Unsupported stage input attribute type! (Int %d bit)", spType.width);
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
                DKLogE("ERROR: DKShader Unsupported stage input attribute type! (Int %d bit)", spType.width);
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
                DKLogE("ERROR: DKShader Unsupported stage input attribute type! (Float %d bit)", spType.width);
            }
            break;
        default:
            DKLogE("ERROR: DKShader Unsupported stage input attribute type!");
        }
        return dataType;
    }
}
using namespace DKFramework;

DKShader::DKShader()
    : stage(DKShaderStage::Unknown)
    , threadgroupSize({1, 1, 1})
{
}

DKShader::DKShader(const DKData* d)
    : DKShader()
{
    Rebuild(d);
}

DKShader::~DKShader()
{
}

bool DKShader::Rebuild(const DKData* d)
{
    if (d)
    {
        if (DKObject<DKData> data = d->ImmutableData(); data)
        {
            DKDataReader reader(data);

            try
            {
                spirv_cross::Compiler compiler(reinterpret_cast<const uint32_t*>(reader.Bytes()), reader.Length() / sizeof(uint32_t));

                switch (spv::ExecutionModel exec = compiler.get_execution_model(); exec)
                {
                case spv::ExecutionModelVertex:
                    this->stage = DKShaderStage::Vertex; break;
                case spv::ExecutionModelTessellationControl:
                    this->stage = DKShaderStage::TessellationControl; break;
                case spv::ExecutionModelTessellationEvaluation:
                    this->stage = DKShaderStage::TessellationEvaluation; break;
                case spv::ExecutionModelGeometry:
                    this->stage = DKShaderStage::Geometry; break;
                case spv::ExecutionModelFragment:
                    this->stage = DKShaderStage::Fragment; break;
                case spv::ExecutionModelGLCompute:
                    this->stage = DKShaderStage::Compute; break;
                default:
                    DKASSERT_DESC_DEBUG(0, "Unknown shader type");
                    break;
                }

                this->functions.Clear();
                this->resources.Clear();
                this->inputs.Clear();
                this->outputs.Clear();
                this->pushConstantLayouts.Clear();
                this->descriptors.Clear();
                this->threadgroupSize = { 1,1,1 };

                // get thread group size
                if (auto model = compiler.get_execution_model(); model == spv::ExecutionModelGLCompute)
                {
                    uint32_t localSizeX = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
                    uint32_t localSizeY = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
                    uint32_t localSizeZ = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);

                    spirv_cross::SpecializationConstant wg_x, wg_y, wg_z;
                    uint32_t constantId = compiler.get_work_group_size_specialization_constants(wg_x, wg_y, wg_z);
                    if (wg_x.id)
                        localSizeX = compiler.get_constant(wg_x.id).scalar();
                    if (wg_y.id)
                        localSizeY = compiler.get_constant(wg_y.id).scalar();
                    if (wg_z.id)
                        localSizeZ = compiler.get_constant(wg_z.id).scalar();

                    DKLog("ComputeShader.constantId: %u", constantId);
                    DKLog("ComputeShader.LocalSize.X: %u (specialized: %u, specializationID: %u)", localSizeX, wg_x.id, wg_x.constant_id);
                    DKLog("ComputeShader.LocalSize.Y: %u (specialized: %u, specializationID: %u)", localSizeY, wg_y.id, wg_y.constant_id);
                    DKLog("ComputeShader.LocalSize.Z: %u (specialized: %u, specializationID: %u)", localSizeZ, wg_z.id, wg_z.constant_id);

                    this->threadgroupSize.x = Max(localSizeX, 1U);
                    this->threadgroupSize.y = Max(localSizeY, 1U);
                    this->threadgroupSize.z = Max(localSizeZ, 1U);
                }

                auto active = compiler.get_active_interface_variables();
                spirv_cross::ShaderResources resources = compiler.get_shader_resources();

				uint32_t stage = static_cast<uint32_t>(this->stage);
                auto getResource = [&compiler, &active, stage](const spirv_cross::Resource& resource, bool writable)->DKShaderResource
                {
                    DKShaderResource out = {};
                    out.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                    out.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                    out.name = compiler.get_name(resource.id).c_str();
                    out.stride = compiler.get_decoration(resource.id, spv::DecorationArrayStride);
                    out.enabled = active.find(resource.id) != active.end();
					out.stages = stage;
                    if (writable)
                        out.access = DKShaderResource::AccessReadWrite;
                    else
                        out.access = DKShaderResource::AccessReadOnly;

                    const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
                    out.count = 1;
                    for (auto n : type.array)
                        out.count = out.count * n;

                    switch (type.basetype)
                    {
                    case spirv_cross::SPIRType::Image:
                        out.type = DKShaderResource::TypeTexture;
                        break;
                    case spirv_cross::SPIRType::SampledImage:
                        out.type = DKShaderResource::TypeTextureSampler;
                        break;
                    case spirv_cross::SPIRType::Sampler:
                        out.type = DKShaderResource::TypeSampler;
                        break;
                    case spirv_cross::SPIRType::Struct:
                        out.type = DKShaderResource::TypeBuffer;
                        out.typeInfo.buffer.dataType = DKShaderDataType::Struct;
                        //out.typeInfo.buffer.alignment = compiler.get_decoration(resource.id, spv::DecorationAlignment);
                        out.typeInfo.buffer.size = compiler.get_declared_struct_size(type);
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
                                    uint32_t type = spType.member_types[i];
                                    const spirv_cross::SPIRType& memberType = compiler.get_type(type);
                                    member.dataType = Private::ShaderDataTypeFromSPIRType(memberType);
                                    DKASSERT_DEBUG(member.dataType != DKShaderDataType::Unknown);
                                    DKASSERT_DEBUG(member.dataType != DKShaderDataType::None);

                                    member.name = compiler.get_member_name(spType.self, i).c_str();
                                    member.offset = compiler.type_struct_member_offset(spType, i);
                                    //member.size = compiler.get_declared_struct_member_size(spType, i);
                                    //DKASSERT_DEBUG(member.size > 0);

                                    if (member.dataType == DKShaderDataType::Struct)
                                    {
                                        member.typeInfoKey = DKString(name).Append(".").Append(member.name);
                                        GetStructMembers getMembers = { base, compiler };
                                        DKShaderResourceStruct memberStruct = getMembers(member.typeInfoKey, memberType);
                                        base.structTypeMemberMap.Update(member.typeInfoKey, memberStruct);
                                    }

                                    member.count = 1;
                                    for (auto n : memberType.array)
                                        member.count = member.count * n;

                                    if (member.count > 1)
                                        member.stride = compiler.type_struct_member_array_stride(spType, i);
                                    else
                                        member.stride = 0;
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

                auto getDescriptor = [&compiler](const spirv_cross::Resource& resource, DescriptorType type)->Descriptor
                {
                    Descriptor desc = {};
                    desc.type = type;
                    desc.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                    desc.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

                    const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);

                    // get item count! (array size)
                    desc.count = 1;
                    if (spType.array.size() > 0)
                    {
                        for (auto i : spType.array)
                            desc.count *= i;
                    }
                    return desc;
                };

                // https://github.com/KhronosGroup/SPIRV-Cross/wiki/Reflection-API-user-guide
                // uniform_buffers
                for (const spirv_cross::Resource& resource : resources.uniform_buffers)
                {
                    this->resources.Add(getResource(resource, false));
                    this->descriptors.Add(getDescriptor(resource, DescriptorTypeUniformBuffer));
                }
                // storage_buffers
                for (const spirv_cross::Resource& resource : resources.storage_buffers)
                {
                    this->resources.Add(getResource(resource, true));
                    this->descriptors.Add(getDescriptor(resource, DescriptorTypeStorageBuffer));
                }
                // storage_images
                for (const spirv_cross::Resource& resource : resources.storage_images)
                {
                    const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);
                    DescriptorType type = DescriptorTypeStorageTexture;
                    if (spType.image.dim == spv::DimBuffer)
                    {
                        type = DescriptorTypeStorageTexelBuffer;
                    }
                    this->resources.Add(getResource(resource, true));
                    this->descriptors.Add(getDescriptor(resource, type));
                }
                // sampled_images (sampler2D)
                for (const spirv_cross::Resource& resource : resources.sampled_images)
                {
                    this->resources.Add(getResource(resource, false));
                    this->descriptors.Add(getDescriptor(resource, DescriptorTypeTextureSampler));
                }
                // separate_images
                for (const spirv_cross::Resource& resource : resources.separate_images)
                {
                    const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);
                    DescriptorType type = DescriptorTypeTexture;
                    if (spType.image.dim == spv::DimBuffer)
                    {
                        type = DescriptorTypeUniformTexelBuffer;
                    }
                    this->resources.Add(getResource(resource, false));
                    this->descriptors.Add(getDescriptor(resource, type));
                }
                // separate_samplers
                for (const spirv_cross::Resource& resource : resources.separate_samplers)
                {
                    this->resources.Add(getResource(resource, false));
                    this->descriptors.Add(getDescriptor(resource, DescriptorTypeSampler));
                }

                auto getAttributes = [&compiler](const spirv_cross::Resource& resource)->DKShaderAttribute
                {
                    uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
                    DKStringU8 name = "";
                    if (resource.name.size() > 0)
                        name = resource.name.c_str();
                    else
                        name = compiler.get_fallback_name(resource.id).c_str();

                    const spirv_cross::SPIRType& spType = compiler.get_type(resource.type_id);

                    DKShaderDataType dataType = Private::ShaderDataTypeFromSPIRType(spType);
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
                    attr.enabled = true;
                    return attr;
                };
                // stage inputs
                this->inputs.Reserve(resources.stage_inputs.size());
                for (const spirv_cross::Resource& resource : resources.stage_inputs)
                {
                    this->inputs.Add(getAttributes(resource));
                }
                // stage outputs
                this->outputs.Reserve(resources.stage_outputs.size());
                for (const spirv_cross::Resource& resource : resources.stage_outputs)
                {
                    this->outputs.Add(getAttributes(resource));
                }

                // get pushConstant range.
                this->pushConstantLayouts.Reserve(resources.push_constant_buffers.size());
                for (const spirv_cross::Resource& resource : resources.push_constant_buffers)
                {
					spirv_cross::SmallVector<spirv_cross::BufferRange> ranges = compiler.get_active_buffer_ranges(resource.id);
                    DKShaderPushConstantLayout layout = {};
                    layout.memberLayouts.Reserve(ranges.size());
					layout.stages = stage;

                    uint32_t pushConstantOffset = (uint32_t)ranges[0].offset;
                    uint32_t pushConstantSize = 0;

                    for (spirv_cross::BufferRange &range : ranges)
                    {
                        // get range.
                        DKShaderPushConstantLayout memberLayout;
                        memberLayout.name = compiler.get_member_name(resource.id, range.index).c_str();
                        memberLayout.offset = range.offset;
                        memberLayout.size = range.range;
                        pushConstantSize = (memberLayout.offset - pushConstantOffset) + memberLayout.size;

                        layout.memberLayouts.Add(std::move(memberLayout));
                    }

                    if (pushConstantSize % 4)	// size must be a multiple of 4
                    {
                        pushConstantSize += 4 - (pushConstantSize % 4);
                    }

                    DKASSERT_DEBUG((pushConstantOffset % 4) == 0);
                    DKASSERT_DEBUG((pushConstantSize % 4) == 0);
                    DKASSERT_DEBUG(pushConstantSize > 0);

                    layout.offset = pushConstantOffset;
                    layout.size = pushConstantSize;
                    layout.name = compiler.get_name(resource.id).c_str();

                    //const spirv_cross::SPIRType& spType = compiler.get_type_from_variable(resource.id);

                    this->pushConstantLayouts.Add(std::move(layout));
                }

                // get module entry points
				spirv_cross::SmallVector<spirv_cross::EntryPoint> entryPoints = compiler.get_entry_points_and_stages();
                for (spirv_cross::EntryPoint& ep : entryPoints)
                {
                    functions.Add(ep.name.c_str());
                }

                // specialization constants
				spirv_cross::SmallVector<spirv_cross::SpecializationConstant> spConsts = compiler.get_specialization_constants();
                for (spirv_cross::SpecializationConstant& sc : spConsts)
                {
                    // 
                }

                // sort bindings
                this->descriptors.Sort([](const Descriptor& a, const Descriptor& b)
                {
                    if (a.set == b.set)
                        return a.binding < b.binding;
                    return a.set < b.set;
                });
                this->resources.Sort([](const DKShaderResource& a, const DKShaderResource& b)
                {
                    if (a.type == b.type)
                    {
                        if (a.set == b.set)
                            return a.binding < b.binding;
                        return a.set < b.set;
                    }
                    return static_cast<int>(a.type) < static_cast<int>(b.type);
                });
                this->inputs.Sort([](const DKShaderAttribute& a, const DKShaderAttribute& b)
                {
                    return a.location < b.location;
                });
                this->outputs.Sort([](const DKShaderAttribute& a, const DKShaderAttribute& b)
                {
                    return a.location < b.location;
                });
                this->descriptors.ShrinkToFit();
                this->resources.ShrinkToFit();
                this->inputs.ShrinkToFit();
                this->outputs.ShrinkToFit();

                this->data = data;
                return true;
            }
            catch (const spirv_cross::CompilerError& err)
            {
                DKLogE("CompileError: %s", err.what());
            }
            catch (...)
            {
                throw;
            }
        }
    }
    return false;
}
