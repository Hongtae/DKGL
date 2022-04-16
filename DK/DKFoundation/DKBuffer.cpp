//
//  File: DKBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
//

#include <ctype.h>
#include <wctype.h>

#define LIBXML_STATIC
#include "../Libs/libxml2/include/libxml/nanohttp.h"
#include "../Libs/libxml2/include/libxml/nanoftp.h"

#include "DKString.h"
#include "DKFile.h"
#include "DKBuffer.h"
#include "DKMemory.h"
#include "DKArray.h"
#include "DKBufferStream.h"
#include "DKLog.h"

namespace DKFoundation::Private
{
    // base64 encode/decode
    template <typename BaseCharT> struct Base64;
    template <> struct Base64<char>
    {
        static const char* Base64Chars()
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
        static const wchar_t* Base64Chars()
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
            output->SetContents((const char*)result, result.Count());
            return true;
        }
        return false;
    }

    static DKObject<DKBuffer> GetHTTPContents(const DKString& url, DKAllocator& alloc)
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

    static DKObject<DKBuffer> GetFTPContents(const DKString& url, DKAllocator& alloc)
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
        char* buffer = (char*)DKMalloc(bufferSize);

        while (true)
        {
            int recv = xmlNanoFTPRead(ctxt, buffer + received, static_cast<int>(bufferSize - received));
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
using namespace DKFoundation;

DKBuffer::DKBuffer(DKAllocator& alloc)
	: contentsPtr(nullptr)
	, contentsLength(0)
	, allocator(&alloc)
{
}

DKBuffer::DKBuffer(const void* p, size_t s, DKAllocator& alloc)
	: contentsPtr(nullptr)
	, contentsLength(0)
	, allocator(&alloc)
{
	SetContents(p, s);
}

DKBuffer::DKBuffer(const DKData* buff, DKAllocator& alloc)
	: contentsPtr(nullptr)
	, contentsLength(0)
	, allocator(&alloc)
{
	SetContents(buff);
}

DKBuffer::DKBuffer(const DKBuffer& b)
	: contentsPtr(NULL)
	, contentsLength(0)
	, allocator(b.allocator)
{
	SetContents(b.contentsPtr, b.contentsLength);
}

DKBuffer::DKBuffer(DKBuffer&& b)
	: contentsPtr(NULL)
	, contentsLength(0)
	, allocator(b.allocator)
{
	this->contentsPtr = b.contentsPtr;
	this->contentsLength = b.contentsLength;
	b.contentsPtr = NULL;
	b.contentsLength = 0;
}

DKBuffer::~DKBuffer()
{
	if (contentsPtr)
		allocator->Dealloc(contentsPtr);
}

size_t DKBuffer::Length() const
{
	return contentsLength;
}

bool DKBuffer::SetLength(size_t len)
{
	if (contentsLength != len)
	{
		if (len > 0)
		{
			void* p = this->allocator->Realloc(contentsPtr, len);
			if (p)
			{
				contentsPtr = p;
				contentsLength = len;
			}
			else
			{
				DKLog("DKBuffer error: Out of memory!");
				return false;
			}
		}
		else
		{
			if (contentsPtr)
				allocator->Dealloc(contentsPtr);
			contentsPtr = NULL;
			contentsLength = 0;
		}
	}
	return true;
}

bool DKBuffer::Base64Encode(DKStringU8& strOut) const
{
	bool ret = false;
	const void* p = this->Contents();
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
	return ret;
}

bool DKBuffer::Base64Encode(DKStringW& strOut) const
{
	bool ret = false;
	const void* p = this->Contents();
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
	const void* p = this->Contents();
	size_t inputLength = this->Length();
	DKObject<DKBuffer> result = Compress(compressor, p, inputLength, alloc);
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
	const void* p = this->Contents();
	size_t len = this->Length();
	DKObject<DKBuffer> result = Decompress(p, len, alloc);
	return result;
}

DKObject<DKBuffer> DKBuffer::Decompress(const void* p, size_t len, DKAllocator& alloc)
{
	if (p && len > 4)
	{
		DKObject<DKBuffer> output = DKBuffer::Create(0, 0, alloc);
		DKBufferStream outputStream(output);
		DKDataStream inputStream(DKData::StaticData(p, len));

		if (DKCompressor().Decompress(&inputStream, &outputStream))
			return outputStream.Buffer();
	}
	return NULL;
}

DKObject<DKBuffer> DKBuffer::Create(const DKString& url, DKAllocator& alloc)
{
	DKString filename;

	if (url.Left(7).CompareNoCase(L"http://") == 0)
		return Private::GetHTTPContents(url, alloc);
	else if (url.Left(6).CompareNoCase(L"ftp://") == 0)
		return Private::GetFTPContents(url, alloc);
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
	data->SetContents(p, s);
	return data;
}

DKObject<DKBuffer> DKBuffer::Create(const DKData* p, DKAllocator& alloc)
{
	DKObject<DKBuffer> data = DKObject<DKBuffer>::Alloc(alloc);
	data->SetContents(p);
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
			void* buff = p->MutableContents();
			size_t bytesRead = s->Read(buff, len);

			if (bytesRead == len)
				return p;
		}
	}
	return NULL;
}

size_t DKBuffer::SetContents(const void* p, size_t s)
{
	if (contentsLength == s)
	{
		if (s > 0 && p)
		{
			memcpy(contentsPtr, p, s);
		}
		else if (s > 0)
		{
			memset(contentsPtr, 0, s);
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

			if (contentsPtr)
				allocator->Dealloc(contentsPtr);

			this->contentsPtr = ptr;
			this->contentsLength = s;
		}
		else
		{
			if (contentsPtr)
				allocator->Dealloc(contentsPtr);

			contentsPtr = NULL;
			contentsLength = 0;
		}
	}
	return contentsLength;
}

size_t DKBuffer::SetContents(const DKData* buff)
{
	size_t ret = 0;
	if (buff)
	{
		const void* p = buff->Contents();
		ret = SetContents(p, buff->Length());
	}
	else
	{
		ret = SetContents(0, 0);
	}
	return ret;
}

DKBuffer& DKBuffer::operator = (const DKBuffer& b)
{
	SetContents(&b);
	return *this;
}

DKBuffer& DKBuffer::operator = (DKBuffer&& b)
{
	if (this != &b)
	{
		if (this->contentsPtr)
			this->allocator->Dealloc(this->contentsPtr);

		this->contentsPtr = b.contentsPtr;
		this->contentsLength = b.contentsLength;
		this->allocator = b.allocator;
		b.contentsPtr = nullptr;
		b.contentsLength = 0;
	}
	return *this;
}

size_t DKBuffer::CopyContents(void* p, size_t offset, size_t length) const
{
	const char* ptr = reinterpret_cast<const char*>(this->Contents());
	size_t contentLength = this->Length();
	size_t ret = 0;
	if (p && offset < contentLength)
	{
		length = Min(length, contentLength - offset);
		memcpy(p, &ptr[offset], length);
		ret = length;
	}
	return ret;
}

void DKBuffer::SwitchAllocator(DKAllocator& alloc)
{
	if (&alloc != this->allocator)
	{
		if (contentsLength > 0)
		{
			void* p = alloc.Alloc(contentsLength);
			memcpy(p, contentsPtr, contentsLength);

			this->contentsPtr = p;
			this->allocator->Dealloc(contentsPtr);
		}
		this->allocator = &alloc;
	}
}

DKAllocator& DKBuffer::Allocator() const
{
	return *this->allocator;
}

const void* DKBuffer::Contents() const
{
	return contentsPtr;
}

void* DKBuffer::MutableContents()
{
	return contentsPtr;
}
