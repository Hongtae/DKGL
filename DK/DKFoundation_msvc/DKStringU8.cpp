//
//  File: DKStringU8.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "DKString.h"
#include "DKBuffer.h"

namespace DKFoundation
{
	namespace Private
	{
		namespace
		{
			inline DKUniCharW LowercaseChar(DKUniChar8 c)
			{
				if (c >= 'A' && c <= 'Z')
					c = (c - 'A') + 'a';
				return c;
			}

			inline int CompareCaseInsensitive(const DKUniChar8* a, const DKUniChar8* b)
			{
				if (a == b)
					return 0;
				if (a == NULL)
					a = "";
				if (b == NULL)
					b = "";

				const DKUniChar8 *p = a;
				const DKUniChar8 *q = b;
				int cmp = 0;
				for ( ; *p || *q; ++p, ++q)
				{
					cmp = LowercaseChar(*p) - LowercaseChar(*q);
					if (cmp != 0)
						return cmp;
				}
				return LowercaseChar(*p) - LowercaseChar(*q);
			}

			inline int CompareCaseSensitive(const DKUniChar8* a, const DKUniChar8* b)
			{
				if (a == b)
					return 0;
				if (a == NULL)
					a = "";
				if (b == NULL)
					b = "";

				const DKUniChar8 *p = a;
				const DKUniChar8 *q = b;
				int cmp = 0;
				for ( ; *p || *q; ++p, ++q)
				{
					cmp = (*p) - (*q);
					if (cmp != 0)
						return cmp;
				}
				return (*p) - (*q);
			}
		}

		bool IsLegalUTF8String(const DKUniChar8* input, size_t inputLen);
		size_t NumberOfCharactersInUTF8(const DKUniChar8* input, size_t length);
	}
}
using namespace DKFoundation;

const DKStringU8& DKStringU8::EmptyString()
{
	static DKStringU8 s = "";
	return s;
}

DKStringEncoding DKStringU8::SystemEncoding(void)
{
	return DKStringU8Encoding();
}

DKStringU8::DKStringU8(void)
	: stringData(NULL)
{
}

DKStringU8::DKStringU8(DKStringU8&& str)
	: stringData(NULL)
{
	stringData = str.stringData;
	str.stringData = NULL;
}

DKStringU8::DKStringU8(const DKStringU8& str)
	: stringData(NULL)
{
	this->SetValue(str);
}

DKStringU8::DKStringU8(const DKUniChar8* str, size_t len)
	: stringData(NULL)
{
	this->SetValue(str, len);
}

DKStringU8::DKStringU8(const DKUniCharW* str, size_t len)
	: stringData(NULL)
{
	this->SetValue(str, len);
}

DKStringU8::DKStringU8(const void* str, size_t bytes, DKStringEncoding e)
	: stringData(NULL)
{
	this->SetValue(str, bytes, e);
}

DKStringU8::DKStringU8(DKUniCharW c)
	: stringData(NULL)
{
	this->SetValue(&c, 1);
}

DKStringU8::DKStringU8(DKUniChar8 c)
	: stringData(NULL)
{
	this->SetValue(&c, 1);
}

DKStringU8::~DKStringU8(void)
{
	if (stringData)
		DKMemoryHeapFree(stringData);
}

DKStringU8 DKStringU8::Format(const DKUniChar8* fmt, ...)
{
	DKStringU8 ret = "";
	if (fmt && fmt[0])
	{
		va_list ap;
		va_start(ap, fmt);
		ret = FormatV(fmt, ap);
		va_end(ap);
	}
	return ret;
}

DKStringU8 DKStringU8::Format(const DKUniCharW* fmt, ...)
{
	DKStringU8 ret = "";
	if (fmt && fmt[0])
	{
		va_list ap;
		va_start(ap, fmt);
		ret = FormatV(fmt, ap);
		va_end(ap);
	}
	return ret;
}

DKStringU8 DKStringU8::FormatV(const DKUniChar8* fmt, va_list v)
{
	DKStringU8 ret = "";
	if (fmt && fmt[0])
	{
#ifdef __GNUC__
		va_list v2;
		va_copy(v2, v);
		DKStringFormatV(ret, fmt, v);
		va_end(v2);
#else
		DKStringFormatV(ret, fmt, v);
#endif
	}
	return ret;
}

DKStringU8 DKStringU8::FormatV(const DKUniCharW* fmt, va_list v)
{
	DKStringU8 ret = "";
	if (fmt && fmt[0])
	{
#ifdef __GNUC__
		va_list v2;
		va_copy(v2, v);
		DKStringFormatV(ret, fmt, v);
		va_end(v2);
#else
		DKStringFormatV(ret, fmt, v);
#endif
	}
	return ret;
}

DKStringU8& DKStringU8::Append(const DKStringU8& str)
{
	return Append((const DKUniChar8*)str);
}

DKStringU8& DKStringU8::Append(const DKUniChar8* str, size_t len)
{
	if (str && str[0])
	{
		size_t len1 = this->Length();
		size_t len2 = 0;
		for (len2 = 0; str[len2] && len2 < len; len2++) {}

		size_t totalLen = len1 + len2;
		if (totalLen > 0)
		{
			DKUniChar8* buff = (DKUniChar8*)DKMemoryHeapAlloc(totalLen + 1);
			DKUniChar8* p = buff;
			if (len1 > 0)
			{
				memcpy(p, stringData, len1);
				p += len1;
			}
			if (len2 > 0)
			{
				memcpy(p, str, len2);
				p += len2;
			}
			*p = 0;

			if (stringData)
				DKMemoryHeapFree(stringData);
			stringData = buff;
		}
	}
	return *this;
}

DKStringU8& DKStringU8::Append(const DKUniCharW* str, size_t len)
{
	DKStringU8 s = "";
	DKStringSetValue(s, str, len);
	return this->Append(s);
}

DKStringU8& DKStringU8::Append(const void* str, size_t bytes, DKStringEncoding e)
{
	DKStringU8 s = "";
	DKStringSetValue(s, str, bytes, e);
	return this->Append(s);
}

DKStringU8& DKStringU8::SetValue(const DKStringU8& str)
{
	if (str.stringData == this->stringData)
		return *this;
	
	if (this->stringData)
		DKMemoryHeapFree(this->stringData);
	this->stringData = NULL;
	
	size_t len = str.Length();
	if (len > 0)
	{
		DKASSERT_DEBUG(str.stringData != NULL);
		this->stringData = (DKUniChar8*)DKMemoryHeapAlloc(len+1);
		memcpy(this->stringData, str.stringData, len);
		this->stringData[len] = 0;
	}
	return *this;
}

DKStringU8& DKStringU8::SetValue(const DKUniChar8* str, size_t len)
{
	if (str == stringData && len >= this->Length())
	{
		return *this;
	}
	
	DKUniChar8* buff = NULL;
	if (str && str[0])
	{
		for (size_t i = 0; i < len; ++i)
		{
			if (str[i] == 0)
			{
				len = i;
				break;
			}
		}
		
		if (len > 0)
		{
			buff = (DKUniChar8*)DKMemoryHeapAlloc(len+1);
			memcpy(buff, str, len);
			buff[len] = NULL;
		}
	}
	if (stringData)
		DKMemoryHeapFree(stringData);
	stringData = buff;
	
	return *this;
}

DKStringU8& DKStringU8::SetValue(const DKUniCharW* str, size_t len)
{
	if (str && str[0])
	{
		DKStringSetValue(*this, str, len);
	}
	else
	{
		if (stringData)
			DKMemoryHeapFree(stringData);
		stringData = NULL;
	}
	return *this;
}

DKStringU8& DKStringU8::SetValue(const void* str, size_t bytes, DKStringEncoding e)
{
	DKStringSetValue(*this, str, bytes, e);
	return *this;
}

DKObject<DKData> DKStringU8::Encode(DKStringEncoding e) const
{
	DKObject<DKBuffer> data = DKObject<DKBuffer>::New();
	DKStringEncode(data, *this, e);
	return data.SafeCast<DKData>();
}

size_t DKStringU8::Length(void) const
{
	return Private::NumberOfCharactersInUTF8(stringData, Bytes());
}

size_t DKStringU8::Bytes(void) const
{
	size_t len = 0;
	if (stringData)
	{
		while (stringData[len])
			len++;
	}
	return len;
}

int DKStringU8::Compare(const DKUniChar8* str) const
{
	return Private::CompareCaseSensitive(stringData, str);
}

int DKStringU8::Compare(const DKStringU8& str) const
{
	return Private::CompareCaseSensitive(stringData, str.stringData);
}

int DKStringU8::CompareNoCase(const DKUniChar8* str) const
{
	return Private::CompareCaseInsensitive(stringData, str);
}

int DKStringU8::CompareNoCase(const DKStringU8& str) const
{
	return Private::CompareCaseInsensitive(stringData, str.stringData);
}

// assignment operators
DKStringU8& DKStringU8::operator = (DKStringU8&& str)
{
	if (this != &str)
	{
		if (stringData)
			DKMemoryHeapFree(stringData);

		stringData = str.stringData;
		str.stringData = NULL;
	}
	return *this;
}

DKStringU8& DKStringU8::operator = (const DKStringU8& str)
{
	if (this != &str)
		return this->SetValue(str);
	return *this;
}

DKStringU8& DKStringU8::operator = (const DKUniCharW* str)
{
	return this->SetValue(str);
}

DKStringU8& DKStringU8::operator = (const DKUniChar8* str)
{
	return this->SetValue(str);
}

DKStringU8& DKStringU8::operator = (DKUniCharW ch)
{
	return this->SetValue(&ch, 1);
}

DKStringU8& DKStringU8::operator = (DKUniChar8 ch)
{
	return this->SetValue(&ch, 1);
}

// conversion operators
DKStringU8::operator const DKUniChar8* (void) const
{
	if (stringData)
		return stringData;
	return "";
}

// concatention operators
DKStringU8& DKStringU8::operator += (const DKStringU8& str)
{
	return Append(str);
}

DKStringU8& DKStringU8::operator += (const DKUniCharW* str)
{
	return Append(str);
}

DKStringU8& DKStringU8::operator += (const DKUniChar8* str)
{
	return Append(str);
}

DKStringU8& DKStringU8::operator += (DKUniCharW ch)
{
	DKUniCharW str[2] = {ch, 0};
	return Append(str);
}

DKStringU8& DKStringU8::operator += (DKUniChar8 ch)
{
	DKUniChar8 str[2] = {ch, 0};
	return Append(str);
}

DKStringU8 DKStringU8::operator + (const DKStringU8& str) const
{
	return DKStringU8(*this).Append(str);
}

DKStringU8 DKStringU8::operator + (const DKUniCharW* str) const
{
	return DKStringU8(*this).Append(str);
}

DKStringU8 DKStringU8::operator + (const DKUniChar8* str) const
{
	return DKStringU8(*this).Append(str);
}

DKStringU8 DKStringU8::operator + (DKUniCharW c) const
{
	return DKStringU8(*this).Append(&c, 1);
}

DKStringU8 DKStringU8::operator + (DKUniChar8 c) const
{
	return DKStringU8(*this).Append(&c, 1);
}

// convert numeric values.
long long DKStringU8::ToInteger(void) const
{
	if (stringData && stringData[0])
		return strtoll(stringData, 0, 0);
	return 0LL;
}

unsigned long long DKStringU8::ToUnsignedInteger(void) const
{
	if (stringData && stringData[0])
		return strtoull(stringData, 0, 0);
	return 0ULL;
}

double DKStringU8::ToRealNumber(void) const
{
	if (stringData && stringData[0])
		return strtod(stringData, 0);
	return 0.0;
}
