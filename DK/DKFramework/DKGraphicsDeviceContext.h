//
//  File: DKGraphicsDeviceContext.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKGraphicsDevice.h"
#include "DKCommandQueue.h"

namespace DKFramework
{
    class DKGL_API DKGraphicsDeviceContext
    {
    public:
        DKGraphicsDeviceContext();
        virtual ~DKGraphicsDeviceContext();

        DKObject<DKGraphicsDevice> device;
        DKObject<DKCommandQueue> transferQueue;
    };
}
