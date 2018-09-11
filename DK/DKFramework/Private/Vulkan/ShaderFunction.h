//
//  File: ShaderFunction.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKShaderFunction.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Vulkan
{
	class ShaderFunction : public DKShaderFunction
	{
	public:
		ShaderFunction(DKShaderModule* module, const DKStringU8& name, const DKShaderSpecialization* values, size_t numValues);
		~ShaderFunction();

		const DKArray<DKVertexAttribute>& VertexAttributes() const override;
		const DKArray<DKShaderAttribute>& StageInputAttributes() const override;

		const DKMap<DKString, Constant>& FunctionConstants() const override { return functionConstantsMap; }
		DKString FunctionName() const override { return DKString(functionName); }

		DKGraphicsDevice* Device() override { return module->Device(); }

		DKObject<DKShaderModule> module;
		DKStringU8 functionName;

		DKArray<DKVertexAttribute> vertexAttributes;
		VkSpecializationInfo specializationInfo;
		void* specializationData;

		DKMap<DKString, Constant> functionConstantsMap;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
