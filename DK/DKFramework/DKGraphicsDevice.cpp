//
//  File: DKGraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include "DKGraphicsDevice.h"
#include "Interface/DKGraphicsDeviceInterface.h"

using namespace DKFramework;

DKGraphicsDevice::DKGraphicsDevice(void)
{
	impl = DKGraphicsDeviceInterface::CreateInterface();
	if (impl == NULL)
		DKError::RaiseException("FATAL ERROR: Cannot create graphics device!");
}

DKGraphicsDevice::~DKGraphicsDevice(void)
{
	DKRawPtrDelete(impl);
}

DKObject<DKCommandQueue> DKGraphicsDevice::CreateCommandQueue(void)
{
	return impl->CreateCommandQueue(this);
}

DKObject<DKShaderFunction> DKGraphicsDevice::CreateShaderFunction(DKShader* shader)
{
	if (shader)
	{
		return impl->CreateShaderFunction(this, shader);
	}
	return NULL;
}

DKObject<DKRenderPipelineState> DKGraphicsDevice::CreateRenderPipeline(const DKRenderPipelineDescriptor& desc, DKPipelineReflection* reflection)
{
	return impl->CreateRenderPipeline(this, desc, reflection);
}

DKObject<DKComputePipelineState> DKGraphicsDevice::CreateComputePipeline(const DKComputePipelineDescriptor& desc, DKPipelineReflection* reflection)
{
	return impl->CreateComputePipeline(this, desc, reflection);
}
