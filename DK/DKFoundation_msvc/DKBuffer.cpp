//
//  File: DKBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#define DKLIB_EXTDEPS_ZLIB
#define DKLIB_EXTDEPS_LIBXML

#include <ctype.h>
#include <wctype.h>
#include "../lib/ExtDeps.h"
#include "DKString.h"
#include "DKFile.h"
#include "DKBuffer.h"
#include "DKHash.h"
#include "DKMemory.h"
#include "DKArray.h"
#include "DKBufferStream.h"
#include "DKMap.h"
#include "DKSpinLock.h"
#include "DKOrderedArray.h"
#include "DKLog.h"

namespace DKFoundation
{
	namespace Private
	{
		namespace
		{
			// base64 encode/decode
			// implemented based on http://www.adp-gmbh.ch/cpp/common/base64.html
			template <typename BaseCharT>
			struct Base64CharPair
			{
				BaseCharT key;
				size_t index;
				bool operator > (const Base64CharPair& p) const		{return this->key > p.key;}
				bool operator < (const Base64CharPair& p) const		{return this->key < p.key;}
				bool operator == (const Base64CharPair& p) const	{return this->key == p.key;}
			};		
			template <typename BaseCharT> struct Base64;
			template <> struct Base64<char>
			{
				static const char* Base64Chars(void)
				{
					return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
				}
				static inline bool IsBase64(char c)
				{
					return ((c == '+') || (c == '/') || isalnum(c));
				}
			};
			template <> struct Base64<wchar_t>
			{
				static const wchar_t* Base64Chars(void)
				{
					return L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
				}
				static inline bool IsBase64(wchar_t c)
				{
					return ((c == L'+') || (c == '/') || iswalnum(c));
				}
			};
			template <typename BaseCharT> int IndexOfBase64Char(BaseCharT c)
			{
#if 1
				typedef DKOrderedArray<Base64CharPair<BaseCharT>> IndexMap;
				static DKSpinLock indexMapInitLock;
				static bool indexMapInitialized = false;
				static IndexMap indexMap(DKArraySortAscending<Base64CharPair<BaseCharT>>);

				if (indexMapInitialized == false)
				{
					DKCriticalSection<DKSpinLock> guard(indexMapInitLock);
					if (indexMapInitialized == false)
					{
						indexMap.Clear();
						indexMap.Reserve(64);
						for (size_t i = 0; i < 64; ++i)
						{
							Base64CharPair<BaseCharT> bp = { Base64<BaseCharT>::Base64Chars()[i], i };
							indexMap.Insert(bp);
						}
						indexMapInitialized = true;
						//DKLog("DKBuffer::Base64 Character-Map Generated for type:%s\n", typeid(BaseCharT).name());
					}
				}
				DKASSERT_DEBUG(indexMap.Count() == 64);

				Base64CharPair<BaseCharT> bp = {c, 0};
				typename IndexMap::Index index = indexMap.Find(bp);
				if (index != IndexMap::invalidIndex)
				{
					return static_cast<int>(indexMap.Value(index).index);
				}
#else
				for (int i = 0; i < 64; ++i)
					if (c == Base64<BaseCharT>::Base64Chars()[i])
						return i;
#endif
				return -1;

			}
			template <typename BaseCharT> void Base64Encode(unsigned char const* bytes_to_encode, size_t in_len, DKArray<BaseCharT>& output)
			{
				output.Clear();
				output.Reserve(((in_len * 4) / 3) + 3);

				int i = 0;
				int j = 0;
				unsigned char char_array_3[3];
				unsigned char char_array_4[4];

				while (in_len--) {
					char_array_3[i++] = *(bytes_to_encode++);
					if (i == 3) {
						char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
						char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
						char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
						char_array_4[3] = char_array_3[2] & 0x3f;

						for(i = 0; (i <4) ; i++)
							output.Add(Base64<BaseCharT>::Base64Chars()[char_array_4[i]]);
						i = 0;
					}
				}

				if (i)
				{
					for(j = i; j < 3; j++)
						char_array_3[j] = '\0';

					char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
					char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
					char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
					char_array_4[3] = char_array_3[2] & 0x3f;

					for (j = 0; (j < i + 1); j++)
						output.Add(Base64<BaseCharT>::Base64Chars()[char_array_4[j]]);

					while((i++ < 3))
						output.Add('=');
				}
				//return output;
			}
			template <typename BaseCharT> bool Base64Decode(const BaseCharT* encoded_string, size_t in_len, DKBuffer* output)
			{
				int i = 0;
				int j = 0;
				int in_ = 0;
				char char_array_4[4], char_array_3[3];
				DKArray<char> result;
				result.Reserve((in_len * 3) / 4 + 3);

				while (in_len-- && ( encoded_string[in_] != '=') && Base64<BaseCharT>::IsBase64(encoded_string[in_]))
				{
					char_array_4[i++] = static_cast<char>(encoded_string[in_]); in_++;
					if (i == 4)
					{
						for (i = 0; i < 4; ++i)
							char_array_4[i] = IndexOfBase64Char(char_array_4[i]);

						char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
						char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
						char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

						for (i = 0; (i < 3); i++)
							result.Add(char_array_3[i]);

						i = 0;
					}
				}

				if (i) {
					for (j = i; j < 4; j++)
						char_array_4[j] = 0;

					for (j = 0; j < 4; j++)
						char_array_4[j] = IndexOfBase64Char(char_array_4[j]);

					char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
					char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
					char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

					for (j = 0; (j < i - 1); j++)
						result.Add(char_array_3[j]);
				}
				if (result.Count())
				{
					output->SetContent((const char*)result, result.Count());
					return true;
				}
				return false;
			}
			DKObject<DKBuffer> GetHTTPContent(const DKString& url)
			{
				if (url.Length() == 0)
					return NULL;

				DKStringU8 filenameUTF8(url);
				if (filenameUTF8 == "" || filenameUTF8.Bytes() == 0)
					return NULL;

				// initialize http
				xmlNanoHTTPInit();

				// connect
				void* ctxt = xmlNanoHTTPOpen((const char*)filenameUTF8, NULL);
				if (ctxt == NULL)
					return NULL;

				DKObject<DKBuffer> data = NULL;

				int len = xmlNanoHTTPContentLength(ctxt);
				if (len > 0)
				{
					void* buffer = DKMemoryHeapAlloc(len);
					int recv = xmlNanoHTTPRead(ctxt, buffer, len); // download
					if (recv == len)
					{
						data = DKBuffer::Create(buffer, len);
					}
					DKMemoryHeapFree(buffer);
				}
				xmlNanoHTTPClose(ctxt);
				xmlNanoHTTPCleanup();

				return data;
			}
			DKObject<DKBuffer> GetFTPContent(const DKString& url)
			{
				if (url.Length() == 0)
					return NULL;

				DKStringU8 filenameUTF8(url);
				if (filenameUTF8 == "" || filenameUTF8.Bytes() == 0)
					return NULL;

				xmlNanoFTPInit();

				void* ctxt = xmlNanoFTPOpen((const char*)filenameUTF8);
				if (ctxt == NULL)
					return NULL;

				DKObject<DKBuffer> data = NULL;

				size_t	bufferSize = 4096;
				size_t	received = 0;
				char*	buffer = (char*)DKMemoryHeapAlloc(bufferSize);

				while (true)
				{
					int recv = xmlNanoFTPRead(ctxt, buffer + received, bufferSize - received);
					received += recv;

					if (recv < 0)		// error!
					{
						break;
					}
					else if (recv == 0)	// succeeded.
					{
						data = DKBuffer::Create(buffer, received);
						break;
					}
					if (received + 100 > bufferSize)
					{
						bufferSize += 4096;
						buffer = (char*)DKMemoryHeapRealloc(buffer, bufferSize);
						if (buffer == NULL)
							DKERROR_THROW("DKMemoryHeapRealloc() failed!");
					}
				}
				DKMemoryHeapFree(buffer);

				xmlNanoFTPClose(ctxt);
				xmlNanoFTPCleanup();

				return data;
			}
		}
	}
}

using namespace DKFoundation;

DKBuffer::DKBuffer(DKAllocator& alloc)
	: contentPtr(NULL)
	, contentLength(0)
	, allocator(&alloc)
{
}

DKBuffer::DKBuffer(const void* p, size_t s, DKAllocator& alloc)
	: contentPtr(NULL)
	, contentLength(0)
	, allocator(&alloc)
{
	SetContent(p, s);
}

DKBuffer::DKBuffer(const DKData* buff, DKAllocator& alloc)
	: contentPtr(NULL)
	, contentLength(0)
	, allocator(&alloc)
{
	SetContent(buff);
}

DKBuffer::DKBuffer(const DKBuffer& b)
	: contentPtr(NULL)
	, contentLength(0)
	, allocator(b.allocator)
{
	SetContent(b.contentPtr, b.contentLength);
}

DKBuffer::DKBuffer(DKBuffer&& b)
	: contentPtr(NULL)
	, contentLength(0)
	, allocator(b.allocator)
{
	this->contentPtr = b.contentPtr;
	this->contentLength = b.contentLength;
	b.contentPtr = NULL;
	b.contentLength = 0;
}

DKBuffer::~DKBuffer(void)
{
	if (contentPtr)
		allocator->Dealloc(contentPtr);
}

size_t DKBuffer::Length(void) const
{
	return contentLength;
}

bool DKBuffer::Encode(DKStringU8& strOut) const
{
	bool ret = false;
	const void* p = this->LockShared();
	size_t inputLength = this->Length();
	if (p && inputLength > 0)
	{	
		DKArray<char> result;
		Private::Base64Encode((const unsigned char*)p, inputLength, result);
		if (result.Count() > 0)
		{
			strOut.SetValue((const char*)result, result.Count());
			ret = true;
		}
	}
	this->UnlockShared();
	return ret;
}

bool DKBuffer::Encode(DKStringW& strOut) const
{
	bool ret = false;
	const void* p = this->LockShared();
	size_t inputLength = this->Length();
	if (p && inputLength > 0)
	{
		DKArray<wchar_t> result;
		Private::Base64Encode((const unsigned char*)p, inputLength, result);
		if (result.Count() > 0)
		{
			strOut.SetValue((const wchar_t*)result, result.Count());
			ret = true;
		}
	}
	this->UnlockShared();
	return ret;
}

DKObject<DKBuffer> DKBuffer::Decode(const DKStringU8& str, DKAllocator& alloc)
{
	DKObject<DKBuffer> buff = DKOBJECT_NEW DKBuffer(alloc);
	if (Private::Base64Decode((const char*)str, str.Bytes(), buff))
		return buff;
	return NULL;
}

DKObject<DKBuffer> DKBuffer::Decode(const DKStringW& str, DKAllocator& alloc)
{
	DKObject<DKBuffer> buff = DKOBJECT_NEW DKBuffer(alloc);
	if (Private::Base64Decode((const wchar_t*)str, str.Length(), buff))
		return buff;
	return NULL;
}

DKObject<DKBuffer> DKBuffer::Compress(DKAllocator& alloc) const
{
	const void* p = this->LockShared();
	size_t inputLength = this->Length();
	DKObject<DKBuffer> result = Compress(p, inputLength, alloc);
	this->UnlockShared();
	return result;
}

DKObject<DKBuffer> DKBuffer::Compress(const void* p, size_t len, DKAllocator& alloc)
{
	DKObject<DKBuffer> result = NULL;
	if (p && len > 0)
	{
		// Z_DEFAULT_COMPRESSION -> 6
		int compressLevel = 9;
		uLongf compressedSize = len + (len / 10) + 100;
		void* compressed = DKMemoryHeapAlloc(compressedSize);		
		if (compress2((Bytef*)compressed, &compressedSize, (const Bytef*)p, len, compressLevel) == Z_OK)
		{
			result = DKBuffer::Create(compressed, compressedSize, alloc);
		}		
		DKMemoryHeapFree(compressed);
	}
	return result;
}


DKObject<DKBuffer> DKBuffer::Decompress(DKAllocator& alloc) const
{
	const void* p = this->LockShared();
	size_t len = this->Length();
	DKObject<DKBuffer> result = Decompress(p, len, alloc);
	this->UnlockShared();
	return result;
}

#define COMPRESS_DEFAULT_BLOCK		0x4000

DKObject<DKBuffer> DKBuffer::Decompress(const void* p, size_t len, DKAllocator& alloc)
{
	DKObject<DKBuffer> result = NULL;

	uLongf inputLength = len;
	if (p && inputLength > 0)
	{
		uLongf outputLength = COMPRESS_DEFAULT_BLOCK;

		Bytef* inputSource = (Bytef*)p;
		Bytef* output = (Bytef*)DKMemoryHeapAlloc(outputLength);

		z_stream stream;
		stream.next_in = inputSource;
		stream.avail_in = inputLength;
		stream.next_out = output;
		stream.avail_out = outputLength;

		stream.zalloc = (alloc_func)0;
		stream.zfree = (free_func)0;

		if (inflateInit(&stream) == Z_OK)
		{
			int err = Z_OK;
			while (err == Z_OK)
			{
				err = inflate(&stream, Z_SYNC_FLUSH);

				if (err == Z_STREAM_END)
				{
					result = DKBuffer::Create(output, stream.total_out, alloc);
					break;
				}
				else if (err == Z_OK)
				{
					output = (Bytef*)DKMemoryHeapRealloc(output, stream.total_out + COMPRESS_DEFAULT_BLOCK);
					if (output == NULL)
					{
						DKERROR_THROW_DEBUG("DKMemoryHeapRealloc() failed!");
						DKLog("ERROR: DKMemoryHeapRealloc() failed.");
						break;
					}

					stream.avail_out += COMPRESS_DEFAULT_BLOCK;
					stream.next_out = output + stream.total_out;
				}				
			}
			inflateEnd(&stream);
		}
		DKMemoryHeapFree(output);		
	}
	return result;
}

bool DKBuffer::CompressEncode(DKStringU8& strOut) const
{
	DKObject<DKBuffer> d = this->Compress();
	if (d)
		return d->Encode(strOut);
	return false;
}

bool DKBuffer::CompressEncode(DKStringW& strOut) const
{
	DKObject<DKBuffer> d = this->Compress();
	if (d)
		return d->Encode(strOut);
	return false;
}

DKObject<DKBuffer> DKBuffer::DecodeDecompress(const DKStringU8& s, DKAllocator& alloc)
{
	DKObject<DKBuffer> d = DKBuffer::Decode(s, alloc);
	if (d)
		return d->Decompress(alloc);
	return NULL;
}

DKObject<DKBuffer> DKBuffer::DecodeDecompress(const DKStringW& s, DKAllocator& alloc)
{
	DKObject<DKBuffer> d = DKBuffer::Decode(s, alloc);
	if (d)
		return d->Decompress(alloc);
	return NULL;
}

DKObject<DKBuffer> DKBuffer::Create(const DKString& url, DKAllocator& alloc)
{
	DKString filename;

	if (url.Left(7).CompareNoCase(L"http://") == 0)
		return Private::GetHTTPContent(url);
	else if (url.Left(6).CompareNoCase(L"ftp://") == 0)
		return Private::GetFTPContent(url);
	else if (url.Left(7).CompareNoCase(L"file://") == 0)
	{
		filename = url.Right(7);
		if (filename.Length() < 1)
			return NULL;
#ifdef _WIN32
		{
			if ((filename[0] == L'\\' || filename[0] == L'/') &&
				filename[2] == L':' &&
				(filename[3] == L'\\' || filename[3] == L'/'))
				filename = filename.Right(1);
		}
#endif
	}
	else
	{
		filename = url;
	}

	if (filename.Length() > 0)
	{
		DKObject<DKFile> f = DKFile::Create(filename, DKFile::ModeOpenReadOnly, DKFile::ModeShareAll);
		DKFile::FileInfo info;
		if (f && f->GetInfo(info) && info.size > 0)
		{
			return f->Read(info.size, alloc);
		}
	}
	return NULL;
}

DKObject<DKBuffer> DKBuffer::Create(const void* p, size_t s, DKAllocator& alloc)
{
	DKObject<DKBuffer> data = DKObject<DKBuffer>::Alloc(alloc);
	data->SetContent(p, s);
	return data;
}

DKObject<DKBuffer> DKBuffer::Create(const DKData* p, DKAllocator& alloc)
{
	DKObject<DKBuffer> data = DKObject<DKBuffer>::Alloc(alloc);
	data->SetContent(p);
	return data;
}

DKObject<DKBuffer> DKBuffer::Create(DKStream* s, DKAllocator& alloc)
{
	if (s == NULL || !s->IsReadable())
		return NULL;

	DKStream::Position len = s->RemainLength();
	if (len > 0)
	{
		DKObject<DKBuffer> p = DKBuffer::Create(NULL, len, alloc);
		if (p)
		{
			void* buff = p->LockExclusive();
			size_t bytesRead = s->Read(buff, len);
			p->UnlockExclusive();

			if (bytesRead == len)
				return p;
		}
	}
	return NULL;
}

size_t DKBuffer::SetContent(const void* p, size_t s)
{
	DKCriticalSection<DKSharedLock> guard(this->sharedLock);

	if (contentLength == s)
	{
		if (s > 0 && p)
		{
			memcpy(contentPtr, p, s);
		}
		else if (s > 0)
		{
			memset(contentPtr, 0, s);
		}
	}
	else
	{
		if (s > 0)
		{
			void* ptr = allocator->Alloc(s);

			if (p)
				memcpy(ptr, p, s);
			else
				memset(ptr, 0, s);

			if (contentPtr)
				allocator->Dealloc(contentPtr);

			this->contentPtr = ptr;
			this->contentLength = s;
		}
		else
		{
			if (contentPtr)
				allocator->Dealloc(contentPtr);

			contentPtr = NULL;
			contentLength = 0;
		}
	}
	return contentLength;
}

size_t DKBuffer::SetContent(const DKData* buff)
{
	size_t ret = 0;
	if (buff)
	{
		const void* p = buff->LockShared();
		ret = SetContent(p, buff->Length());
		buff->UnlockShared();
	}
	else
	{
		ret = SetContent(0, 0);
	}
	return ret;
}

DKBuffer& DKBuffer::operator = (const DKBuffer& b)
{
	SetContent(&b);
	return *this;
}

DKBuffer& DKBuffer::operator = (DKBuffer&& b)
{
	if (this != &b)
	{
		DKCriticalSection<DKSharedLock> guard(this->sharedLock);

		if (this->contentPtr)
			this->allocator->Dealloc(this->contentPtr);

		this->contentPtr = b.contentPtr;
		this->contentLength = b.contentLength;
		this->allocator = b.allocator;
		b.contentPtr = NULL;
		b.contentLength = 0;
	}
	return *this;
}

size_t DKBuffer::CopyContent(void* p, size_t offset, size_t length) const
{
	const char* ptr = reinterpret_cast<const char*>(this->LockShared());
	size_t contentLength = this->Length();
	size_t ret = 0;
	if (p && offset < contentLength)
	{
		length = Min<size_t>(length, contentLength - offset);
		memcpy(p, &ptr[offset], length);
		ret = length;
	}
	this->UnlockShared();
	return ret;
}

void DKBuffer::SwitchAllocator(DKAllocator& alloc)
{
	if (&alloc != this->allocator)
	{
		DKCriticalSection<DKSharedLock> guard(this->sharedLock);

		if (contentLength > 0)
		{
			void* p = alloc.Alloc(contentLength);
			memcpy(p, contentPtr, contentLength);

			this->contentPtr = p;
			this->allocator->Dealloc(contentPtr);
		}
		this->allocator = &alloc;
	}
}

DKAllocator& DKBuffer::Allocator(void) const
{
	return *this->allocator;
}

void* DKBuffer::LockContent(void)
{
	return contentPtr;
}

void DKBuffer::UnlockContent(void)
{
}
