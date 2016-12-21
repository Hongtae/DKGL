//
//  File: DKGpuResource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	/// @brief GPU buffer object
	class DKGpuBuffer
	{
	public:
		virtual ~DKGpuBuffer(void) {}
	};
	/// @brief GPU texture object
	class DKGpuTexture
	{
	public:
		virtual ~DKGpuTexture(void) {}
	};
}
