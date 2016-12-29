//
//  File: DKGraphicsDeviceInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "DKBackendInterface.h"
#include "../DKGraphicsDevice.h"

namespace DKFramework
{
	/// @brief Interface for graphics device (GPU) control.
	class DKGraphicsDeviceInterface : public DKBackendInterface
	{
	public:

		virtual DKObject<DKCommandQueue> CreateCommandQueue(void) = 0;


		static DKGraphicsDeviceInterface* CreateInterface(void);
	};
}
