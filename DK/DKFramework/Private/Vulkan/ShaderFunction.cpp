//
//  File: ShaderFunction.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "../../../Libs/SPIRV-Cross/spirv_cross.hpp"
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

	if (values && numValues)
	{
		size_t size = 0;
		for (size_t i = 0; i < numValues; ++i)
		{
			const DKShaderSpecialization& sp = values[i];
			size += sp.size;
		}

		if (size > 0)
		{
			specializationData = DKMalloc(size + sizeof(VkSpecializationMapEntry) * numValues);
			VkSpecializationMapEntry* mapEntries = reinterpret_cast<VkSpecializationMapEntry*>(specializationData);
			uint8_t* data = reinterpret_cast<uint8_t*>(&mapEntries[numValues]);

			specializationInfo.mapEntryCount = numValues;
			specializationInfo.pMapEntries = mapEntries;
			specializationInfo.pData = data;

			size_t offset = 0;
			for (size_t i = 0; i < numValues; ++i)
			{
				const DKShaderSpecialization& sp = values[i];
				mapEntries[i].constantID = sp.index;
				mapEntries[i].offset = offset;
				mapEntries[i].size = sp.size;

				memcpy(&data[offset], sp.data, sp.size);
				offset += sp.size;
			}
		}
	}
}

ShaderFunction::~ShaderFunction()
{
	if (specializationData)
		DKFree(specializationData);
}

const DKArray<DKShaderAttribute>& ShaderFunction::StageInputAttributes() const
{
	return module.StaticCast<ShaderModule>()->inputAttributes;
}

DKShaderStage ShaderFunction::Stage() const
{
	switch (module.StaticCast<ShaderModule>()->stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		return DKShaderStage::Vertex;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return DKShaderStage::TessellationControl;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return DKShaderStage::TessellationEvaluation;
	case VK_SHADER_STAGE_GEOMETRY_BIT:
		return DKShaderStage::Geometry;
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return DKShaderStage::Fragment;
	case VK_SHADER_STAGE_COMPUTE_BIT:
		return DKShaderStage::Compute;
	}
	return DKShaderStage::Unknown;
}
#endif //#if DKGL_ENABLE_VULKAN
