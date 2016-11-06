//
//  File: DKBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <ctype.h>
#include <wctype.h>

#include "../Libs/Inc_zlib.h"
#include "../Libs/Inc_lz4.h"
#include "../Libs/Inc_libxml2.h"

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

namespace DKGL
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
				void* buffer = DKMemoryDefaultAllocator::Alloc(len);
				int recv = xmlNanoHTTPRead(ctxt, buffer, len); // download
				if (recv == len)
				{
					data = DKBuffer::Create(buffer, len, alloc);
				}
				DKMemoryDefaultAllocator::Free(buffer);
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
			char*	buffer = (char*)DKMemoryDefaultAllocator::Alloc(bufferSize);

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
					char* tmp = (char*)DKMemoryDefaultAllocator::Realloc(buffer, bufferSize);
					if (tmp == NULL)
					{
						DKLog("Warning: Out of memory!\n");
						break;
					}
					buffer = tmp;
				}
			}
			DKMemoryDefaultAllocator::Free(buffer);

			xmlNanoFTPClose(ctxt);
			xmlNanoFTPCleanup();

			return data;
		}
	}
}

using namespace DKGL;

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

DKObject<DKBuffer> DKBuffer::Compress(DKCompressor compressor, DKAllocator& alloc) const
{
	const void* p = this->LockShared();
	size_t inputLength = this->Length();
	DKObject<DKBuffer> result = Compress(p, inputLength, compressor, alloc);
	this->UnlockShared();
	return result;
}

DKObject<DKBuffer> DKBuffer::Compress(const void* p, size_t len, DKCompressor compressor, DKAllocator& alloc)
{
	DKObject<DKBuffer> result = NULL;
	if (p && len > 0)
	{
		if (compressor == DKCompressor::LZ4 || compressor == DKCompressor::LZ4HC)
		{
			LZ4F_preferences_t prefs;
			memset(&prefs, 0, sizeof(prefs));
			prefs.autoFlush = 1;
			prefs.compressionLevel = compressor == DKCompressor::LZ4 ? 0 : 9;	// 0 for LZ4 fast, 9 for LZ4HC
			prefs.frameInfo.blockMode = LZ4F_blockLinked;	// for better compression ratio.
			prefs.frameInfo.contentChecksumFlag = LZ4F_contentChecksumEnabled; // to detect data corruption.
			prefs.frameInfo.contentSize = len;

			size_t bufferSize = LZ4F_compressFrameBound(len, &prefs);
			void* compressed = DKMemoryDefaultAllocator::Alloc(bufferSize);
			size_t compressedSize = LZ4F_compressFrame(compressed, bufferSize, p, len, &prefs);
			if (LZ4F_isError(compressedSize))
			{
				DKLog("Compression failed : %s", LZ4F_getErrorName(compressedSize));
			}
			else
			{
				result = DKBuffer::Create(compressed, compressedSize, alloc);
			}
			DKMemoryDefaultAllocator::Free(compressed);
		}
		else if (compressor == DKCompressor::Deflate)
		{
			int compressLevel = 9;	// Z_DEFAULT_COMPRESSION is 6
			uLongf compressedSize = len + (len / 10) + 100;
			void* compressed = DKMemoryDefaultAllocator::Alloc(compressedSize);
			if (compress2((Bytef*)compressed, &compressedSize, (const Bytef*)p, len, compressLevel) == Z_OK)
			{
				result = DKBuffer::Create(compressed, compressedSize, alloc);
			}
			DKMemoryDefaultAllocator::Free(compressed);
		}
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

//#define COMPRESS_DEFAULT_BLOCK		0x4000
#define COMPRESS_DEFAULT_BLOCK		0x10000

DKObject<DKBuffer> DKBuffer::Decompress(const void* p, size_t len, DKAllocator& alloc)
{
	DKObject<DKBuffer> result = NULL;

	if (p && len > 4)
	{
		const uint32_t lz4_Header = DKSystemToLittleEndian(0x184D2204U);
		const uint32_t lz4_SkipHeader = DKSystemToLittleEndian(0x184D2A50U);

		// LZ7 header: 0x184D2204 (little endian) or 0x184D2250~F for skippable frame
		if (reinterpret_cast<const uint32_t*>(p)[0] == lz4_Header ||
			(reinterpret_cast<const uint32_t*>(p)[0] & 0xfffffff0U) == lz4_SkipHeader)
		{
			// LZ7 / LZ7HC
			LZ4F_decompressionContext_t ctxt;
			LZ4F_errorCode_t errorCode = LZ4F_createDecompressionContext(&ctxt, LZ4F_VERSION);

			if (LZ4F_isError(errorCode))
			{
				DKLog("Decompress Error: can't create Lz4 context : %s", LZ4F_getErrorName(errorCode));
			}
			else
			{
				uint8_t* outData = (uint8_t*)DKMemoryDefaultAllocator::Alloc(COMPRESS_DEFAULT_BLOCK);

				uint32_t header;
				size_t decoded = 0;
				size_t processed = 0;
				size_t inSize = len;
				const uint8_t* inData = reinterpret_cast<const uint8_t*>(p);
				size_t outSize = COMPRESS_DEFAULT_BLOCK;
				bool decodeError = false;
				LZ4F_errorCode_t nextToLoad;

				while (processed + sizeof(uint32_t) < len && !decodeError)
				{
					header = reinterpret_cast<const uint32_t*>(&inData[processed])[0];

					if (header == lz4_Header)
					{
						do {
							inSize = (len - processed);
							outSize = COMPRESS_DEFAULT_BLOCK;
							nextToLoad = LZ4F_decompress(ctxt, &outData[decoded], &outSize, &inData[processed], &inSize, NULL);
							if (LZ4F_isError(nextToLoad))
							{
								DKLog("Decompress Error: Lz4 Header Error: %s\n", LZ4F_getErrorName(nextToLoad));
								decodeError = true;
								break;
							}
							processed += inSize;
							if (outSize > 0)
							{
								decoded += outSize;
								uint8_t* tmp = (uint8_t*)DKMemoryDefaultAllocator::Realloc(outData, decoded + COMPRESS_DEFAULT_BLOCK);
								if (tmp == NULL)
								{
									DKLog("Decompress Error: Out of memory!");
									decodeError = true;
									break;
								}
								outData = tmp;
							}
						} while (nextToLoad);
					}
					else if ((header & 0xfffffff0U) == lz4_SkipHeader)
					{
						uint32_t bytesToSkip = reinterpret_cast<const uint32_t*>(&inData[processed])[0];
						bytesToSkip = DKLittleEndianToSystem(bytesToSkip);
						size_t remains = len - processed;
						if (bytesToSkip > remains)	// overflow!
						{
							DKLog("Decompress Error: Lz4 skip frame overflow!\n");
							decodeError = true;
							break;
						}
						processed += bytesToSkip;
					}
					else
					{
						DKLog("Decompress Error: Lz4 stream followed by unrecognized data\n");
						decodeError = true;
						break;
					}
				}

				if (!decodeError && decoded > 0)
				{
					result = DKBuffer::Create(outData, decoded, alloc);
				}

				DKMemoryDefaultAllocator::Free(outData);

				errorCode = LZ4F_freeDecompressionContext(ctxt);
				if (LZ4F_isError(errorCode))
				{
					DKLog("Decompress Error: can't free LZ4F context resource : %s", LZ4F_getErrorName(errorCode));
				}
			}
		}
		else if (reinterpret_cast<const char*>(p)[0] == 0x78)
		{
			// zlib
			uLongf outputLength = COMPRESS_DEFAULT_BLOCK;

			Bytef* inputSource = (Bytef*)p;
			Bytef* output = (Bytef*)DKMemoryDefaultAllocator::Alloc(outputLength);
			uLongf inputLength = len;

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
						Bytef* tmp = (Bytef*)DKMemoryDefaultAllocator::Realloc(output, stream.total_out + COMPRESS_DEFAULT_BLOCK);
						if (tmp == NULL)
						{
							DKLog("Decompress Error: OUT OF MEMORY!");
							break;
						}
						output = tmp;

						stream.avail_out += COMPRESS_DEFAULT_BLOCK;
						stream.next_out = output + stream.total_out;
					}
				}
				inflateEnd(&stream);
			}
			DKMemoryDefaultAllocator::Free(output);
		}
	}
	return result;
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
