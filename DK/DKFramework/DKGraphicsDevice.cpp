//
//  File: DKGraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include "DKGraphicsDevice.h"
#include "Interface/DKGraphicsDeviceInterface.h"

using namespace DKFramework;

DKGraphicsDevice::DKGraphicsDevice()
{
	impl = DKGraphicsDeviceInterface::CreateInterface();
	if (impl == NULL)
		DKError::RaiseException("FATAL ERROR: Cannot create graphics device!");
}

DKGraphicsDevice::~DKGraphicsDevice()
{
	delete impl;
}

DKObject<DKCommandQueue> DKGraphicsDevice::CreateCommandQueue(uint32_t queueTypeFlags)
{
	return impl->CreateCommandQueue(this, queueTypeFlags);
}

DKObject<DKShaderModule> DKGraphicsDevice::CreateShaderModule(DKShader* shader)
{
	if (shader)
	{
		return impl->CreateShaderModule(this, shader);
	}
	return NULL;
}

DKObject<DKRenderPipelineState> DKGraphicsDevice::CreateRenderPipeline(const DKRenderPipelineDescriptor& desc, DKRenderPipelineReflection* reflection)
{
	return impl->CreateRenderPipeline(this, desc, reflection);
}

DKObject<DKComputePipelineState> DKGraphicsDevice::CreateComputePipeline(const DKComputePipelineDescriptor& desc, DKComputePipelineReflection* reflection)
{
	return impl->CreateComputePipeline(this, desc, reflection);
}

DKObject<DKGpuBuffer> DKGraphicsDevice::CreateBuffer(size_t size, DKGpuBuffer::StorageMode storage, DKCpuCacheMode cache)
{
	return impl->CreateBuffer(this, size, storage, cache);
}

DKObject<DKTexture> DKGraphicsDevice::CreateTexture(const DKTextureDescriptor& d)
{
	return impl->CreateTexture(this, d);
}
