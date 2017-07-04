//
//  File: DKGraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCommandQueue.h"
#include "DKShaderModule.h"
#include "DKShader.h"
#include "DKRenderPipeline.h"
#include "DKComputePipeline.h"
#include "DKPipelineReflection.h"

namespace DKFramework
{
	class DKGraphicsDeviceInterface;
	/// @brief Graphics device context
	class DKGL_API DKGraphicsDevice : public DKSharedInstance<DKGraphicsDevice>
	{
	public:
		DKGraphicsDevice(void);
		~DKGraphicsDevice(void);


		DKObject<DKCommandQueue> CreateCommandQueue(void);
		DKObject<DKShaderModule> CreateShaderModule(DKShader*);

		DKObject<DKRenderPipelineState> CreateRenderPipeline(const DKRenderPipelineDescriptor&, DKPipelineReflection* reflection = NULL);
		DKObject<DKComputePipelineState> CreateComputePipeline(const DKComputePipelineDescriptor&, DKPipelineReflection* reflection = NULL);

	private:
		DKGraphicsDeviceInterface* impl;
		friend class DKGraphicsDeviceInterface;
	};
}
