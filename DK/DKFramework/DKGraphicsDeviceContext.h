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
        DKGraphicsDeviceContext(DKGraphicsDevice*);
        virtual ~DKGraphicsDeviceContext();

        DKGraphicsDevice* Device();

        // cached command queue.
        DKCommandQueue* GraphicsQueue();
        DKCommandQueue* ComputeQueue();
        DKCommandQueue* TransferQueue();

        DKMap<DKString, DKObject<DKUnknown>> cachedDeviceResources;

    private:
        DKObject<DKGraphicsDevice> device;

        DKArray<DKObject<DKCommandQueue>> graphicsQueues;
        DKArray<DKObject<DKCommandQueue>> computeQueues;
        DKArray<DKObject<DKCommandQueue>> transferQueues;
    };
}
