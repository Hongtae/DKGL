//
//  File: GraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
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
				return DKRawPtrNew<GraphicsDevice>();
			}
		}
	}
}


using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

GraphicsDevice::GraphicsDevice(void)
	: fenceCompletionEvent(NULL)
	, fenceCompletionThreadRunning(true)
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
	
	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))))
		throw std::runtime_error("CreateDXGIFactory2 failed");

	auto featureLevelToCStr = [](D3D_FEATURE_LEVEL lv) -> const char*
	{
		switch (lv)
		{
		case D3D_FEATURE_LEVEL_9_1: return "9_1";
		case D3D_FEATURE_LEVEL_9_2: return "9_2";
		case D3D_FEATURE_LEVEL_9_3: return "9_3";
		case D3D_FEATURE_LEVEL_10_0: return "10_0";
		case D3D_FEATURE_LEVEL_10_1: return "10_1";
		case D3D_FEATURE_LEVEL_11_0: return "11_0";
		case D3D_FEATURE_LEVEL_11_1: return "11_1";
		case D3D_FEATURE_LEVEL_12_0: return "12_0";
		case D3D_FEATURE_LEVEL_12_1: return "12_1";
		}
		return "Unknown";
	};

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
		DKPropertySet::SystemConfig().SetValue(graphicsDeviceListKey, deviceList);
	}

	// select preferred device first.
	DKPropertySet::SystemConfig().LookUpValueForKeyPath(preferredDeviceNameKey,
														DKFunction([&](const DKVariant& var)->bool
	{
		if (var.ValueType() == DKVariant::TypeString)
		{
			DKString preferredDeviceName = var.String();
			if (preferredDeviceName.Length() > 0)
			{
				ComPtr<IDXGIAdapter1> adapter = nullptr;
				for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
				{
					DXGI_ADAPTER_DESC1 desc;
					adapter->GetDesc1(&desc);
					if (preferredDeviceName.CompareNoCase(desc.Description) == 0)
					{
						for (D3D_FEATURE_LEVEL featureLevel : featureLevels)
						{
							if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&this->device))))
							{
								this->deviceName = desc.Description;
								DKLogI("D3D12CreateDevice created with \"%ls\" (NodeCount:%u, FeatureLevel:%s(%x))",
									  desc.Description, this->device->GetNodeCount(), featureLevelToCStr(featureLevel), featureLevel);
								break;
							}
						}
						if (this->device == nullptr)
						{
							DKLogW("Cannot create device with preferred-device (%ls)", (const wchar_t*)preferredDeviceName);
						}
						break;
					}
				}
			}
			return true;
		}
		return false;
	}));
	
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

			//ComPtr<IDXGIAdapter3> dxgiAdapter3;
			//adapter->QueryInterface(IID_PPV_ARGS(&dxgiAdapter3));
			//if (dxgiAdapter3)
			//{
			//}

			HRESULT hr = D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&this->device));
			if (SUCCEEDED(hr))
			{
				this->deviceName = desc.Description;
				DKLogI("D3D12CreateDevice created with \"%ls\" (NodeCount:%u, FeatureLevel:%s(%x))",
					desc.Description, this->device->GetNodeCount(), featureLevelToCStr(featureLevel), featureLevel);
				break;
			}
		}
	}
	if (this->device == nullptr)
	{
		// use software (WARP) device.
		ComPtr<IDXGIAdapter> adapter = nullptr;
		if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter) )))
			throw std::runtime_error("IDXGIFactory5::EnumWarpAdapter failed");

		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		D3D_FEATURE_LEVEL featureLevel = *featureLevels.begin();
		HRESULT hr = D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&this->device));
		if (SUCCEEDED(hr))
		{
			this->deviceName = desc.Description;
			DKLogI("D3D12CreateDevice created WARP device with \"%ls\" (NodeCount:%u, FeatureLevel:%s(%x))",
				desc.Description, this->device->GetNodeCount(), featureLevelToCStr(featureLevel), featureLevel);
		}
		else
			throw std::runtime_error("D3D12CreateDevice failed");
	}
	DKASSERT_DEBUG(this->device != nullptr);

	if (FAILED(this->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->dummyAllocator))))
	{
		throw std::runtime_error("ID3D12Device::CreateCommandAllocator failed");
	}

	for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		descriptorHandleIncrementSizes[i] = this->device->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)i);
	}
	descriptorHeapInitialCapacities[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = 4096;
	descriptorHeapInitialCapacities[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = 1024;
	descriptorHeapInitialCapacities[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = 128;
	descriptorHeapInitialCapacities[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = 128;

	fenceCompletionEvent = CreateEventW(0, 0, 0, 0);
	DKASSERT_DEBUG(fenceCompletionEvent);

	fenceCompletionThread = DKThread::Create(DKFunction(this, &GraphicsDevice::FenceCompletionCallbackThreadProc)->Invocation());
}

GraphicsDevice::~GraphicsDevice(void)
{
	if (fenceCompletionThread && fenceCompletionThread->IsAlive())
	{
		// finish Fence-Completion-Helper thread
		fenceCompletionCond.Lock();
		fenceCompletionThreadRunning = false;
		::SetEvent(fenceCompletionEvent);
		fenceCompletionCond.Broadcast();
		fenceCompletionCond.Unlock();

		fenceCompletionThread->WaitTerminate();
	}
	CloseHandle(fenceCompletionEvent);

	PurgeCachedCommandLists();
	PurgeCachedCommandAllocators();
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
		DKLogE("ERROR: ID3D12Device::CreateCommandQueue() failed");
		return NULL;
	}
	ComPtr<ID3D12Fence> fence;
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
	{
		DKLogE("ERROR: ID3D12Device::CreateFence() failed");
		return NULL;
	}

	DKObject<CommandQueue> queue = DKOBJECT_NEW CommandQueue(commandQueue.Get(), fence.Get(), dev);
	return queue.SafeCast<DKCommandQueue>();
}

CommandAllocator* GraphicsDevice::GetCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
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
		DKLogE("ERROR: ID3D12Device::CreateCommandAllocator() failed");
		return NULL;
	}

	return DKRawPtrNew<CommandAllocator>(commandAllocator.Get(), type);
}

void GraphicsDevice::ReleaseCommandAllocator(CommandAllocator* allocator)
{
	DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
	this->reusableCommandAllocators.Add(allocator);
}

void GraphicsDevice::PurgeCachedCommandAllocators(void)
{
	DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
	this->reusableCommandAllocators.EnumerateForward([](CommandAllocator* allocator)
	{
		DKRawPtrDelete(allocator);
	});
	this->reusableCommandAllocators.Clear();
}

ComPtr<ID3D12GraphicsCommandList> GraphicsDevice::GetCommandList(D3D12_COMMAND_LIST_TYPE type)
{
	if (true)
	{
		DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
		for (size_t i = 0, numItems = this->reusableCommandLists.Count(); i < numItems; ++i)
		{
			ComPtr<ID3D12GraphicsCommandList>& list = this->reusableCommandLists.Value(i);
			if (list->GetType() == type)
			{
				ComPtr<ID3D12GraphicsCommandList> commandList = list;
				this->reusableCommandLists.Remove(i);
				return commandList;
			}
		}
	}
	ComPtr<ID3D12GraphicsCommandList> commandList;
	if (FAILED(this->device->CreateCommandList(0, type, this->dummyAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList))))
	{
		DKLogE("ERROR: ID3D12Device::CreateCommandList() failed");
		return NULL;
	}
	commandList->Close();
	return commandList;
}

void GraphicsDevice::ReleaseCommandList(ID3D12GraphicsCommandList* list)
{
	DKASSERT_DEBUG(list);
	list->Reset(this->dummyAllocator.Get(), nullptr);
	list->Close();
	DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
	this->reusableCommandLists.Add(list);
	list->AddRef();
}

void GraphicsDevice::PurgeCachedCommandLists(void)
{
	DKCriticalSection<DKSpinLock> guard(reusableItemsLock);
	this->reusableCommandLists.Clear();
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDevice::GetDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	DKASSERT_DEBUG( type >= 0 && type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);

	DKArray<DescriptorHeap>& heapList = descriptorHeaps[type];
	UINT incrementSizes = descriptorHandleIncrementSizes[type];

	DKCriticalSection<DKSpinLock> guard(descriptorHeapLock);
	for (DescriptorHeap& dh : heapList)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = dh.heap->GetCPUDescriptorHandleForHeapStart();
		for (INT i = 0, c = (INT)dh.inUseBits.Count(); i < c; ++i)
		{
			if (dh.inUseBits.Value(i) == false)
			{
				dh.inUseBits.SetValue(i, true);
				return handle;
			}
			handle.ptr += incrementSizes;
		}
	}
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = type;
	desc.NumDescriptors = descriptorHeapInitialCapacities[type];
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	ComPtr<ID3D12DescriptorHeap> descHeap;
	if (SUCCEEDED(this->device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap))))
	{
		auto index = heapList.Add(DescriptorHeap());
		DescriptorHeap& dh = heapList.Value(index);
		dh.heap = descHeap;
		dh.inUseBits.Resize(desc.NumDescriptors, false);
		dh.inUseBits.SetValue(0, true);
		D3D12_CPU_DESCRIPTOR_HANDLE handle = dh.heap->GetCPUDescriptorHandleForHeapStart();
		return handle;
	}
	else
	{
		DKLogE("ERROR: ID3D12Device::CreateDescriptorHeap() failed");
	}

	return CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
}

void GraphicsDevice::ReleaseDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	DKASSERT_DEBUG( type >= 0 && type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);

	DKArray<DescriptorHeap>& heapList = descriptorHeaps[type];
	UINT incrementSizes = descriptorHandleIncrementSizes[type];

	DKCriticalSection<DKSpinLock> guard(descriptorHeapLock);
	for (DescriptorHeap& dh : heapList)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = dh.heap->GetDesc();
		DKASSERT_DEBUG(desc.Type == type);

		SIZE_T start = dh.heap->GetCPUDescriptorHandleForHeapStart().ptr;
		SIZE_T end = start + incrementSizes * desc.NumDescriptors;

		if (handle.ptr >= start && handle.ptr < end)
		{
			SIZE_T offset = handle.ptr - start;
			SIZE_T index = offset / incrementSizes;

			dh.inUseBits.SetValue(index, false);
			return;
		}
	}

	DKASSERT_DEBUG(false);	// error!
}

void GraphicsDevice::AddFenceCompletionHandler(ID3D12Fence* fence, UINT64 value, DKObject<DKOperation> op, bool useEventLoop)
{
	DKASSERT_DEBUG(fence);
	if (fence && op)
	{
		DKCriticalSection<DKCondition> guard(fenceCompletionCond);

		PendingFenceCallback cb = { fence, value, op, 0 };
		if (useEventLoop)
			cb.threadId = DKThread::CurrentThreadId();

		pendingFenceCallbacks.Add(cb);
		SetEvent(fenceCompletionEvent);
		fenceCompletionCond.Broadcast();
	}
}

void GraphicsDevice::FenceCompletionCallbackThreadProc(void)
{
	DKArray<ID3D12Fence*> waitingFences;
	DKArray<UINT64> waitingFenceValues;

	struct ThreadOperation
	{
		DKObject<DKOperation> operation;
		DKThread::ThreadId threadId;
	};
	struct FenceCallback
	{
		UINT64 fenceValueToFire;
		DKObject<DKOperation> operation;
		DKThread::ThreadId threadId;
	};
	struct PendingFenceCallbackArray
	{
		ComPtr<ID3D12Fence> fence;
		DKArray<FenceCallback> callbacks;
	};

	DKArray<ThreadOperation> callbacksToFire;
	DKMap<ID3D12Fence*, PendingFenceCallbackArray> pendingFenceCallbackArrayMap;

	DKLogI("Direct3D12 Fence Completion Helper thread is started.");

	DKCriticalSection<DKCondition> guard(fenceCompletionCond);
	while (fenceCompletionThreadRunning)
	{
		for (PendingFenceCallback& cb : pendingFenceCallbacks)
		{
			DKASSERT_DEBUG(cb.callback);

			PendingFenceCallbackArray& cb2 = pendingFenceCallbackArrayMap.Value(cb.fence.Get());
			cb2.fence = cb.fence.Get();
			cb2.callbacks.Add({ cb.fenceValue, cb.callback, cb.threadId });
		}
		pendingFenceCallbacks.Clear();

		if (pendingFenceCallbackArrayMap.IsEmpty())
		{
			fenceCompletionCond.Wait();
		}
		else
		{
			fenceCompletionCond.Unlock();

			waitingFences.Clear();		// to wait fences
			waitingFenceValues.Clear();	// fence completed values to signal

			waitingFences.Reserve(pendingFenceCallbackArrayMap.Count());
			waitingFenceValues.Reserve(pendingFenceCallbackArrayMap.Count());
			size_t totalCallbacks = 0;

			pendingFenceCallbackArrayMap.EnumerateForward([&](decltype(pendingFenceCallbackArrayMap)::Pair& pair)
			{
				pair.value.callbacks.Sort([](const FenceCallback& lhs, const FenceCallback& rhs)
				{
					return lhs.fenceValueToFire < rhs.fenceValueToFire;
				});
				waitingFences.Add(pair.key);
				waitingFenceValues.Add(pair.value.callbacks.Value(0).fenceValueToFire);
				totalCallbacks += pair.value.callbacks.Count();
			});
			DKASSERT_DEBUG(waitingFences.Count() > 0);
			DKASSERT_DEBUG(waitingFences.Count() == waitingFenceValues.Count());

			// wait event!
			this->device->SetEventOnMultipleFenceCompletion(
				(ID3D12Fence**)waitingFences,
				(const UINT64*)waitingFenceValues,
				static_cast<UINT>(waitingFences.Count()),
				D3D12_MULTIPLE_FENCE_WAIT_FLAG_ANY,
				this->fenceCompletionEvent);

			DWORD waitResult = WaitForSingleObject(this->fenceCompletionEvent, INFINITE);

			callbacksToFire.Clear();
			if (waitResult == WAIT_OBJECT_0)
			{
				waitingFences.Clear();		// store key(fence), that is no longer needed
				callbacksToFire.Reserve(totalCallbacks);

				pendingFenceCallbackArrayMap.EnumerateForward([&](decltype(pendingFenceCallbackArrayMap)::Pair& pair)
				{
					UINT64 completedValue = pair.key->GetCompletedValue();
					size_t index = 0;
					while (index < pair.value.callbacks.Count())
					{
						FenceCallback& cb = pair.value.callbacks.Value(index);
						if (cb.fenceValueToFire > completedValue)
							break;

						callbacksToFire.Add({ cb.operation, cb.threadId });
						index++;
					}
					pair.value.callbacks.Remove(0, index);
					if (pair.value.callbacks.IsEmpty())
						waitingFences.Add(pair.key);
				});
				for (ID3D12Fence* fence : waitingFences)
				{
					pendingFenceCallbackArrayMap.Remove(fence);
				}
			}

			for (ThreadOperation& tc : callbacksToFire)
			{
				DKObject<DKEventLoop> eventLoop = NULL;
				if (tc.threadId != 0)
					eventLoop = DKEventLoop::EventLoopForThreadId(tc.threadId);
				if (eventLoop)
					eventLoop->Post(tc.operation);
				else
					tc.operation->Perform();
			}
			callbacksToFire.Clear();

			fenceCompletionCond.Lock();
		}
	}
	DKLogI("Direct3D12 Fence Completion Helper thread is finished.");
}

#endif //#if DKGL_ENABLE_DIRECT3D12
