//
//  File: DKCommandBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKCommandBuffer.h"
#include "DKCommandQueue.h"
#include "DKGraphicsDevice.h"

using namespace DKFramework;

DKCommandBuffer::~DKCommandBuffer(void)
{
}

DKGraphicsDevice* DKCommandBuffer::Device(void)
{
	return Queue()->Device();
}
