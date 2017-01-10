//
//  File: DKBlitCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	/// @brief CommandEncoder for GPU blit operation
	class DKBlitCommandEncoder
	{
	public:
		enum Option
		{
			OptionNone                       = 0,
			OptionDepthFromDepthStencil      = 1 << 0,
			OptionStencilFromDepthStencil    = 1 << 1,
		};

		virtual ~DKBlitCommandEncoder(void) {}
	};
}
