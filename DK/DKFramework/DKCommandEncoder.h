//
//  File: DKCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	class DKGraphicsDevice;
	/// @brief Command encoder for GPU compute operation
	class DKCommandEncoder
	{
	public:
		virtual ~DKCommandEncoder(void) {}

		virtual void EndEncoding(void) = 0;
		virtual DKGraphicsDevice* Device(void) = 0;
	};
}
