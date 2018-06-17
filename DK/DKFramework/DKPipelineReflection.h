//
//  File: DKRenderPipelineReflection.h
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
	@brief Render Pipeline's shader reflection
	*/
	struct DKRenderPipelineReflection
	{
		DKArray<DKShaderResource> vertexResources;
		DKArray<DKShaderResource> fragmentResources;
	};
	/**
	@brief Compute Pipeline's shader reflection
	*/struct DKComputePipelineReflection
	{
		DKArray<DKShaderResource> resources;
	};
}
