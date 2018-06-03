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
		enum StageType
		{
			Vertex,
			Fragment,
			Compute,
		};

		DKShader(void);
		DKShader(DKData*, StageType stage); // share data
		DKShader(const void*, size_t, StageType stage); // copy data
		~DKShader(void);

		StageType stage;
		DKObject<DKData> codeData;
	};
}
