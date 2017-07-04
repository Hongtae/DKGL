//
//  File: DKGraphicsDeviceInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKGraphicsDevice.h"

namespace DKFramework
{
	/// @brief Interface for graphics device (GPU) control.
	class DKGraphicsDeviceInterface
	{
	public:
		virtual ~DKGraphicsDeviceInterface(void) {}

		virtual DKString DeviceName(void) const = 0;
		virtual DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*) = 0;
		virtual DKObject<DKShaderModule> CreateShaderModule(DKGraphicsDevice*, DKShader*) = 0;
		virtual DKObject<DKRenderPipelineState> CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKPipelineReflection*) = 0;
		virtual DKObject<DKComputePipelineState> CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*) = 0;

		static DKGraphicsDeviceInterface* CreateInterface(void);
		static DKGraphicsDeviceInterface* Instance(DKGraphicsDevice* p) { return p->impl; }
	};
}
