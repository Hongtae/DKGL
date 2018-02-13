//
//  File: DKGpuBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//


#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	class DKGpuBuffer
	{
	public:
		virtual ~DKGpuBuffer(void);

	protected:
		DKGpuBuffer(void);
	};
}