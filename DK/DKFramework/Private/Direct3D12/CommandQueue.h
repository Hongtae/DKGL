//
//  File: CommandQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include <wrl.h>
#include <D3D12.h>
#include <dxgi1_5.h>
using Microsoft::WRL::ComPtr;

#include "../../DKCommandQueue.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class CommandQueue : public DKCommandQueue
			{
			public:
				~CommandQueue(void);

				DKObject<DKCommandBuffer> CreateCommandBuffer(void) override;

				DKObject<DKGraphicsDevice> device;
				ComPtr<ID3D12CommandQueue> queue;
			};
		}
	}
}
#endif //#if DKGL_USE_DIRECT3D
