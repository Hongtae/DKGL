//
//  File: DKAllocator.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKObjectRefCounter.h"
#include "DKSpinLock.h"
#include "DKMap.h"
#include "DKArray.h"
#include "DKMemory.h"
#include "DKFixedSizeAllocator.h"

namespace DKFoundation
{
	namespace Private
	{
		enum {AllocatorTableLength = 977}; // should be prime-number.
		struct AllocationNode
		{
			struct NodeInfo
			{
				DKAllocator*								allocator;
				DKObjectRefCounter::RefIdValue				refId;
				volatile DKObjectRefCounter::RefCountValue	refCount;
			};
			typedef DKSpinLock							Lock;
			typedef DKCriticalSection<Lock>				CriticalSection;

			// use fixed-size allocator.
			struct Allocator
			{
				enum { nodeSize = DKMap<void*, NodeInfo>::NodeSize() };
				using FixedAllocator = DKFixedSizeAllocator<nodeSize, 64, Lock>;

				static void* Alloc(size_t s)
				{
					DKASSERT_DEBUG(s == nodeSize);
					return FixedAllocator::Instance().Alloc(s);
				}
				static void Free(void* p)
				{
					FixedAllocator::Instance().Dealloc(p);
				}
			};

			using Key = void*;
			using Container = DKMap<Key, NodeInfo, DKDummyLock, DKMapKeyComparison<Key>, DKMapValueCopy<Key>, Allocator>;

			Lock		lock;
			Container	container;
		};

		////////////////////////////////////////////////////////////////////////
		// GetAllocationNode
		// generate map for object address, info pair and return address.
		//
		// Note:
		//  The map (refMaps) instance which contains actual address,
		//  should be static-variable inside of this function. If it declared
		//  as global variable, it will not be initialized at time, becouse of
		//  module's global variable initialize order.
		//  It would be safe to be initialized, if it initalized by calling
		//  function as static-variable. it will be initialized on first call.
		namespace
		{
			DKSpinLock spinLock;

			AllocationNode& GetAllocationNode(void* ptr)
			{
				static AllocationNode* nodeTables = NULL;
				if (nodeTables == NULL)
				{
					DKCriticalSection<DKSpinLock> guard(spinLock);
					if (nodeTables == NULL)
					{
						static AllocationNode nodes[AllocatorTableLength];
						nodeTables = nodes;
					}
				}
				return nodeTables[reinterpret_cast<uintptr_t>(ptr) % AllocatorTableLength];
			}
			DKObjectRefCounter::RefIdValue GenerateRefId(void)
			{
				DKCriticalSection<DKSpinLock> guard(spinLock);
				static DKObjectRefCounter::RefIdValue counter = 0;
				DKObjectRefCounter::RefIdValue value = ++counter;
				return value;
			}
		}
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

bool DKObjectRefCounter::SetRefCounter(void* p, DKAllocator* alloc, RefCountValue c, RefIdValue* refId)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair == NULL)
		{
			AllocationNode::NodeInfo nodeInfo = {alloc, GenerateRefId(), c};
			node.container.Insert(p, nodeInfo);
			if (refId)
				*refId = nodeInfo.refId;
			return true;
		}
	}
	return false;
}

bool DKObjectRefCounter::UnsetRefCounterIfEqual(void* p, RefCountValue c, DKAllocator** alloc)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair && pair->value.refCount == c)
		{
			if (alloc)
				*alloc = pair->value.allocator;
			node.container.Remove(p);
			return true;
		}
	}
	return false;
}

bool DKObjectRefCounter::UnsetRefCounterIfZero(void* p, DKAllocator** alloc)
{
	return UnsetRefCounterIfEqual(p, 0, alloc);
}

bool DKObjectRefCounter::UnsetRefCounter(void* p, RefCountValue* c, DKAllocator** alloc)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair)
		{
			if (c)
				*c = pair->value.refCount;
			if (alloc)
				*alloc = pair->value.allocator;
			node.container.Remove(p);
			return true;
		}
	}
	return false;
}

bool DKObjectRefCounter::IncrementRefCount(void* p, RefIdValue id)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair && pair->value.refId == id)
		{
			++(pair->value.refCount);
			return true;
		}
	}
	return false;
}

bool DKObjectRefCounter::IncrementRefCount(void* p)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair)
		{
			++(pair->value.refCount);
			return true;
		}
	}
	return false;
}

bool DKObjectRefCounter::DecrementRefCount(void* p)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair)
		{
			if (pair->value.refCount > 0)
			{
				--(pair->value.refCount);
			}
			else
			{
				DKERROR_THROW_DEBUG("Ref-Count already zero!");
				return false;
			}
			return true;
		}
	}
	return false;
}

bool DKObjectRefCounter::DecrementRefCountAndUnsetIfEqual(void* p, RefCountValue c, DKAllocator** alloc)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair)
		{
			DKASSERT_DEBUG(pair->value.refCount > 0);

			--(pair->value.refCount);

			if (pair->value.refCount == c)
			{
				if (alloc)
					*alloc = pair->value.allocator;
				node.container.Remove(p);
				return true;
			}
		}
	}
	return false;
}

bool DKObjectRefCounter::DecrementRefCountAndUnsetIfZero(void* p, DKAllocator** alloc)
{
	return DecrementRefCountAndUnsetIfEqual(p, 0, alloc);
}

bool DKObjectRefCounter::RefCount(void* p, RefCountValue* c)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair)
		{
			if (c)
				*c = pair->value.refCount;
			return true;
		}
	}
	return 0;
}

bool DKObjectRefCounter::RefId(void* p, RefIdValue* ref)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair)
		{
			if (ref)
				*ref = pair->value.refId;
			return true;
		}
	}
	return false;
}

DKMemoryLocation DKObjectRefCounter::Location(void* p)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair)
		{
			return pair->value.allocator->Location();
		}
	}
	return DKMemoryLocationCustom;	
}

DKAllocator* DKObjectRefCounter::Allocator(void* p)
{
	if (p)
	{
		AllocationNode& node = GetAllocationNode(p);
		AllocationNode::CriticalSection guard(node.lock);
		AllocationNode::Container::Pair* pair = node.container.Find(p);
		if (pair)
		{
			return pair->value.allocator;
		}
	}
	return NULL;	
}

size_t DKObjectRefCounter::TableSize(void)
{
	return Private::AllocatorTableLength;	
}

void DKObjectRefCounter::TableDump(size_t* tables)
{
	for (size_t i = 0; i < Private::AllocatorTableLength; ++i)
	{
		AllocationNode& node = GetAllocationNode(reinterpret_cast<void*>(i));
		tables[i] = node.container.Count();
	}
}
