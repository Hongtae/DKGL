//
//  File: ComputeCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "ComputeCommandEncoder.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

ComputeCommandEncoder::ComputeCommandEncoder(ID3D12GraphicsCommandList* cm, CommandBuffer* cb)
	: commandList(cm)
	, commandBuffer(cb)
{
	DKASSERT_DEBUG(commandList != nullptr);
}

ComputeCommandEncoder::~ComputeCommandEncoder(void)
{
}

void ComputeCommandEncoder::EndEncoding(void)
{
	if (commandList)
	{
		commandList->Close();

		commandBuffer->FinishCommandList(commandList.Get());
		commandList = nullptr;
	}
}

DKCommandBuffer* ComputeCommandEncoder::Buffer(void)
{
	return commandBuffer;
}

#endif //#if DKGL_USE_DIRECT3D
