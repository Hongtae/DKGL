//
//  File: CopyCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12

#include "CopyCommandEncoder.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

CopyCommandEncoder::CopyCommandEncoder(ID3D12GraphicsCommandList* cm, class CommandBuffer* cb)
	: commandList(cm)
	, commandBuffer(cb)
{
	DKASSERT_DEBUG(commandList != nullptr);

}

CopyCommandEncoder::~CopyCommandEncoder()
{
}

void CopyCommandEncoder::EndEncoding()
{
	if (commandList)
	{
		commandList->Close();

		commandBuffer->FinishCommandList(commandList.Get());
		commandList = nullptr;
	}
}

DKCommandBuffer* CopyCommandEncoder::CommandBuffer()
{
	return commandBuffer;
}

bool CopyCommandEncoder::IsCompleted() const
{
    return commandList == nullptr; 
}

#endif //#if DKGL_ENABLE_DIRECT3D12
