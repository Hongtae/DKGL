//
//  File: DKPipelineReflection.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKShader.h"

namespace DKFramework
{
	struct DKShaderArgument
	{
		enum Type
		{
			TypeBuffer,
			TypeTexture,
			TypeSampler,
			TypeThreadgroupMemory,
		};

		DKString name;
		uint32_t index;
		Type type;
	};

	/**
	@brief Pipeline's shader reflection
	*/
	struct DKPipelineReflection
	{
		using ShaderArgumentArray = DKArray<DKShaderArgument>;
		using StageArgumentMap = DKMap<DKShader::StageType, ShaderArgumentArray>;

		StageArgumentMap argumentMap;
	};
}
