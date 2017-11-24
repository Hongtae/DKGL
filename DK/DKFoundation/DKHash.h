//
//  File: DKHash.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKEndianness.h"
#include "DKString.h"
#include "DKStream.h"

namespace DKFoundation
{
	/// @brief Hash context, returned by DKHash
	///
	/// You can access digest result or represent as a string.
	template <typename BASE, int BIT> struct DKHashResult
	{
		enum {
			UnitSize = sizeof(BASE),
			Length = BIT / (sizeof(BASE) * 8),
		};
		BASE digest[Length]; ///< hash digest in unit size (usually uint32_t)

		bool operator == (const DKHashResult& r) const		{return memcmp(digest, r.digest, sizeof(digest)) == 0;}
		bool operator != (const DKHashResult& r) const		{return memcmp(digest, r.digest, sizeof(digest)) != 0;}
		bool operator > (const DKHashResult& r) const		{return memcmp(digest, r.digest, sizeof(digest)) > 0;}
		bool operator < (const DKHashResult& r) const		{return memcmp(digest, r.digest, sizeof(digest)) < 0;}
		bool operator >= (const DKHashResult& r) const		{return memcmp(digest, r.digest, sizeof(digest)) >= 0;}
		bool operator <= (const DKHashResult& r) const		{return memcmp(digest, r.digest, sizeof(digest)) <= 0;}

		DKString String(void) const ///< represent hash digest as a string
		{
			char buff[Length * 8];
			char* tmp = buff;
			for (size_t i = 0; i < Length; ++i)
			{
				BASE val = DKSystemToBigEndian(digest[i]);
				for (size_t k = 0; k < sizeof(BASE); ++k)
				{
					unsigned char v = reinterpret_cast<unsigned char*>(&val)[k];
					unsigned char v1 = (v >> 4) & 0x0f;
					unsigned char v2 = v & 0x0f;

					*(tmp++) = v1 <= 9 ? v1 + '0' : 'a' + (v1 - 10);
					*(tmp++) = v2 <= 9 ? v2 + '0' : 'a' + (v2 - 10);
				}
			}
			return DKString(buff, Length * 8);
		}
	};
	
	/// Hash context for CRC32
	typedef DKHashResult<uint32_t, 32>	DKHashResult32;
	typedef DKHashResult<uint32_t, 32>	DKHashResultCRC32;
	/// Hash context for MD5
	typedef DKHashResult<uint32_t, 128>	DKHashResult128;
	typedef DKHashResult<uint32_t, 128>	DKHashResultMD5;
	/// Hash context for SHA1
	typedef DKHashResult<uint32_t, 160>	DKHashResult160;
	typedef DKHashResult<uint32_t, 160>	DKHashResultSHA1;
	/// Hash context for SHA2 (SHA-224)
	typedef DKHashResult<uint32_t, 224>	DKHashResult224;
	typedef DKHashResult<uint32_t, 224>	DKHashResultSHA224;
	/// Hash context for SHA2 (SHA-256)
	typedef DKHashResult<uint32_t, 256>	DKHashResult256;
	typedef DKHashResult<uint32_t, 256>	DKHashResultSHA256;
	/// Hash context for SHA2 (SHA-384)
	typedef DKHashResult<uint32_t, 384>	DKHashResult384;
	typedef DKHashResult<uint32_t, 384>	DKHashResultSHA384;
	/// Hash context for SHA2 (SHA-512)
	typedef DKHashResult<uint32_t, 512>	DKHashResult512;
	typedef DKHashResult<uint32_t, 512>	DKHashResultSHA512;

	/// CRC32
	DKGL_API DKHashResultCRC32 DKHashCRC32(const void* p, size_t len);
	DKGL_API bool DKHashCRC32(DKStream*, DKHashResultCRC32&);
	/// MD5
	DKGL_API DKHashResultMD5 DKHashMD5(const void* p, size_t len);
	DKGL_API bool DKHashMD5(DKStream*, DKHashResultMD5&);
	/// SHA1
	DKGL_API DKHashResultSHA1 DKHashSHA1(const void* p, size_t len);
	DKGL_API bool DKHashSHA1(DKStream*, DKHashResultSHA1&);
	/// SHA2 (SHA-224)
	DKGL_API DKHashResultSHA224 DKHashSHA224(const void* p, size_t len);
	DKGL_API bool DKHashSHA224(DKStream*, DKHashResultSHA224&);
	/// SHA2 (SHA-256)
	DKGL_API DKHashResultSHA256 DKHashSHA256(const void* p, size_t len);
	DKGL_API bool DKHashSHA256(DKStream*, DKHashResultSHA256&);
	/// SHA2 (SHA-384)
	DKGL_API DKHashResultSHA384 DKHashSHA384(const void* p, size_t len);
	DKGL_API bool DKHashSHA384(DKStream*, DKHashResultSHA384&);
	/// SHA2 (SHA-512)
	DKGL_API DKHashResultSHA512 DKHashSHA512(const void* p, size_t len);
	DKGL_API bool DKHashSHA512(DKStream*, DKHashResultSHA512&);

	/// @brief Hash calculation class
	///
	/// Following hash digest algorithms are supported.\n
	/// CRC32, MD5, SHA1, SHA2, SHA-224, SHA-256, SHA-384, SHA-512
	class DKGL_API DKHash
	{
	public:
		virtual ~DKHash(void);

		void Initialize(void);
		void Update(const void*, size_t);
		void Finalize(void);

		struct Context;
	protected:
		enum Type
		{
			Type32,		///< CRC32
			Type128,	///< MD5
			Type160,	///< SHA1
			Type224,	///< SHA2 (SHA-224)
			Type256,	///< SHA2 (SHA-256)
			Type384,	///< SHA2 (SHA-384)
			Type512,	///< SHA2 (SHA-512)
		};
		enum Name
		{
			CRC32      = Type32,
			MD5        = Type128,
			SHA1       = Type160,
			SHA2_224   = Type224,
			SHA2_256   = Type256,
			SHA2_384   = Type384,
			SHA2_512   = Type512,
		};

		DKHash(Type t);
		DKHash(Name n) : DKHash((Type)n) {}

		const Type type;
		bool finalized;
		Context* ctxt;
	};
	class DKGL_API DKHash32 : public DKHash
	{
	public:
		DKHash32(void) : DKHash(Type32) {}
		DKHashResult32 Result(void) const;
	};
	class DKGL_API DKHash128 : public DKHash
	{
	public:
		DKHash128(void) : DKHash(Type128) {}
		DKHashResult128 Result(void) const;
	};
	class DKGL_API DKHash160 : public DKHash
	{
	public:
		DKHash160(void) : DKHash(Type160) {}
		DKHashResult160 Result(void) const;
	};
	class DKGL_API DKHash224 : public DKHash
	{
	public:
		DKHash224(void) : DKHash(Type224) {}
		DKHashResult224 Result(void) const;
	};
	class DKGL_API DKHash256 : public DKHash
	{
	public:
		DKHash256(void) : DKHash(Type256) {}
		DKHashResult256 Result(void) const;
	};
	class DKGL_API DKHash384 : public DKHash
	{
	public:
		DKHash384(void) : DKHash(Type384) {}
		DKHashResult384 Result(void) const;
	};
	class DKGL_API DKHash512 : public DKHash
	{
	public:
		DKHash512(void) : DKHash(Type512) {}
		DKHashResult512 Result(void) const;
	};
}
