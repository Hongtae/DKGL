//
//  File: DKComputeCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCommandEncoder.h"
#include "DKComputePipeline.h"
#include "DKShaderBindingSet.h"

namespace DKFramework
{
	/// @brief Command encoder for GPU compute operation
	class DKComputeCommandEncoder : public DKCommandEncoder
	{
	public:
		virtual ~DKComputeCommandEncoder() {}

        virtual void SetResources(uint32_t set, const DKShaderBindingSet*) = 0;
        virtual void SetComputePipelineState(const DKComputePipelineState*) = 0;

        virtual void PushConstant(uint32_t stages, uint32_t offset, uint32_t size, const void*) = 0;

        virtual void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) = 0;
	};
}
