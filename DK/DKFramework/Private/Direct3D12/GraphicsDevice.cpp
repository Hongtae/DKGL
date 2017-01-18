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
#include "../../DKPropertySet.h"

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
		D3D_FEATURE_LEVEL_11_0
	};

	if (true)
	{
		DKVariant deviceList = DKVariant::TypeArray;

		ComPtr<IDXGIAdapter1> adapter = nullptr;
		for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			//if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			//	continue;

			DKLog("D3D12 Adapter[%d]: \"%ls\" (VideoMemory:%.1fMB, SystemMemory:%.1fMB, SharedMemory:%.1fMB)",
				index, desc.Description,
				double(desc.DedicatedVideoMemory) / (1024.0 * 1024.0),
				double(desc.DedicatedSystemMemory) / (1024.0 * 1024.0),
				double(desc.SharedSystemMemory) / (1024.0 * 1024.0));
			
			deviceList.Array().Add(desc.Description);
		}
		DKPropertySet::SystemConfig().SetValue("GraphicsDeviceList", deviceList);
	}

	// select preferred device first.
	const char* preferredDeviceNameKey = "PreferredGraphicsDeviceName";
	if (DKPropertySet::SystemConfig().HasValue(preferredDeviceNameKey))
	{
		if (DKPropertySet::SystemConfig().Value(preferredDeviceNameKey).ValueType() == DKVariant::TypeString)
		{
			DKString prefDevName = DKPropertySet::SystemConfig().Value(preferredDeviceNameKey).String();

			if (prefDevName.Length() > 0)
			{
				ComPtr<IDXGIAdapter1> adapter = nullptr;
				for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
				{
					DXGI_ADAPTER_DESC1 desc;
					adapter->GetDesc1(&desc);
					if (prefDevName.CompareNoCase(desc.Description) == 0)
					{
						for (D3D_FEATURE_LEVEL featureLevel : featureLevels)
						{
							if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&this->device))))
							{
								this->deviceName = desc.Description;
								DKLog("D3D12CreateDevice created with \"%ls\" (NodeCount:%u, FeatureLevel:%x)",
									desc.Description, this->device->GetNodeCount(), featureLevel);
								break;
							}
						}
						if (this->device == nullptr)
						{
							DKLog("Cannot create device with preferred-device (%ls)", (const wchar_t*)prefDevName);
						}
						break;
					}
				}
			}
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

			ComPtr<IDXGIAdapter3> dxgiAdapter3;
			adapter->QueryInterface(IID_PPV_ARGS(&dxgiAdapter3));
			if (dxgiAdapter3)
			{
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&this->device))))
			{
				this->deviceName = desc.Description;
				DKLog("D3D12CreateDevice created with \"%ls\" (NodeCount:%u, FeatureLevel:%x)",
					desc.Description, this->device->GetNodeCount(), featureLevel);
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
			this->deviceName = desc.Description;
			DKLog("D3D12CreateDevice created WARP device with \"%ls\" (NodeCount:%u, FeatureLevel:%x)",
				desc.Description, this->device->GetNodeCount(), featureLevel);
		}
		else
			throw std::exception("D3D12CreateDevice failed");
	}
	DKASSERT_DEBUG(this->device != nullptr);

	if (FAILED(this->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->dummyAllocator))))
	{
		throw std::exception("ID3D12Device::CreateCommandAllocator failed");
	}
}

GraphicsDevice::~GraphicsDevice(void)
{
	PurgeAllReusableCommandLists();
	PurgeAllReusableCommandAllocators();
	this->dummyAllocator = nullptr;
	this->device = nullptr;
	DKLog("Direct3D12 Device destroyed.");
}

DKString GraphicsDevice::DeviceName(void) const
{
	return deviceName;
}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice* dev)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ComPtr<ID3D12CommandQueue> commandQueue;
	if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue))))
	{
		DKLog("ERROR: ID3D12Device::CreateCommandQueue() failed");
		return NULL;
	}
	ComPtr<ID3D12Fence> fence;
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
	{
		DKLog("ERROR: ID3D12Device::CreateFence() failed");
		return NULL;
	}

	DKObject<CommandQueue> queue = DKOBJECT_NEW CommandQueue(commandQueue.Get(), fence.Get(), dev);
	return queue.SafeCast<DKCommandQueue>();
}

void GraphicsDevice::PushReusableCommandAllocator(CommandAllocator* allocator)
{
	DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
	this->reusableCommandAllocators.Add(allocator);
}

CommandAllocator* GraphicsDevice::RetrieveReusableCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
	if (true)
	{
		DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
		for (size_t i = 0, numItems = this->reusableCommandAllocators.Count(); i < numItems; ++i)
		{
			CommandAllocator* cm = this->reusableCommandAllocators.Value(i);
			if (cm->type == type && cm->IsCompleted())
			{
				cm->Reset();
				this->reusableCommandAllocators.Remove(i);
				return cm;
			}
		}
	}
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	if (FAILED(this->device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator))))
	{
		DKLog("ERROR: ID3D12Device::CreateCommandAllocator() failed");
		return NULL;
	}

	return (CommandAllocator*)new(DKAllocator::DefaultAllocator().Alloc(sizeof(CommandAllocator))) CommandAllocator(commandAllocator.Get(), type);
}

void GraphicsDevice::PurgeAllReusableCommandAllocators(void)
{
	DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
	this->reusableCommandAllocators.EnumerateForward([](CommandAllocator* allocator)
	{
		void* ptr = static_cast<void*>(allocator);
		allocator->~CommandAllocator();
		DKAllocator::DefaultAllocator().Dealloc(ptr);
	});
	this->reusableCommandAllocators.Clear();
}

void GraphicsDevice::PushReusableCommandList(ID3D12CommandList* list)
{
	ComPtr<ID3D12GraphicsCommandList> graphicsCommandList;
	if (SUCCEEDED(list->QueryInterface(IID_PPV_ARGS(&graphicsCommandList))))
	{
		graphicsCommandList->Reset(this->dummyAllocator.Get(), nullptr);
		DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
		this->reusableCommandLists.Add( list );
		list->AddRef();
	}
	else
	{
		DKLog("ERROR: ID3D12CommandList::QueryInterface(ID3D12GraphicsCommandList) failed");
	}
}

ComPtr<ID3D12CommandList> GraphicsDevice::RetrieveReusableCommandList(D3D12_COMMAND_LIST_TYPE type)
{
	if (true)
	{
		DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
		for (size_t i = 0, numItems = this->reusableCommandLists.Count(); i < numItems; ++i)
		{
			ComPtr<ID3D12CommandList>& list = this->reusableCommandLists.Value(i);
			if (list->GetType() == type)
			{
				ComPtr<ID3D12CommandList> commandList = list;
				this->reusableCommandLists.Remove(i);
				return commandList;
			}
		}
	}
	ComPtr<ID3D12CommandList> commandList;
	if (FAILED(this->device->CreateCommandList(0, type, this->dummyAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList))))
	{
		DKLog("ERROR: ID3D12Device::CreateCommandList() failed");
		return NULL;
	}
	return commandList;
}

void GraphicsDevice::PurgeAllReusableCommandLists(void)
{
	DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
	this->reusableCommandLists.Clear();
}

#endif //#if DKGL_USE_DIRECT3D
