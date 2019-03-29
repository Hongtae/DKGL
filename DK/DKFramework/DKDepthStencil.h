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


	class DKStencilDescriptor
	{
	};

	class DKDepthStencilDescriptor
	{
	};

	class DKDepthStencilState
	{
	};
}
