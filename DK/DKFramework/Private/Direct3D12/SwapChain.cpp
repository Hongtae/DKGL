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
	ID3D12Device1* device = dc->device.Get();

	HWND hWnd = (HWND)window->PlatformHandle();
	if (hWnd == NULL)
		return false;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> sc;
	if (FAILED(factory->CreateSwapChainForHwnd(
		queue->queue.Get(),
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&sc)))
	{
		DKLog("ERROR: IDXGIFactory5::CreateSwapChainForHwnd() failed");
		return false;
	}

	if (FAILED(sc.As(&this->swapChain)))
	{
		DKLog("ERROR: IDXGISwapChain1::QueryInterface(IDXGISwapChain4) failed");
		return false;
	}
	if (FAILED(this->swapChain->GetDesc1(&swapChainDesc)))
	{
		DKLog("ERROR: IDXGISwapChain1::GetDesc1 failed");
		return false;
	}

	DKLog("IDXGISwapChain4 created with resolution: %u x %u",
		  swapChainDesc.Width,
		  swapChainDesc.Height);

	// Create a RTV for each frame.
	for (UINT n = 0; n < swapChainDesc.BufferCount; n++)
	{
		ComPtr<ID3D12Resource> renderTarget;
		if (FAILED(this->swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTarget))))
		{
			DKLog("ERROR: IDXGISwapChain4::GetBuffer failed");
			return false;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dc->GetDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(renderTarget.Get(), nullptr, rtvHandle);

		DKObject<RenderTarget> texture = DKOBJECT_NEW RenderTarget(dc, renderTarget.Get(), CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT), rtvHandle);
		this->renderTargets.Add(texture);
	}

	DKLog("%d RenderTargets for SwapChain created.", (int)this->renderTargets.Count());

	return true;
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
