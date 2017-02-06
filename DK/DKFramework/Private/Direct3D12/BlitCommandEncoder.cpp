//
//  File: BlitCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "BlitCommandEncoder.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

BlitCommandEncoder::BlitCommandEncoder(ID3D12GraphicsCommandList* cm, CommandBuffer* cb)
	: commandList(cm)
	, commandBuffer(cb)
{
	DKASSERT_DEBUG(commandList != nullptr);

}

BlitCommandEncoder::~BlitCommandEncoder(void)
{
}

void BlitCommandEncoder::EndEncoding(void)
{
	if (commandList)
	{
		commandList->Close();

		commandBuffer->FinishCommandList(commandList.Get());
		commandList = nullptr;
	}
}

DKCommandBuffer* BlitCommandEncoder::Buffer(void)
{
	return commandBuffer;
}

#endif //#if DKGL_USE_DIRECT3D
