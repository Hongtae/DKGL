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

#include "../../ShaderFunction.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class ShaderFunction : public DKShaderModule
			{
			public:
				ShaderFunction(DKGraphicsDevice*, VkShaderModule, const void* data, size_t, DKShader::StageType, const DKStringU8&);
				~ShaderFunction(void);

				const DKArray<DKVertexAttribute> VertexAttributes(void) const override { return vertexAttributes;}
				const DKArray<DKShaderAttribute> StageInputAttributes(void) const override { return stageInputAttributes;}

				DKGraphicsDevice* Device(void) override { return device; }

				DKObject<DKGraphicsDevice> device;
				VkShaderModule module;
				VkShaderStageFlagBits stage;
				DKStringU8 entryPoint;

				DKArray<DKVertexAttribute> vertexAttributes;
				DKArray<DKShaderAttribute> stageInputAttributes;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_VULKAN
