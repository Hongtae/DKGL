//
//  File: SwapChain.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12

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

SwapChain::~SwapChain()
{
	window->RemoveEventHandler(this);

	swapChain->Present(0, DXGI_PRESENT_RESTART);

#if 0
	// wait until queue completed.
	UINT64 counter = this->queue->EnqueuedCounterValue();
	ID3D12Fence* fence = this->queue->fence.Get();

	if (fence->GetCompletedValue() < counter)
	{
		HANDLE fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		fence->SetEventOnCompletion(counter, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}
#endif
}

bool SwapChain::Setup()
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

	this->swapChain = nullptr;
	ComPtr<IDXGISwapChain1> sc;
	if (FAILED(factory->CreateSwapChainForHwnd(
		queue->queue.Get(),
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&sc)))
	{
		DKLogE("ERROR: IDXGIFactory5::CreateSwapChainForHwnd() failed");
		return false;
	}

	if (FAILED(sc.As(&this->swapChain)))
	{
		DKLogE("ERROR: IDXGISwapChain1::QueryInterface(IDXGISwapChain4) failed");
		return false;
	}
	if (FAILED(this->swapChain->GetDesc1(&swapChainDesc)))
	{
		DKLogE("ERROR: IDXGISwapChain1::GetDesc1 failed");
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
			DKLogE("ERROR: IDXGISwapChain4::GetBuffer failed");
			return false;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dc->GetDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(renderTarget.Get(), nullptr, rtvHandle);

		DKObject<Texture> texture = DKOBJECT_NEW Texture(queue->Device(), renderTarget.Get(), CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT), rtvHandle);
		this->renderTargets.Add(texture);
	}

	DKLog("%d RenderTargets for SwapChain created.", (int)this->renderTargets.Count());

	return true;
}
DKPixelFormat SwapChain::ColorPixelFormat() const
{
    return DKPixelFormat::Invalid;
}

DKPixelFormat SwapChain::DepthStencilPixelFormat() const
{
    return DKPixelFormat::Invalid;
}

void SwapChain::SetColorPixelFormat(DKPixelFormat)
{
}

void SwapChain::SetDepthStencilPixelFormat(DKPixelFormat)
{
}

DKRenderPassDescriptor SwapChain::CurrentRenderPassDescriptor()
{
	if (renderPassDescriptor.colorAttachments.Count() == 0)
		this->SetupFrame();

	return renderPassDescriptor;
}

void SwapChain::SetupFrame()
{
	UINT frameIndex = swapChain->GetCurrentBackBufferIndex();

	DKRenderPassColorAttachmentDescriptor colorAttachment = {};
	colorAttachment.renderTarget = renderTargets.Value(frameIndex);
	colorAttachment.clearColor = DKColor(0, 0, 0, 0);
	colorAttachment.loadAction = DKRenderPassAttachmentDescriptor::LoadActionClear;
	colorAttachment.storeAction = DKRenderPassAttachmentDescriptor::StoreActionStore;

	this->renderPassDescriptor.colorAttachments.Clear();
	this->renderPassDescriptor.colorAttachments.Add(colorAttachment);
}

bool SwapChain::Present()
{
	UINT syncInterval = enableVerticalSync ? 1 : 0;
	swapChain->Present(syncInterval, 0);

	renderPassDescriptor.colorAttachments.Clear();
	return true;
}

void SwapChain::OnWindowEvent(const DKWindow::WindowEvent& e)
{
	if (e.type == DKWindow::WindowEvent::WindowResized)
	{
		//this->Update();
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
