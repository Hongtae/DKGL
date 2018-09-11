//
//  File: DKShaderFunction.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKShaderResource.h"

namespace DKFramework
{
	class DKGraphicsDevice;

	struct DKShaderAttribute
	{
		DKString name;
		uint32_t location;
		DKShaderDataType type;
		bool active;
		bool patchControlPointData;
		bool patchData;
	};

	using DKVertexAttribute = DKShaderAttribute;

	class DKShaderFunction
	{
	public:
		struct Constant
		{
			DKString name;
			DKShaderDataType type;
			uint32_t index;
			bool required;
		};

		virtual ~DKShaderFunction() {}

		virtual const DKArray<DKVertexAttribute>& VertexAttributes() const = 0;
		virtual const DKArray<DKShaderAttribute>& StageInputAttributes() const = 0;
		virtual const DKMap<DKString, Constant>& FunctionConstants() const = 0;
		virtual DKString FunctionName() const = 0;

		virtual DKGraphicsDevice* Device() = 0;
	};
}
