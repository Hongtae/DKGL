//
//  File: DKComputePipeline.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKShaderFunction.h"

namespace DKFramework
{
	struct DKComputePipelineDescriptor
	{
        DKObject<DKShaderFunction> computeFunction;
        bool deferCompile = false;
        bool disableOptimization = false;
	};

    class DKGraphicsDevice;
	class DKComputePipelineState
	{
    public:
        virtual ~DKComputePipelineState() {}
        virtual DKGraphicsDevice* Device() = 0;
	};
}
