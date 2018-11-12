//
//  File: SwapChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKWindow.h"
#include "CommandQueue.h"
#include "Texture.h"

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
				~SwapChain();

				bool Setup();
				void SetupFrame();

                DKPixelFormat ColorPixelFormat() const override;
                DKPixelFormat DepthStencilPixelFormat() const override;
				void SetColorPixelFormat(DKPixelFormat) override;
				void SetDepthStencilPixelFormat(DKPixelFormat) override;
				DKRenderPassDescriptor CurrentRenderPassDescriptor() override;

				bool Present() override;


				DKObject<DKWindow> window;
				DKObject<CommandQueue> queue;

				ComPtr<IDXGISwapChain4> swapChain;

				ComPtr<ID3D12DescriptorHeap> rtvHeap;
				UINT rtvDescriptorSize;

				DKArray<DKObject<Texture>> renderTargets;

			private:
				bool enableVerticalSync = false;
				UINT bufferCount = 3;	// triple-buffering
				DKRenderPassDescriptor renderPassDescriptor;

				void OnWindowEvent(const DKWindow::WindowEvent&);
			};
		}
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
