//
//  File: DKShaderModule.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKShaderFunction.h"

namespace DKFramework
{
	struct DKShaderSpecialization
	{
		DKShaderDataType type;
		const void* data;
		uint32_t index;
		size_t size;
	};

	class DKShaderModule
	{
	public:
		virtual ~DKShaderModule(void) {}

		virtual DKObject<DKShaderFunction> CreateFunction(const DKString& name) const = 0;
		virtual DKObject<DKShaderFunction> CreateSpecializedFunction(const DKString& name, const DKShaderSpecialization* values, size_t numValues) const = 0;
		virtual const DKArray<DKString>& FunctionNames(void) const = 0;

		virtual DKGraphicsDevice* Device(void) = 0;
	};
}
