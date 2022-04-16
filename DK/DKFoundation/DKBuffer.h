//
//  File: DKBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKSpinLock.h"
#include "DKString.h"
#include "DKData.h"
#include "DKStream.h"
#include "DKAllocator.h"
#include "DKCompressor.h"

namespace DKFoundation
{
	/// @brief Create writable memory object from URL or other stream, memory object.
	/// provide compression, base64-encoding functionality.
	/// thread safe.
	///
	/// @note
	///  Encode means 'encode with base64' in this class
	class DKGL_API DKBuffer : public DKData
	{
	public:
		DKBuffer(DKAllocator& alloc = DKAllocator::DefaultAllocator());
		DKBuffer(const DKData* p, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		/// if p is NULL, an empty buffer with given length will be created.
		DKBuffer(const void* p, size_t s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		DKBuffer(const DKBuffer&);
		DKBuffer(DKBuffer&&);
		~DKBuffer();
		
		size_t Length() const override;
		size_t CopyContents(void* p, size_t offset, size_t length) const;

		bool SetLength(size_t);

		/// compress / decompress data
		DKObject<DKBuffer> Compress(const DKCompressor&, DKAllocator& alloc = DKAllocator::DefaultAllocator()) const;
		DKObject<DKBuffer> Decompress(DKAllocator& alloc = DKAllocator::DefaultAllocator()) const;
		static DKObject<DKBuffer> Compress(const DKCompressor&, const void* p, size_t len, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Decompress(const void* p, size_t len, DKAllocator& alloc = DKAllocator::DefaultAllocator());

		/// base64 encode / decode
		bool Base64Encode(DKStringU8& strOut) const;
		bool Base64Encode(DKStringW& strOut) const;
		static DKObject<DKBuffer> Base64Decode(const DKStringU8& str, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Base64Decode(const DKStringW& str, DKAllocator& alloc = DKAllocator::DefaultAllocator());

		/// create object from file or URL.
		static DKObject<DKBuffer> Create(const DKString& url, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(const void* p, size_t s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(const DKData* p, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		static DKObject<DKBuffer> Create(DKStream* s, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		
		size_t SetContents(const void* p, size_t s); ///< create zero-fill buffer if p is NULL.
		size_t SetContents(const DKData* p);
		
		DKBuffer& operator = (const DKBuffer&);
		DKBuffer& operator = (DKBuffer&&);

		bool IsReadable() const override { return true; }
		bool IsWritable() const	override { return true; }
		bool IsExcutable() const override { return false; }
		bool IsTransient() const override { return false; }
	
		void SwitchAllocator(DKAllocator& alloc);
		DKAllocator& Allocator() const;

		const void* Contents() const override;
		void* MutableContents() override;

	private:
		void*	contentsPtr;
		size_t	contentsLength;
		DKAllocator* allocator;
	};
}
