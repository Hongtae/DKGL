//
//  File: DKBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKSpinLock.h"
#include "DKString.h"
#include "DKData.h"
#include "DKStream.h"
#include "DKAllocator.h"

////////////////////////////////////////////////////////////////////////////////
// DKBuffer
//
// create memory object from URL or other stream, memory object.
// provide compression, base64-encoding functionality.
// thread safe.
//
// Note:
//  Encode means 'encode with base64' in this class
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	enum class DKCompressor
	{
		Deflate,	// good compression ratio.
		LZ4,		// fast compression, fast decompression.
		LZ4HC,		// fast decompression, better compression ratio than LZ4.
	};

	class DKGL_API DKBuffer : public DKData
	{
	public:
		DKBuffer(DKAllocator& alloc = DKAllocator::DefaultAllocator());
		DKBuffer(const DKData* p, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		// if p is NULL, an empty buffer with given length will be created.
		DKBuffer(const void* p, size_t s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		DKBuffer(const DKBuffer&);
		DKBuffer(DKBuffer&&);
		~DKBuffer(void);
		
		size_t Length(void) const;
		size_t CopyContent(void* p, size_t offset, size_t length) const;

		// compress / decompress data
		DKObject<DKBuffer> Compress(DKCompressor, DKAllocator& alloc = DKAllocator::DefaultAllocator()) const;
		DKObject<DKBuffer> Decompress(DKAllocator& alloc = DKAllocator::DefaultAllocator()) const;
		static DKObject<DKBuffer> Compress(const void* p, size_t len, DKCompressor, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Decompress(const void* p, size_t len, DKAllocator& alloc = DKAllocator::DefaultAllocator());

		// base64 encode / decode
		bool Base64Encode(DKStringU8& strOut) const;
		bool Base64Encode(DKStringW& strOut) const;
		static DKObject<DKBuffer> Base64Decode(const DKStringU8& str, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Base64Decode(const DKStringW& str, DKAllocator& alloc = DKAllocator::DefaultAllocator());

		// create object from file or URL.
		static DKObject<DKBuffer> Create(const DKString& url, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(const void* p, size_t s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(const DKData* p, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(DKStream* s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		
		size_t SetContent(const void* p, size_t s); // create zero-fill buffer if p is NULL.
		size_t SetContent(const DKData* p);
		
		DKBuffer& operator = (const DKBuffer&);
		DKBuffer& operator = (DKBuffer&&);
		
		bool IsReadable(void) const		{return true;}
		bool IsWritable(void) const		{return true;}
		bool IsExcutable(void) const	{return false;}
	
		void SwitchAllocator(DKAllocator& alloc);
		DKAllocator& Allocator(void) const;

		const void* LockShared(void) const;
		bool TryLockShared(const void**) const;
		void UnlockShared(void) const;

		void* LockExclusive(void);
		bool TryLockExclusive(void**);
		void UnlockExclusive(void);

	protected:
		void* LockContent(void);
		void UnlockContent(void);

	private:
		void*	contentPtr;
		size_t	contentLength;
		DKSharedLock sharedLock;
		DKAllocator* allocator;
	};
}
