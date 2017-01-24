//
//  File: SwapChain.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "SwapChain.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

SwapChain::SwapChain(CommandQueue* q, DKWindow* w)
	: queue(q)
	, window(w)
{
	window->AddEventHandler(this, DKFunction(this, &SwapChain::OnWindowEvent), nullptr, nullptr);
}

SwapChain::~SwapChain(void)
{
	window->RemoveEventHandler(this);
}

bool SwapChain::Setup(void)
{
	GraphicsDevice* dc = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	IDXGIFactory5* factory = dc->factory.Get();

	HWND hWnd = (HWND)window->PlatformHandle();
	if (hWnd)
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = 2; // double-buffering
		swapChainDesc.Width = 0;
		swapChainDesc.Height = 0;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> sc;
		if (SUCCEEDED(factory->CreateSwapChainForHwnd(
			queue->queue.Get(),
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&sc)))
		{
			if (SUCCEEDED(sc.As(&this->swapChain)))
			{
				if (SUCCEEDED(this->swapChain->GetDesc1(&swapChainDesc)))
				{
					DKLog("IDXGISwapChain4 created with resolution: %u x %u",
						swapChainDesc.Width,
						swapChainDesc.Height);

					return true;
				}
				else
				{
					DKLog("ERROR: IDXGISwapChain1::GetDesc1 failed");
				}
			}
			else
			{
				DKLog("ERROR: IDXGISwapChain1::QueryInterface(IDXGISwapChain4) failed");
			}
		}
		else
		{
			DKLog("ERROR: ID3D12Device::CreateFence() failed");
		}
	}
	return false;
}

bool SwapChain::Present(void)
{
	return false;
}

void SwapChain::OnWindowEvent(const DKWindow::WindowEvent& e)
{
	if (e.type == DKWindow::WindowEvent::WindowResized)
	{
		//this->Update();
	}
}

#endif //#if DKGL_USE_DIRECT3D
