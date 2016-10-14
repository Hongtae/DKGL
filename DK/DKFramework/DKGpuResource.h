//
//  File: DKGpuResource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

namespace DKGL
{
	class DKGpuBuffer
	{
	public:
		virtual ~DKGpuBuffer(void) {}
	};

	class DKGpuTexture
	{
	public:
		virtual ~DKGpuTexture(void) {}
	};
}
