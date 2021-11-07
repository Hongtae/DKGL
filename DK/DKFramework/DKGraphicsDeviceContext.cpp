//
//  File: DKGraphicsDeviceContext.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2019 Hongtae Kim. All rights reserved.
//

#include "DKGraphicsDeviceContext.h"

using namespace DKFramework;

DKGraphicsDeviceContext::DKGraphicsDeviceContext(DKGraphicsDevice* dev)
    : device(dev)
{
}

DKGraphicsDeviceContext::~DKGraphicsDeviceContext()
{
    graphicsQueues.Clear();
    computeQueues.Clear();
    transferQueues.Clear();

    cachedDeviceResources.Clear();

    device = nullptr;
}

DKGraphicsDevice* DKGraphicsDeviceContext::Device()
{
    return device;
}

DKCommandQueue* DKGraphicsDeviceContext::GraphicsQueue()
{
    if (graphicsQueues.IsEmpty())
    {
        DKObject<DKCommandQueue> queue = device->CreateCommandQueue(DKCommandQueue::Graphics);
        if (queue)
        {
            auto type = queue->Type();
            if (type & DKCommandQueue::Graphics)
                graphicsQueues.Add(queue);
            if (type & DKCommandQueue::Compute)
                computeQueues.Add(queue);
            transferQueues.Add(queue);

            if (type & DKCommandQueue::Graphics)
                return queue;
        }
    }
    if (graphicsQueues.Count() > 0)
        return graphicsQueues.Value(0);
    return nullptr;
}

DKCommandQueue* DKGraphicsDeviceContext::ComputeQueue()
{
    if (computeQueues.IsEmpty())
    {
        DKObject<DKCommandQueue> queue = device->CreateCommandQueue(DKCommandQueue::Compute);
        if (queue)
        {
            auto type = queue->Type();
            if (type & DKCommandQueue::Graphics)
                graphicsQueues.Add(queue);
            if (type & DKCommandQueue::Compute)
                computeQueues.Add(queue);
            transferQueues.Add(queue);

            if (type & DKCommandQueue::Compute)
                return queue;
        }
    }
    if (computeQueues.Count() > 0)
        return computeQueues.Value(0);
    return nullptr;
}

DKCommandQueue* DKGraphicsDeviceContext::TransferQueue()
{
    if (transferQueues.IsEmpty())
    {
        DKObject<DKCommandQueue> queue = device->CreateCommandQueue(0);
        if (queue)
        {
            auto type = queue->Type();
            if (type & DKCommandQueue::Graphics)
                graphicsQueues.Add(queue);
            if (type & DKCommandQueue::Compute)
                computeQueues.Add(queue);
            transferQueues.Add(queue);

            return queue;
        }
    }
    if (transferQueues.Count() > 0)
        return transferQueues.Value(0);
    return nullptr;
}
