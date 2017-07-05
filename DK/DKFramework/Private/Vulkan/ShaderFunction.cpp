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
	, pipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO })
{
	DKASSERT_DEBUG(module.SafeCast<ShaderModule>() != NULL);
	ShaderModule* shaderModule = module.StaticCast<ShaderModule>();
	pipelineShaderStageCreateInfo.stage = shaderModule->stage;
	pipelineShaderStageCreateInfo.module = shaderModule->module;
	pipelineShaderStageCreateInfo.pName = (const char*)functionName;
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
			specializationData = DKMalloc(size + sizeof(VkSpecializationInfo) + sizeof(VkSpecializationMapEntry) * numValues);
			VkSpecializationInfo* specializationInfo = reinterpret_cast<VkSpecializationInfo*>(specializationData);
			VkSpecializationMapEntry* mapEntries = reinterpret_cast<VkSpecializationMapEntry*>(&specializationInfo[1]);
			uint8_t* data = reinterpret_cast<uint8_t*>(&mapEntries[numValues]);

			specializationInfo->mapEntryCount = numValues;
			specializationInfo->pMapEntries = mapEntries;
			specializationInfo->pData = data;

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
			pipelineShaderStageCreateInfo.pSpecializationInfo = specializationInfo;
		}
	}
}

ShaderFunction::~ShaderFunction(void)
{
	if (specializationData)
		DKFree(specializationData);
}


#endif //#if DKGL_ENABLE_VULKAN
