//
//  File: DKPipelineReflection.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKShaderResource.h"

namespace DKFramework
{
	/**
	@brief Pipeline's shader reflection
	*/
	struct DKPipelineReflection
	{
		DKArray<DKShaderAttribute> inputAttributes;
		DKArray<DKShaderPushConstantLayout> pushConstantLayouts;
		DKArray<DKShaderResource> resources;
	};
}
