//
//  File: DKGraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include "DKGraphicsDevice.h"
#include "Interface/DKGraphicsDeviceInterface.h"

using namespace DKFramework;

DKGraphicsDevice::DKGraphicsDevice(void)
{
	impl = DKGraphicsDeviceInterface::CreateInterface();
	if (impl == NULL)
		DKError::RaiseException("FATAL ERROR: Cannot create graphics device!");
}

DKGraphicsDevice::~DKGraphicsDevice(void)
{
	DKRawPtrDelete(impl);
}

DKObject<DKCommandQueue> DKGraphicsDevice::CreateCommandQueue(void)
{
	return impl->CreateCommandQueue(this);
}
