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

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class ShaderFunction : public DKShaderFunction
			{
			public:
				ShaderFunction(DKGraphicsDevice*, VkShaderModule, const void* data, size_t, DKShader::StageType, const DKStringU8&, ShaderFunction* parent = NULL);
				~ShaderFunction(void);

				const DKArray<DKVertexAttribute> VertexAttributes(void) const override { return vertexAttributes;}
				const DKArray<DKShaderAttribute> StageInputAttributes(void) const override { return stageInputAttributes;}

				const DKMap<DKString, Constant>& FunctionConstants(void) const override { return functionConstantsMap;}
				DKObject<DKShaderFunction> CreateSpecializedFunction(const DKShaderSpecialization* values, size_t numValues) const override;

				DKGraphicsDevice* Device(void) override { return device; }

				DKObject<DKGraphicsDevice> device;
				DKObject<ShaderFunction> parent;
				VkShaderModule module;
				VkShaderStageFlagBits stage;
				DKStringU8 entryPoint;

				DKArray<DKVertexAttribute> vertexAttributes;
				DKArray<DKShaderAttribute> stageInputAttributes;
				DKMap<DKString, Constant> functionConstantsMap;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_VULKAN
