//
//  File: DKShaderModule.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	class DKGraphicsDevice;
	/// @brief shader module (executable)
	class DKShaderModule
	{
	public:
		virtual ~DKShaderModule(void) {}

		virtual DKGraphicsDevice* Device(void) = 0;
	};
}
