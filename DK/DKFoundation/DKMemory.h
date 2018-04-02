//
//  File: DKMemory.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include <new>
#include <memory>
#include "../DKInclude.h"
#include "DKTypes.h"

namespace DKFoundation
{
	/// allocate memory from system heap.
	DKGL_API void* DKMemoryHeapAlloc(size_t);
	/// resize memory allocated by DKMemoryHeapAlloc
	DKGL_API void* DKMemoryHeapRealloc(void*, size_t);
	/// release memory allocated by DKMemoryHeapAlloc
	DKGL_API void  DKMemoryHeapFree(void*);

	/// allocate memory from system VM.
	DKGL_API void* DKMemoryVirtualAlloc(size_t);
	/// resize memory allocated by DKMemoryVirtualAlloc
	DKGL_API void* DKMemoryVirtualRealloc(void*, size_t);
	/// release memory allocated by DKMemoryVirtualAlloc
	DKGL_API void  DKMemoryVirtualFree(void*);
	/// query allocation size of memory allocated by DKMemoryVirtualAlloc
	DKGL_API size_t  DKMemoryVirtualSize(void*);

	/// query system page size (default allocation size)
	DKGL_API size_t DKMemoryPageSize(void);
	/// reserve page memory with given location if available.
	/// address should be aligned with value returned by DKMemoryPageSize or NULL
	DKGL_API void* DKMemoryPageReserve(void*, size_t);
	/// release memory reserved by DKMemoryPageReserve
	/// or decommitted by DKMemoryPageDecommit
	DKGL_API void DKMemoryPageRelease(void*);
	/// commit virtual memory reserved by DKMemoryPageReserve
	DKGL_API void DKMemoryPageCommit(void*, size_t);
	/// decommit virtual memory committed by DKMemoryPageCommit
	DKGL_API void DKMemoryPageDecommit(void*, size_t);

	/// allocate memory from memory pool
	/// If the request exceeds 32KB, the system allocates using VM.
	DKGL_API void* DKMemoryPoolAlloc(size_t);
	/// resize memory allocated by DKMemoryPoolAlloc
	DKGL_API void* DKMemoryPoolRealloc(void*, size_t);
	/// release memory allocated by DKMemoryPoolAlloc
	DKGL_API void  DKMemoryPoolFree(void*);
	/// purge unused memory pool chunks
	/// @note
	///   If you run out of memory, call DKAllocatorChain::Cleanup
	///   instead of calling DKMemoryPoolPurge, which purges memory pool only.
	DKGL_API size_t DKMemoryPoolPurge(void);
	/// query memory pool size
	DKGL_API size_t DKMemoryPoolSize(void);

	/**
	 @brief
		Memory pool allocation status info (for statistics)
		There are many sub-allocators in the memory pool.
		This is a single sub-allocator state for the pool.			
	 @code
		// getting allocation status!
		size_t numBuckets = DKMemoryPoolNumberOfBuckets();
		DKMemoryPoolBucketStatus* buckets = new DKMemoryPoolBucketStatus[numBuckets];
		DKMemoryPoolQueryAllocationStatus(buckets, numBuckets);
		for (int i = 0; i < numBuckets; ++i)
			printf("unit-size:%lu, allocated:%lu, reserved:%lu (usage:%.1f%%)\n",
				buckets[i].chunkSize,
				buckets[i].chunkSize * buckets[i].usedChunks, 
				buckets[i].chunkSize * (buckets[i].totalChunks - buckets[i].usedChunks),
				double(buckets[i].usedChunks) / double(buckets[i].totalChunks) * 100.0);
		printf("MemoryPool Usage: %.1fMB / %.1fMB\n", double(usedBytes) / (1024 * 1024), double(DKMemoryPoolSize()) / (1024 * 1024));
		delete[] buckets;
	 @endcode
	 @note
		This structure may contain a memory pool allocation range (that is, less than 32KB).
	 */
	struct DKMemoryPoolBucketStatus
	{
		size_t chunkSize;		///< allocation unit size of the allocator
		size_t totalChunks;		///< total chunks in the allocator
		size_t usedChunks;		///< allocated units
	};
	/// Get number of buckets, a bucket is a unit of sub-allocator in memory pool.
	/// this value does not change during run-time.
	DKGL_API size_t DKMemoryPoolNumberOfBuckets(void);
	/// Query allocation status of Memory-Pool.
	/// Do not call this function at exiting.
	/// @see DKMemoryPoolBucketStatus
	DKGL_API void DKMemoryPoolQueryAllocationStatus(DKMemoryPoolBucketStatus* status, size_t numBuckets);

	/// @brief track allocator location for debugging purpose
	/// you can provide your own allocator.
	/// @note
	/// you can use DKFileMap for file backed allocation
	/// @see DKFileMap
	enum DKMemoryLocation
	{
		DKMemoryLocationCustom = 0,	///< custom allocator, You can use DKFileMap as allocator
		DKMemoryLocationHeap,		///< allocated by DKMemoryHeapAlloc
		DKMemoryLocationVirtual,	///< allocated by DKMemoryVirtualAlloc
		DKMemoryLocationPool,		///< allocated by DKMemoryPoolAlloc
	};

	/// simple heap allocator type for template classes.
	/// if you need an allocator instance, use DKAllocator
	struct DKMemoryHeapAllocator
	{
		enum { Location = DKMemoryLocationHeap };
		static void* Alloc(size_t s)			{ return DKMemoryHeapAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryHeapRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryHeapFree(p); }
	};
	/// simple vm allocator type for template classes.
	/// if you need an allocator instance, use DKAllocator
	struct DKMemoryVirtualAllocator
	{
		enum { Location = DKMemoryLocationVirtual };
		static void* Alloc(size_t s)			{ return DKMemoryVirtualAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryVirtualRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryVirtualFree(p); }
	};
	/// simple memory pool allocator type for template class
	/// if you need an allocator instance, use DKAllocator
	struct DKMemoryPoolAllocator
	{
		enum { Location = DKMemoryLocationPool };
		static void* Alloc(size_t s)			{ return DKMemoryPoolAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryPoolRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryPoolFree(p); }
	};

#ifdef DKGL_HEAP_ALLOCATOR_IS_DEFAULT	// define if you don't want to use memory-pool
	using DKMemoryDefaultAllocator = DKMemoryHeapAllocator;
	constexpr DKMemoryLocation DKMemoryLocationDefault = DKMemoryLocationHeap;
#else
	using DKMemoryDefaultAllocator = DKMemoryPoolAllocator;
	constexpr DKMemoryLocation DKMemoryLocationDefault = DKMemoryLocationPool;
#endif

	/// Allocate memory using the default allocator (DKMemoryDefaultAllocator)
	FORCEINLINE void* DKMalloc(size_t s)
	{
		return DKMemoryDefaultAllocator::Alloc(s);
	}
	/// Reallocate memory using the default allocator (DKMemoryDefaultAllocator)
	FORCEINLINE void* DKRealloc(void* p, size_t s)
	{
		return DKMemoryDefaultAllocator::Realloc(p, s);
	}
	/// Release memory using the default allocator (DKMemoryDefaultAllocator)
	FORCEINLINE void DKFree(void* p)
	{
		DKMemoryDefaultAllocator::Free(p);
	}

	namespace Private
	{
		template <typename T> struct AllocationOperatorTest
		{
#if _MSC_VER > 1900
			template <typename A, A a> struct _Test;
			//void* T::operator new  (std::size_t);
			template <typename U> static auto _TestOperatorNew1(U*, _Test<void*(*)(std::size_t), &U::operator new>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorNew1(...)->DKFalse;
			enum { _HasOperatorNew1 = decltype(_TestOperatorNew1<T>(nullptr))::Value };
#if _MSVC_LANG >= 201703L
			//void* T::operator new  (std::size_t, std::align_val_t); //C++17
			template <typename U> static auto _TestOperatorNew2(U*, _Test<void*(*)(std::size_t, std::align_val_t), &U::operator new>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorNew2(...)->DKFalse;
			enum { _HasOperatorNew2 = decltype(_TestOperatorNew2<T>(nullptr))::Value };
#else
			enum { _HasOperatorNew2 = 0 };
#endif
			//void* T::operator new[](std::size_t);
			template <typename U> static auto _TestOperatorNewArray1(U*, _Test<void*(*)(std::size_t), &U::operator new[]>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorNewArray1(...)->DKFalse;
			enum { _HasOperatorNewArray1 = decltype(_TestOperatorNewArray1<T>(nullptr))::Value };
#if _MSVC_LANG >= 201703L
			//void* T::operator new[](std::size_t, std::align_val_t); //C++17
			template <typename U> static auto _TestOperatorNewArray2(U*, _Test<void*(*)(std::size_t, std::align_val_t), &U::operator new[]>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorNewArray2(...)->DKFalse;
			enum { _HasOperatorNewArray2 = decltype(_TestOperatorNewArray2<T>(nullptr))::Value };
#else
			enum { _HasOperatorNewArray2 = 0 };
#endif

			//void T::operator delete  (void*);
			template <typename U> static auto _TestOperatorDelete1(U*, _Test<void(*)(void*), &U::operator delete>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorDelete1(...)->DKFalse;
			enum { _HasOperatorDelete1 = decltype(_TestOperatorDelete1<T>(nullptr))::Value };
			//void T::operator delete  (void*, std::size_t);
			template <typename U> static auto _TestOperatorDelete2(U*, _Test<void(*)(void*, std::size_t), &U::operator delete>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorDelete2(...)->DKFalse;
			enum { _HasOperatorDelete2 = decltype(_TestOperatorDelete2<T>(nullptr))::Value };
#if _MSVC_LANG >= 201703L
			//void T::operator delete  (void*, std::align_val_t); //C++17
			template <typename U> static auto _TestOperatorDelete3(U*, _Test<void(*)(void*, std::align_val_t), &U::operator delete>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorDelete3(...)->DKFalse;
			enum { _HasOperatorDelete3 = decltype(_TestOperatorDelete3<T>(nullptr))::Value };
			//void T::operator delete  (void*, std::size_t, std::align_val_t); //C++17
			template <typename U> static auto _TestOperatorDelete4(U*, _Test<void(*)(void*, std::size_t, std::align_val_t), &U::operator delete>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorDelete4(...)->DKFalse;
			enum { _HasOperatorDelete4 = decltype(_TestOperatorDelete4<T>(nullptr))::Value };
#else
			enum { _HasOperatorDelete3 = 0 };
			enum { _HasOperatorDelete4 = 0 };
#endif
			//void T::operator delete[](void*);
			template <typename U> static auto _TestOperatorDeleteArray1(U*, _Test<void(*)(void*), &U::operator delete[]>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorDeleteArray1(...)->DKFalse;
			enum { _HasOperatorDeleteArray1 = decltype(_TestOperatorDeleteArray1<T>(nullptr))::Value };
			//void T::operator delete[](void*, std::size_t);
			template <typename U> static auto _TestOperatorDeleteArray2(U*, _Test<void(*)(void*, std::size_t), &U::operator delete[]>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorDeleteArray2(...)->DKFalse;
			enum { _HasOperatorDeleteArray2 = decltype(_TestOperatorDeleteArray2<T>(nullptr))::Value };
#if _MSVC_LANG >= 201703L
			//void T::operator delete[](void*, std::align_val_t); //C++17
			template <typename U> static auto _TestOperatorDeleteArray3(U*, _Test<void(*)(void*, std::align_val_t), &U::operator delete[]>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorDeleteArray3(...)->DKFalse;
			enum { _HasOperatorDeleteArray3 = decltype(_TestOperatorDeleteArray3<T>(nullptr))::Value };
			//void T::operator delete[](void*, std::size_t, std::align_val_t); //C++17
			template <typename U> static auto _TestOperatorDeleteArray4(U*, _Test<void(*)(void*, std::size_t, std::align_val_t), &U::operator delete[]>* = nullptr)->DKTrue;
			template <typename U> static auto _TestOperatorDeleteArray4(...)->DKFalse;
			enum { _HasOperatorDeleteArray4 = decltype(_TestOperatorDeleteArray4<T>(nullptr))::Value };
#else
			enum { _HasOperatorDeleteArray3 = 0 };
			enum { _HasOperatorDeleteArray4 = 0 };
#endif

			enum { HasOperatorNew = _HasOperatorNew1 || _HasOperatorNew2 };
			enum { HasOperatorNewArray = _HasOperatorNewArray1 || _HasOperatorNewArray2 };
			enum { HasOperatorDelete = _HasOperatorDelete1 || _HasOperatorDelete2 || _HasOperatorDelete3 || _HasOperatorDelete4 };
			enum { HasOperatorDeleteArray = _HasOperatorDeleteArray1 || _HasOperatorDeleteArray2 || _HasOperatorDeleteArray3 || _HasOperatorDeleteArray4 };
#else
			template <typename U> static auto _TestOperatorNew(decltype(&U::operator new))->DKTrue;
			template <typename U> static auto _TestOperatorNew(...)->DKFalse;
			template <typename U> static auto _TestOperatorNewArray(decltype(&U::operator new[]))->DKTrue;
			template <typename U> static auto _TestOperatorNewArray(...)->DKFalse;
			template <typename U> static auto _TestOperatorDelete(decltype(&U::operator delete))->DKTrue;
			template <typename U> static auto _TestOperatorDelete(...)->DKFalse;
			template <typename U> static auto _TestOperatorDeleteArray(decltype(&U::operator delete[]))->DKTrue;
			template <typename U> static auto _TestOperatorDeleteArray(...)->DKFalse;

			enum { HasOperatorNew = decltype(_TestOperatorNew<T>(0))::Value };
			enum { HasOperatorNewArray = decltype(_TestOperatorNewArray<T>(0))::Value };
			enum { HasOperatorDelete = decltype(_TestOperatorDelete<T>(0))::Value };
			enum { HasOperatorDeleteArray = decltype(_TestOperatorDeleteArray<T>(0))::Value };
#endif
		};

	}

	// disable custom memory functions:
	// DKRawPtrNew, DKRawPtrNewArray, DKRawPtrDelete, DKRawPtrDeleteArray
#if 0
	template <typename T, typename... Args>
	inline T* DKRawPtrNew(Args&&... args)
	{
		if (Private::AllocationOperatorTest<T>::HasOperatorNew)
			return new T(std::forward<Args>(args)...);

		void* p = DKMalloc(sizeof(T));
		if (p)
		{
			try {
				return ::new(p) T(std::forward<Args>(args)...); 
			} catch (...) {
				DKFree(p);
				throw;
			}
		}
		throw std::bad_alloc();
	}
	template <typename T>
	inline T* DKRawPtrNewArray(size_t num)
	{
		if (Private::AllocationOperatorTest<T>::HasOperatorNewArray)
			return new T[num];

		if (num > 0)
		{
			void* p = DKMalloc(sizeof(T) * num);
			if (p)
			{
				T* ptr = (T*)p;
				size_t initialized = 0;
				try {
					for (initialized = 0; initialized < num; ++initialized)
					{
						::new(std::addressof(ptr[initialized])) T();
					}
				} catch(...) {
					for (size_t i = 0; i < initialized; ++i)
					{
						ptr[i].~T();
					}
					DKFree(p);
					throw;
				}
			}
		}
		throw std::bad_alloc();
	}
	template <typename T>
	inline void DKRawPtrDelete(T* p)
	{
		if (Private::AllocationOperatorTest<T>::HasOperatorDelete)
			delete p;
		else if (p)
		{
			void* addr = DKTypeBaseAddressCast(p);
			p->~T();
			DKFree(addr);
		}
	}
	template <typename T>
	inline void DKRawPtrDeleteArray(T* p, size_t num)
	{
		if (Private::AllocationOperatorTest<T>::HasOperatorDeleteArray)
			delete[] p;
		else if (p)
		{
			if (num > 0)
			{
				for (size_t i = 0; i < num; ++i)
				{
					p[i].~T();
				}
			}
			DKFree(p);
		}
	}
#endif
}
