//
//  File: GraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include "GraphicsDevice.h"
#include "CommandQueue.h"

#pragma comment(lib, "D3d12.lib")
#pragma comment(lib, "DXGI.lib")

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			DKGraphicsDeviceInterface* CreateInterface(void)
			{
				return new GraphicsDevice();
			}
		}
	}
}


using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

GraphicsDevice::GraphicsDevice(void)
{
	UINT dxgiFactoryFlags = 0;

#ifdef DKGL_DEBUG_ENABLED
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	ComPtr<IDXGIFactory5> factory;
	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))))
		throw std::exception("CreateDXGIFactory2 failed");

	// feature levels (order by priority)
	std::initializer_list<D3D_FEATURE_LEVEL> featureLevels = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
	};

	if (true)
	{
		ComPtr<IDXGIAdapter1> adapter = nullptr;
		for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			DKLog("D3D12 Adapter[%d]: \"%ls\" (VideoMemory:%.1fMB, SystemMemory:%.1fMB, SharedMemory:%.1fMB)",
				index, desc.Description,
				double(desc.DedicatedVideoMemory) / (1024.0 * 1024.0),
				double(desc.DedicatedSystemMemory) / (1024.0 * 1024.0),
				double(desc.SharedSystemMemory) / (1024.0 * 1024.0));

		}
	}

	for (D3D_FEATURE_LEVEL featureLevel : featureLevels)
	{
		if (this->device)
			break;

		ComPtr<IDXGIAdapter1> adapter = nullptr;
		for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			IDXGIAdapter3 * dxgiAdapter3;
			adapter->QueryInterface(IID_PPV_ARGS(&dxgiAdapter3));
			if (dxgiAdapter3)
			{
				dxgiAdapter3->Release();
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&this->device))))
			{
				DKLog("D3D12CreateDevice created with \"%ls\" (featureLevel:%x)",
					desc.Description, featureLevel);
				break;
			}
		}
	}
	if (this->device == nullptr)
	{
		// use software (WARP) device.
		ComPtr<IDXGIAdapter> adapter = nullptr;
		if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter) )))
			throw std::exception("IDXGIFactory5::EnumWarpAdapter failed");

		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		D3D_FEATURE_LEVEL featureLevel = *featureLevels.begin();
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&this->device))))
		{
			DKLog("D3D12CreateDevice created WARP device with \"%ls\" (featureLevel:%x)",
				desc.Description, featureLevel);
		}
		else
			throw std::exception("D3D12CreateDevice failed");
	}
}

GraphicsDevice::~GraphicsDevice(void)
{

}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice* ctxt)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ComPtr<ID3D12CommandQueue> commandQueue;
	if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue))))
	{
		DKLog("ERROR: ID3D12Device1::CreateCommandQueue() failed");
		return NULL;
	}

	DKObject<CommandQueue> queue = DKOBJECT_NEW CommandQueue();
	queue->queue = commandQueue;
	queue->device = ctxt;
	return queue.SafeCast<DKCommandQueue>();
}

#endif //#if DKGL_USE_DIRECT3D
