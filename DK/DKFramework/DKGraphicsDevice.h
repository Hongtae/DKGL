//
//  File: DKGraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"


////////////////////////////////////////////////////////////////////////////////
// DKGraphicsDevice
// Graphics Device
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKGraphicsDeviceInterface;
	class DKGL_API DKGraphicsDevice : public DKFoundation::DKSharedInstance<DKGraphicsDevice>
	{
		DKGraphicsDevice(void);
		~DKGraphicsDevice(void);

	private:
		DKGraphicsDeviceInterface* impl;
	};
}
