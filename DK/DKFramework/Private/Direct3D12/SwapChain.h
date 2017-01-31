//
//  File: SwapChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include "d3d12_headers.h"

#include "../../DKWindow.h"
#include "CommandQueue.h"
#include "RenderTarget.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class SwapChain : public DKSwapChain
			{
			public:
				SwapChain(CommandQueue*, DKWindow*);
				~SwapChain(void);

				bool Setup(void);

				bool Present(void) override;

				DKObject<DKWindow> window;
				DKObject<CommandQueue> queue;

				ComPtr<IDXGISwapChain4> swapChain;

				ComPtr<ID3D12DescriptorHeap> rtvHeap;
				UINT rtvDescriptorSize;

				DKArray<DKObject<RenderTarget>> renderTargets;

			private:
				UINT bufferCount = 2;	// double-buffering

				void OnWindowEvent(const DKWindow::WindowEvent&);
			};
		}
	}
}

#endif //#if DKGL_USE_DIRECT3D
