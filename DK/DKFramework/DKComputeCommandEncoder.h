//
//  File: DKComputeCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCommandEncoder.h"

namespace DKFramework
{
	/// @brief Command encoder for GPU compute operation
	class DKComputeCommandEncoder : public DKCommandEncoder
	{
	public:
		virtual ~DKComputeCommandEncoder() {}
	};
}
