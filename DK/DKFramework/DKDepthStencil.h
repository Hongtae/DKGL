//
//  File: DKDepthStencil.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
    enum DKCompareFunction
    {
        DKCompareFunctionNever,
        DKCompareFunctionLess,
        DKCompareFunctionEqual,
        DKCompareFunctionLessEqual,
        DKCompareFunctionGreater,
        DKCompareFunctionNotEqual,
        DKCompareFunctionGreaterEqual,
        DKCompareFunctionAlways,
    };

    enum DKStencilOperation
    {
        DKStencilOperationKeep,
        DKStencilOperationZero,
        DKStencilOperationReplace,
        DKStencilOperationIncrementClamp,
        DKStencilOperationDecrementClamp,
        DKStencilOperationInvert,
        DKStencilOperationIncrementWrap,
        DKStencilOperationDecrementWrap,
    };

	struct DKStencilDescriptor
	{
        DKCompareFunction stencilCompareFunction = DKCompareFunctionAlways;
        DKStencilOperation stencilFailureOperation = DKStencilOperationKeep;
        DKStencilOperation depthFailOperation = DKStencilOperationKeep;
        DKStencilOperation depthStencilPassOperation = DKStencilOperationKeep;
        uint32_t readMask;
        uint32_t writeMask;
	};

    struct DKDepthStencilDescriptor
	{
        DKCompareFunction depthCompareFunction = DKCompareFunctionAlways;
        DKStencilDescriptor frontFaceStencil;
        DKStencilDescriptor backFaceStencil;
        bool depthWriteEnabled = false;
	};
}
