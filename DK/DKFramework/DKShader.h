//
//  File: DKShader.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"

namespace DKFramework
{
	/// @brief shader object
	class DKGL_API DKShader : public DKResource
	{
	public:
		enum class StageType
		{
			Vertex,
			TessellationControl,
			TessellationEvaluation,
			Geometry,
			Fragment,
			Compute,
		};

		DKShader(void);
		DKShader(DKData*, StageType stage, const DKString& entry = "main"); // share data
		DKShader(const void*, size_t, StageType stage, const DKString& entry = "main"); // copy data
		~DKShader(void);

		StageType stage;
		DKString entryPoint;
		DKObject<DKData> codeData;
	};
}
