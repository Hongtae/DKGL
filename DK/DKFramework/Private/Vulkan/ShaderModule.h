//
//  File: ShaderModule.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKShaderModule.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class ShaderModule : public DKShaderModule
			{
			public:
				ShaderModule(DKGraphicsDevice*, VkShaderModule, const void* data, size_t, DKShader::StageType, const DKStringU8&);
				~ShaderModule(void);


				DKGraphicsDevice* Device(void) override { return device; }

				DKObject<DKGraphicsDevice> device;
				VkShaderModule module;
				VkShaderStageFlagBits stage;
				DKStringU8 entryPoint;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_VULKAN
