//
//  File: DKBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
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

namespace DKFoundation
{
	class DKLIB_API DKBuffer : public DKData
	{
	public:
		DKBuffer(DKAllocator& alloc = DKAllocator::DefaultAllocator());
		DKBuffer(const DKData* p, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		DKBuffer(const void* p, size_t s, DKAllocator& alloc = DKAllocator::DefaultAllocator());	// p 가 NULL 이면 s 만큼 버퍼 생성함 (0 으로 초기화됨)
		DKBuffer(const DKBuffer&);
		DKBuffer(DKBuffer&&);
		~DKBuffer(void);
		
		size_t Length(void) const;
		size_t CopyContent(void* p, size_t offset, size_t length) const;

		DKObject<DKBuffer> Compress(DKAllocator& alloc = DKAllocator::DefaultAllocator()) const;
		DKObject<DKBuffer> Decompress(DKAllocator& alloc = DKAllocator::DefaultAllocator()) const;
		static DKObject<DKBuffer> Compress(const void* p, size_t len, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Decompress(const void* p, size_t len, DKAllocator& alloc = DKAllocator::DefaultAllocator());

		bool Encode(DKStringU8& strOut) const;
		bool Encode(DKStringW& strOut) const;
		static DKObject<DKBuffer> Decode(const DKStringU8& str, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Decode(const DKStringW& str, DKAllocator& alloc = DKAllocator::DefaultAllocator());

		// compress and encode (base64)
		bool CompressEncode(DKStringU8& strOut) const;
		bool CompressEncode(DKStringW& strOut) const;
		// base64-decode and decompress
		static DKObject<DKBuffer> DecodeDecompress(const DKStringU8& s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> DecodeDecompress(const DKStringW& s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		
		// create object from file or URL.
		static DKObject<DKBuffer> Create(const DKString& url, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(const void* p, size_t s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(const DKData* p, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(DKStream* s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		
		size_t SetContent(const void* p, size_t s);// create zero-fill buffer if p is NULL.
		size_t SetContent(const DKData* p);
		
		DKBuffer& operator = (const DKBuffer&);
		DKBuffer& operator = (DKBuffer&&);
		
		bool IsReadable(void) const		{return true;}
		bool IsWritable(void) const		{return true;}
		bool IsExcutable(void) const	{return false;}
	
		void SwitchAllocator(DKAllocator& alloc);
		DKAllocator& Allocator(void) const;

	protected:
		void* LockContent(void);
		void UnlockContent(void);

	private:
		void*	contentPtr;
		size_t	contentLength;
		DKAllocator* allocator;
	};
}
