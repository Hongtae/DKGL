//
//  File: DKBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <ctype.h>
#include <wctype.h>

#include "../Libs/zlib/zlib.h"
#include "../Libs/zlib/contrib/minizip/zip.h"
#include "../Libs/zlib/contrib/minizip/unzip.h"

#include "../Libs/lz4/lib/lz4.h"
#include "../Libs/lz4/lib/lz4hc.h"
#include "../Libs/lz4/lib/lz4frame.h"
#include "../Libs/lz4/lib/xxhash.h"

#define LIBXML_STATIC
#include "../Libs/libxml2/include/libxml/nanohttp.h"
#include "../Libs/libxml2/include/libxml/nanoftp.h"

#include "DKEndianness.h"
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
		// base64 encode/decode
		template <typename BaseCharT> struct Base64;
		template <> struct Base64<char>
		{
			static const char* Base64Chars(void)
			{
				return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			}
			static FORCEINLINE bool IsBase64(char c)
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
			static FORCEINLINE bool IsBase64(wchar_t c)
			{
				return ((c == L'+') || (c == '/') || iswalnum(c));
			}
		};

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

					for (i = 0; (i < 4); i++)
						output.Add(Base64<BaseCharT>::Base64Chars()[char_array_4[i]]);
					i = 0;
				}
			}

			if (i)
			{
				for (j = i; j < 3; j++)
					char_array_3[j] = '\0';

				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (j = 0; (j < i + 1); j++)
					output.Add(Base64<BaseCharT>::Base64Chars()[char_array_4[j]]);

				while ((i++ < 3))
					output.Add('=');
			}
			//return output;
		}

		template <typename BaseCharT> bool Base64Decode(const BaseCharT* encoded_string, size_t in_len, DKBuffer* output)
		{
			auto IndexOfBase64Char = [](BaseCharT c) -> int
			{
				for (int i = 0; i < 64; ++i)
					if (c == Base64<BaseCharT>::Base64Chars()[i])
						return i;
				return -1;
			};

			int i = 0;
			int j = 0;
			int in_ = 0;
			char char_array_4[4], char_array_3[3];
			DKArray<char> result;
			result.Reserve((in_len * 3) / 4 + 3);

			while (in_len-- && (encoded_string[in_] != '=') && Base64<BaseCharT>::IsBase64(encoded_string[in_]))
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

		static DKObject<DKBuffer> GetHTTPContent(const DKString& url, DKAllocator& alloc)
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
				void* buffer = DKMalloc(len);
				int recv = xmlNanoHTTPRead(ctxt, buffer, len); // download
				if (recv == len)
				{
					data = DKBuffer::Create(buffer, len, alloc);
				}
				DKFree(buffer);
			}
			xmlNanoHTTPClose(ctxt);
			xmlNanoHTTPCleanup();

			return data;
		}

		static DKObject<DKBuffer> GetFTPContent(const DKString& url, DKAllocator& alloc)
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
			char*	buffer = (char*)DKMalloc(bufferSize);

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
					data = DKBuffer::Create(buffer, received, alloc);
					break;
				}
				if (received + 100 > bufferSize)
				{
					bufferSize += 4096;
					char* tmp = (char*)DKRealloc(buffer, bufferSize);
					if (tmp == NULL)
					{
						DKLog("Warning: Out of memory!\n");
						break;
					}
					buffer = tmp;
				}
			}
			DKFree(buffer);

			xmlNanoFTPClose(ctxt);
			xmlNanoFTPCleanup();

			return data;
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
#ifdef DKGL_DEBUG_ENABLED
	if (sharedLock.TryLock())
	{
		sharedLock.Unlock();
	}
	else
	{
		DKERROR_THROW("Data Locked!");
	}
#endif

	if (contentPtr)
		allocator->Dealloc(contentPtr);
}

size_t DKBuffer::Length(void) const
{
	return contentLength;
}

bool DKBuffer::SetLength(size_t len)
{
	if (contentLength != len)
	{
		DKCriticalSection<DKSharedLock> guard(this->sharedLock);
		DKBuffer buff;
		buff.SetContent(0, len);
		if (buff.contentPtr)
		{
			void* p = buff.contentPtr;
			size_t n = buff.contentLength;
			buff.contentPtr = this->contentPtr;
			buff.contentLength = this->contentLength;
			this->contentPtr = p;
			this->contentLength = n;
			memcpy(this->contentPtr, buff.contentPtr, Min(this->contentLength, buff.contentLength));
			return true;
		}
		return false;
	}
	return true;
}

bool DKBuffer::Base64Encode(DKStringU8& strOut) const
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

bool DKBuffer::Base64Encode(DKStringW& strOut) const
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

DKObject<DKBuffer> DKBuffer::Base64Decode(const DKStringU8& str, DKAllocator& alloc)
{
	DKObject<DKBuffer> buff = DKOBJECT_NEW DKBuffer(alloc);
	if (Private::Base64Decode((const char*)str, str.Bytes(), buff))
		return buff;
	return NULL;
}

DKObject<DKBuffer> DKBuffer::Base64Decode(const DKStringW& str, DKAllocator& alloc)
{
	DKObject<DKBuffer> buff = DKOBJECT_NEW DKBuffer(alloc);
	if (Private::Base64Decode((const wchar_t*)str, str.Length(), buff))
		return buff;
	return NULL;
}

DKObject<DKBuffer> DKBuffer::Compress(const DKCompressor& compressor, DKAllocator& alloc) const
{
	const void* p = this->LockShared();
	size_t inputLength = this->Length();
	DKObject<DKBuffer> result = Compress(compressor, p, inputLength, alloc);
	this->UnlockShared();
	return result;
}

DKObject<DKBuffer> DKBuffer::Compress(const DKCompressor& compressor, const void* p, size_t len, DKAllocator& alloc)
{
	if (p && len > 0)
	{
		DKObject<DKBuffer> output = DKBuffer::Create(0, 0, alloc);
		DKBufferStream outputStream(output);
		DKDataStream inputStream(DKData::StaticData(p, len));

		if (compressor.Compress(&inputStream, &outputStream))
			return outputStream.Buffer();
	}
	return NULL;
}


DKObject<DKBuffer> DKBuffer::Decompress(DKAllocator& alloc) const
{
	const void* p = this->LockShared();
	size_t len = this->Length();
	DKObject<DKBuffer> result = Decompress(p, len, alloc);
	this->UnlockShared();
	return result;
}

DKObject<DKBuffer> DKBuffer::Decompress(const void* p, size_t len, DKAllocator& alloc)
{
	if (p && len > 4)
	{
		DKObject<DKBuffer> output = DKBuffer::Create(0, 0, alloc);
		DKBufferStream outputStream(output);
		DKDataStream inputStream(DKData::StaticData(p, len));

		if (DKCompressor::Decompress(&inputStream, &outputStream))
			return outputStream.Buffer();
	}
	return NULL;
}

DKObject<DKBuffer> DKBuffer::Create(const DKString& url, DKAllocator& alloc)
{
	DKString filename;

	if (url.Left(7).CompareNoCase(L"http://") == 0)
		return Private::GetHTTPContent(url, alloc);
	else if (url.Left(6).CompareNoCase(L"ftp://") == 0)
		return Private::GetFTPContent(url, alloc);
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
		length = Min(length, contentLength - offset);
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

const void* DKBuffer::LockShared(void) const
{
	sharedLock.LockShared();
	return contentPtr;
}

bool DKBuffer::TryLockShared(const void ** ptr) const
{
	if (sharedLock.TryLockShared())
	{
		if (ptr)
			*ptr = contentPtr;
		return true;
	}
	return false;
}

void DKBuffer::UnlockShared(void) const
{
	sharedLock.UnlockShared();
}

void* DKBuffer::LockExclusive(void)
{
	sharedLock.Lock();
	return contentPtr;
}

bool DKBuffer::TryLockExclusive(void ** ptr)
{
	if (sharedLock.TryLock())
	{
		if (ptr)
			*ptr = contentPtr;
		return true;
	}
	return false;
}

void DKBuffer::UnlockExclusive(void)
{
	sharedLock.Unlock();
}
