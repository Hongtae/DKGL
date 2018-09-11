//
//  File: DKStringW.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>

#include "DKArray.h"
#include "DKSet.h"
#include "DKStringW.h"
#include "DKStringU8.h"
#include "DKBuffer.h"

#ifdef _WIN32
#define wcstoll		_wcstoi64
#define wcstoull	_wcstoui64
#endif

#ifdef __ANDROID__
#warning "CHECK 'wcstoll', 'wcstoull' FOR ANDROID!"
int64_t int wcstoll(const wchar_t* str, wchar_t** endptr, int base)
{
	return DKStringU8(str).ToInteger();
}
uint64_t wcstoull(const wchar_t* str, wchar_t** endptr, int base)
{
	return DKStringU8(str).ToUnsignedInteger();
}
#endif

namespace DKFoundation
{
	namespace Private
	{
		namespace
		{
			const DKStringW::CharacterSet& WhitespaceCharacterSet()
			{
				static const struct WCSet
				{
					WCSet()
					{
						const DKUniCharW whitespaces[] = {
							0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x0020, 0x0085, 0x00a0,
							0x1680, 0x180e, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
							0x2006, 0x2007, 0x2008, 0x2009, 0x200a, 0x2028, 0x2029, 0x202f,
							0x205f, 0x3000
						};
						const size_t numChars = sizeof(whitespaces) / sizeof(DKUniCharW);
						set.Insert(whitespaces, numChars);
					}
					DKStringW::CharacterSet set;
				} whitespaceCharacterSet;
				return whitespaceCharacterSet.set;
			}

			inline DKUniCharW LowercaseChar(DKUniCharW c)
			{
				if (c >= L'A' && c <= L'Z')
					c = (c - L'A') + L'a';
				return c;
			}

			inline int CompareCaseInsensitive(const DKUniCharW* a, const DKUniCharW* b)
			{
				if (a == b)
					return 0;
				if (a == NULL)
					a = L"";
				if (b == NULL)
					b = L"";

				const DKUniCharW *p = a;
				const DKUniCharW *q = b;
				int cmp = 0;
				for ( ; *p || *q; ++p, ++q)
				{
					cmp = LowercaseChar(*p) - LowercaseChar(*q);
					if (cmp != 0)
						return cmp;
				}
				return LowercaseChar(*p) - LowercaseChar(*q);
			}

			inline int CompareCaseSensitive(const DKUniCharW* a, const DKUniCharW* b)
			{
				if (a == b)
					return 0;
				if (a == NULL)
					a = L"";
				if (b == NULL)
					b = L"";

				const DKUniCharW *p = a;
				const DKUniCharW *q = b;
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
	}
}

using namespace DKFoundation;

const DKStringW DKStringW::empty = L"";

DKStringEncoding DKStringW::SystemEncoding()
{
	return DKStringWEncoding();
}

// DKStringW class
DKStringW::DKStringW()
	: stringData(NULL)
{
}

DKStringW::DKStringW(DKStringW&& str)
	: stringData(NULL)
{
	stringData = str.stringData;
	str.stringData = NULL;
}

DKStringW::DKStringW(const DKStringW& str)
	: stringData(NULL)
{
	this->SetValue(str);
}

DKStringW::DKStringW(const DKUniCharW* str, size_t len)
	: stringData(NULL)
{
	this->SetValue(str, len);
}

DKStringW::DKStringW(const DKUniChar8* str, size_t len)
	: stringData(NULL)
{
	this->SetValue(str, len);
}

DKStringW::DKStringW(const void* str, size_t len, DKStringEncoding e)
	: stringData(NULL)
{
	this->SetValue(str, len, e);
}

DKStringW::DKStringW(DKUniCharW c)
	: stringData(NULL)
{
	this->SetValue(&c, 1);
}

DKStringW::DKStringW(DKUniChar8 c)
	: stringData(NULL)
{
	this->SetValue(&c, 1);
}

DKStringW::~DKStringW()
{
	if (stringData)
		DKFree(stringData);
}

DKStringW DKStringW::Format(const DKUniChar8* fmt, ...)
{
	DKStringW ret = L"";
	if (fmt && fmt[0])
	{
		va_list ap;
		va_start(ap, fmt);
		ret = FormatV(fmt, ap);
		va_end(ap);
	}
	return ret;
}

DKStringW DKStringW::Format(const DKUniCharW* fmt, ...)
{
	DKStringW ret = L"";
	if (fmt && fmt[0])
	{
		va_list ap;
		va_start(ap, fmt);
		ret = FormatV(fmt, ap);
		va_end(ap);
	}
	return ret;
}

DKStringW DKStringW::FormatV(const DKUniChar8* fmt, va_list v)
{
	DKStringW ret = L"";
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

DKStringW DKStringW::FormatV(const DKUniCharW* fmt, va_list v)
{
	DKStringW ret = L"";
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

size_t DKStringW::Length() const
{
	size_t len = 0;
	if (stringData)
	{
		while (stringData[len])
			len++;
	}
	return len;
}

size_t DKStringW::Bytes() const
{
	return Length() * sizeof(DKUniCharW);
}

long DKStringW::Find(DKUniCharW c, long begin) const
{
	if (begin < 0)	begin = 0;

	const DKUniCharW *data = stringData;
	size_t len = Length();
	for (long i = begin; i < (long)len; ++i)
	{
		if (data[i] == c)
			return (long)i;
	}
	return -1;
}

long DKStringW::Find(const DKUniCharW* str, long begin) const
{
	if (str == NULL)
		return -1;

	if (begin < 0)	begin = 0;

	long strLength = (long)wcslen(str);
	long maxLength = (long)Length() - strLength;

	for (long i = begin; i <= maxLength; ++i)
	{
		if (wcsncmp(&stringData[i], str, strLength) == 0)
			return (long)i;
	}
	return -1;
}

long DKStringW::Find(const DKStringW& str, long begin) const
{
	return Find((const DKUniCharW*)str, begin);
}

long DKStringW::FindWhitespaceCharacter(long begin) const
{
	return FindAnyCharactersInSet(Private::WhitespaceCharacterSet(), begin);
}

long DKStringW::FindAnyCharactersInSet(const CharacterSet& cs, long begin) const
{
	if (cs.Count() > 0)
	{
		size_t len = Length();
		for (size_t i = begin; i < len; ++i)
		{
			if (cs.Contains(this->stringData[i]))
				return (long)i;
		}
	}
	return -1;
}

DKStringW DKStringW::Right(long index) const
{
	DKStringW string;
	size_t len = Length();
	if (index < (long)len)
	{
		if (index < 0)
			index = 0;

		int count = len - index;
		if (count < 0)
			count = 0;

		DKUniCharW* tmp = (DKUniCharW*)DKMalloc((len+1) * sizeof(DKUniCharW));
		memset(tmp, 0, sizeof(DKUniCharW) * (len+1));
		wcsncpy(tmp, &stringData[index], count);

		string = tmp;

		DKFree(tmp);
	}
	return string;
}

DKStringW DKStringW::Left(size_t count) const
{
	DKStringW string;
	if (count > 0)
	{
		size_t len = Length();
		if (count > len)
			count = len;

		DKUniCharW* tmp = (DKUniCharW*)DKMalloc((len+1) * sizeof(DKUniCharW));
		memset(tmp, 0 , sizeof(DKUniCharW) * (len+1));
		wcsncpy(tmp, stringData, count);

		string = tmp;

		DKFree(tmp);
	}
	return string;
}

DKStringW DKStringW::Mid(long index, size_t count) const
{
	DKStringW string = L"";

	if (count == 0)
		return string;
	size_t len = Length();

	if (count > 0 && index + count < len)
	{
		DKUniCharW* buff = (DKUniCharW*)DKMalloc((count+1) * sizeof(DKUniCharW));
		wcsncpy(buff, &stringData[index], count);
		buff[count] = 0;

		string = buff;

		DKFree(buff);
	}
	else
	{
		string = Right(index);
	}
	return string;
}

DKStringW DKStringW::LowercaseString() const
{
	if (stringData)
	{
		DKUniCharW *buff = (DKUniCharW*)DKMalloc((Length()+1) * sizeof(DKUniCharW));
		int i;
		for (i = 0 ; stringData[i] != 0; ++i)
		{
			buff[i] = towlower(stringData[i]);
		}
		buff[i] = 0;

		DKStringW ret = buff;
		DKFree(buff);
		return ret;
	}
	return DKStringW(L"");			
}

DKStringW DKStringW::UppercaseString() const
{
	if (stringData)
	{
		DKUniCharW *buff = (DKUniCharW*)DKMalloc((Length()+1) * sizeof(DKUniCharW));
		int i;
		for (i = 0 ; stringData[i] != 0; ++i)
		{
			buff[i] = towupper(stringData[i]);
		}
		buff[i] = 0;

		DKStringW ret = buff;
		DKFree(buff);
		return ret;
	}
	return DKStringW(L"");			
}

int DKStringW::Compare(const DKUniCharW* str) const
{
	return Private::CompareCaseSensitive(stringData, str);
}

int DKStringW::Compare(const DKStringW& str) const
{
	return Private::CompareCaseSensitive(stringData, str.stringData);
}

int DKStringW::CompareNoCase(const DKUniCharW* str) const
{
	return Private::CompareCaseInsensitive(stringData, str);
}

int DKStringW::CompareNoCase(const DKStringW& str) const
{
	return Private::CompareCaseInsensitive(stringData, str.stringData);
}

int DKStringW::Replace(const DKUniCharW c1, const DKUniCharW c2)
{
	if (!stringData)
		return 0;
	if (c1 == c2)
		return 0;
	int result = 0;
	if (c1)
	{
		if (c2)
		{
			for (size_t i = 0; stringData[i]; ++i)
			{
				if (stringData[i] == c1)
				{
					stringData[i] = c2;
					++result;
				}
			}
		}
		else
		{
			size_t len = Length();
			DKUniCharW* tmp = (DKUniCharW*)DKMalloc((len+1) * sizeof(DKUniCharW));
			size_t tmpLen = 0;
			for (size_t i = 0; stringData[i]; ++i)
			{
				if (stringData[i] == c1)
					++result;
				else
					tmp[tmpLen++] = stringData[i];
			}
			tmp[tmpLen] = 0;
			this->SetValue(tmp, tmpLen);
			DKFree(tmp);
		}
	}
	return result;
}

int DKStringW::Replace(const DKUniCharW* strOld, const DKUniCharW* strNew)
{
	if (strOld == NULL || strOld[0] == 0)
		return 0;
	size_t len = Length();
	if (len == 0)
		return 0;

	if (strNew == NULL)
		strNew = L"";

	size_t len1 = (size_t)wcslen(strOld);

	if (len < len1)
		return 0;

	long index = Find(strOld);
	if (index < 0)
		return 0;

	DKStringW prev = Left(index) + strNew;
	DKStringW next = Right(index + len1);

	int ret = next.Replace(strOld, strNew) + 1;

	this->SetValue(prev + next);

	return ret;
}

int DKStringW::Replace(const DKStringW& src, const DKStringW& dst)
{
	return Replace((const DKUniCharW*)src, (const DKUniCharW*)dst);
}

DKStringW& DKStringW::Insert(long index, const DKUniCharW* str)
{
	if (str == NULL)
		return *this;

	size_t len = Length();
	if (index > (long)len)
	{
		return Append(str);
	}
	int newStrLen = (int)wcslen(str);
	if (newStrLen)
	{
		DKUniCharW* tmp = (DKUniCharW*)DKMalloc((len + newStrLen + 4) * sizeof(DKUniCharW));
		memset(tmp, 0, sizeof(DKUniCharW) * (len + newStrLen + 4));
		if (index > 0)
		{
			wcsncpy(tmp, stringData, index);
		}
		wcscat(tmp, str);
		wcscat(tmp, &stringData[index]);

		this->SetValue(tmp);

		DKFree(tmp);
	}
	return *this;
}

DKStringW& DKStringW::Insert(long index, DKUniCharW ch)
{
	return Insert(index, (const DKUniCharW*)DKStringW(ch));
}

DKStringW DKStringW::FilePathString() const
{
	DKStringW str(*this);
#ifdef _WIN32
	str.Replace(L'/', L'\\');
#else
	str.Replace(L'\\', L'/');
#endif
	return str;
}

DKStringW DKStringW::FilePathStringByAppendingPath(const DKStringW& path) const
{
	DKStringW str(*this);
	str.TrimWhitespaces();
	size_t len = str.Length();
	if (len > 0)
	{
		size_t pathLen = path.Length();
		if (pathLen > 0)
		{
			const wchar_t* pathStr = path;
			while (pathStr[0] == L'/' || pathStr[0] == L'\\')
				pathStr++;

			if (pathStr[0])
			{
				if (str[len - 1] != L'/' && str[len - 1] != L'\\')
				{
					str.Append(L"/");
				}
				str.Append(pathStr);
			}
		}
	}
	else
	{
		str = path;
		str.TrimWhitespaces();
	}
	return str.FilePathString();
}

DKStringW DKStringW::LastPathComponent() const
{
	DKStringW result = L"/";
	StringArray strs = PathComponents();
	size_t c = strs.Count();
	if (c > 0)
		result = strs.Value(c - 1);
	return result;
}

DKStringW::StringArray DKStringW::PathComponents() const
{
	CharacterSet cs = {L'/', L'\\'};
	return SplitByCharactersInSet(cs, true);
}

bool DKStringW::IsWhitespaceCharacterAtIndex(long index) const
{
	DKASSERT_DEBUG(Length() > index);
	return Private::WhitespaceCharacterSet().Contains(stringData[index]);
}

DKStringW& DKStringW::TrimWhitespaces()
{
	size_t len = Length();
	if (len == 0)
		return *this;

	size_t begin = 0;
	// finding whitespaces at beginning.
	while (begin < len)
	{
		if (!this->IsWhitespaceCharacterAtIndex(begin))
			break;
		begin++;
	}

	if (begin < len)
	{
		size_t end = len-1;
		// finding whitespaces at ending.
		while (end > begin)
		{
			if (!this->IsWhitespaceCharacterAtIndex(end))
				break;
			end--;
		}
		if (end >= begin)
		{
			DKStringW tmp = this->Mid(begin, end - begin + 1);
			return SetValue(tmp);
		}
	}
	else
	{
		// string is whitespaces entirely.
		return this->SetValue(L"");
	}
	return *this;
}

DKStringW& DKStringW::RemoveWhitespaces(long begin, long count)
{
	begin = Max(begin, 0);

	size_t length = Length();

	if (begin >= length)
		return *this;

	if (count < 0)
		count = length - begin;
	else
		count = Min<long>(length - begin, count);

	if (count <= 0)
		return *this;

	DKUniCharW* buffer = (DKUniCharW*)DKMalloc((count+2) * sizeof(DKUniCharW));
	size_t bufferIndex = 0;

	for (long i = 0; i < count; i++)
	{
		if (!IsWhitespaceCharacterAtIndex(i + begin))
		{
			buffer[bufferIndex++] = stringData[i+begin];
		}
	}
	buffer[bufferIndex] = NULL;

	DKStringW tmp = DKStringW(buffer) + Right(begin + count);

	DKFree(buffer);

	return *this = tmp;
}

bool DKStringW::HasPrefix(const DKStringW& str) const
{
	return str.Compare(this->Left( str.Length() )) == 0;
}

bool DKStringW::HasSuffix(const DKStringW& str) const
{
	return str.Compare(this->Right( this->Length() - str.Length() )) == 0;
}

DKStringW& DKStringW::RemovePrefix(const DKStringW& str)
{
	if (HasPrefix(str))
	{
		this->SetValue( this->Right( str.Length() ) );
	}
	return *this;
}

DKStringW& DKStringW::RemoveSuffix(const DKStringW& str)
{
	if (HasSuffix(str))
	{
		this->SetValue( this->Left( this->Length() - str.Length()) );
	}
	return *this;
}

DKStringW& DKStringW::Append(const DKStringW& str)
{
	return Append((const DKUniCharW*)str);
}

DKStringW& DKStringW::Append(const DKUniCharW* str, size_t len)
{
	if (str && str[0])
	{
		size_t len1 = Length();
		size_t len2 = 0;
		for (len2 = 0; str[len2] && len2 < len; len2++) {}

		size_t totalLen = len1 + len2;

		if (totalLen > 0)
		{
			DKUniCharW* buff = (DKUniCharW*)DKMalloc((len1 + len2 + 1) * sizeof(DKUniCharW));
			memset(buff, 0, sizeof(DKUniCharW) * (len1 + len2 + 1));

			if (stringData && stringData[0])
			{
				wcscat(buff, stringData);
				//wcscat_s(pNewBuff, nLen+nLen2+4, stringData);
			}

			wcscat(buff, str);
			//wcscat_s(pNewBuff, nLen+nLen2+4, str);

			if (stringData)
				DKFree(stringData);
			stringData = buff;
		}
	}
	return *this;
}

DKStringW& DKStringW::Append(const DKUniChar8* str, size_t len)
{
	DKStringW s = L"";
	DKStringSetValue(s, str, len);
	return this->Append(s);
}

DKStringW& DKStringW::Append(const void* str, size_t bytes, DKStringEncoding e)
{
	DKStringW s = L"";
	DKStringSetValue(s, str, bytes, e);
	return this->Append(s);
}

DKStringW& DKStringW::SetValue(const DKStringW& str)
{
	if (str.stringData == this->stringData)
		return *this;

	if (this->stringData)
		DKFree(this->stringData);
	this->stringData = NULL;

	size_t len = str.Length();
	if (len > 0)
	{
		DKASSERT_DEBUG(str.stringData != NULL);
		this->stringData = (DKUniCharW*)DKMalloc((len+1) * sizeof(DKUniCharW));
		wcscpy(this->stringData, str.stringData);
	}
	return *this;
}

DKStringW& DKStringW::SetValue(const DKUniCharW* str, size_t len)
{
	if (str == stringData && len >= this->Length())
		return *this;

	DKUniCharW* buff = NULL;
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
			buff = (DKUniCharW*)DKMalloc((len+1) * sizeof(DKUniCharW));
			memcpy(buff, str, len * sizeof(DKUniCharW));
			buff[len] = NULL;
		}
	}
	if (stringData)
		DKFree(stringData);
	stringData = buff;

	return *this;
}

DKStringW& DKStringW::SetValue(const DKUniChar8* str, size_t len)
{
	if (str && str[0])
		DKStringSetValue(*this, str, len);
	else
		SetValue((const DKUniCharW*)NULL, 0);

	return *this;
}

DKStringW& DKStringW::SetValue(const void* str, size_t bytes, DKStringEncoding e)
{
	DKStringSetValue(*this, str, bytes, e);
	return *this;
}

// assignment operators
DKStringW& DKStringW::operator = (DKStringW&& str)
{
	if (this != &str)
	{
		if (stringData)
			DKFree(stringData);

		stringData = str.stringData;
		str.stringData = NULL;
	}
	return *this;
}

DKStringW& DKStringW::operator = (const DKStringW& str)
{
	if (this != &str)
		return this->SetValue(str);
	return *this;
}

DKStringW& DKStringW::operator = (const DKUniCharW* str)
{
	return this->SetValue(str);
}

DKStringW& DKStringW::operator = (const DKUniChar8* str)
{
	return this->SetValue(str);
}

DKStringW& DKStringW::operator = (DKUniCharW ch)
{
	return this->SetValue(&ch, 1);
}

DKStringW& DKStringW::operator = (DKUniChar8 ch)
{
	return this->SetValue(&ch, 1);
}

// conversion operators
DKStringW::operator const DKUniCharW*() const
{
	if (this && this->stringData)
		return (const DKUniCharW*)this->stringData;
	return L"";
}

// concatention operators
DKStringW& DKStringW::operator += (const DKStringW& str)
{
	return Append(str);
}

DKStringW& DKStringW::operator += (const DKUniCharW* str)
{
	return Append(str);
}

DKStringW& DKStringW::operator += (const DKUniChar8* str)
{
	return Append(str);
}

DKStringW& DKStringW::operator += (DKUniCharW ch)
{
	return Append(&ch, 1);
}

DKStringW& DKStringW::operator += (DKUniChar8 ch)
{
	return Append(&ch, 1);
}

DKStringW DKStringW::operator + (const DKStringW& str) const
{
	return DKStringW(*this).Append(str);
}

DKStringW DKStringW::operator + (const DKUniCharW* str) const
{
	return DKStringW(*this).Append(str);
}

DKStringW DKStringW::operator + (const DKUniChar8* str) const
{
	return DKStringW(*this).Append(str);
}

DKStringW DKStringW::operator + (DKUniCharW c) const
{
	return DKStringW(*this).Append(&c, 1);
}

DKStringW DKStringW::operator + (DKUniChar8 c) const
{
	return DKStringW(*this).Append(&c, 1);
}

DKObject<DKData> DKStringW::Encode(DKStringEncoding e) const
{
	DKObject<DKBuffer> data = DKObject<DKBuffer>::New();
	DKStringEncode(data, *this, e);
	return data.SafeCast<DKData>();
}

int64_t DKStringW::ToInteger() const
{
	if (stringData && stringData[0])
		return wcstoll(stringData, 0, 0);
	return 0LL;
}

uint64_t DKStringW::ToUnsignedInteger() const
{
	if (stringData && stringData[0])
		return wcstoull(stringData, 0, 0);
	return 0ULL;
}

double DKStringW::ToRealNumber() const
{
	if (stringData && stringData[0])
		return wcstod(stringData, 0);
	return 0.0;
}

DKStringW::IntegerArray DKStringW::ToIntegerArray(const DKStringW& delimiter, bool ignoreEmptyString) const
{
	StringArray strings = Split(delimiter, ignoreEmptyString);
	IntegerArray result;
	result.Reserve(strings.Count());
	for (size_t i = 0; i < strings.Count(); ++i)
		result.Add(strings.Value(i).ToInteger());

	return result;
}

DKStringW::UnsignedIntegerArray DKStringW::ToUnsignedIntegerArray(const DKStringW& delimiter, bool ignoreEmptyString) const
{
	StringArray strings = Split(delimiter, ignoreEmptyString);
	UnsignedIntegerArray result;
	result.Reserve(strings.Count());
	for (size_t i = 0; i < strings.Count(); ++i)
		result.Add(strings.Value(i).ToUnsignedInteger());

	return result;
}

DKStringW::RealNumberArray DKStringW::ToRealNumberArray(const DKStringW& delimiter, bool ignoreEmptyString) const
{
	StringArray strings = Split(delimiter, ignoreEmptyString);
	RealNumberArray result;
	result.Reserve(strings.Count());
	for (size_t i = 0; i < strings.Count(); ++i)
		result.Add(strings.Value(i).ToRealNumber());

	return result;
}

DKStringW::StringArray DKStringW::Split(const DKStringW& delimiter, bool ignoreEmptyString) const
{
	StringArray strings;
	size_t len = Length();
	size_t dlen = delimiter.Length();
	if (dlen == 0)
	{
		strings.Add(*this);
		return strings;
	}

	long begin = 0;
	while (begin < len)
	{
		long next = this->Find(delimiter, begin);
		if (next >= begin)
		{
			DKStringW subString = this->Mid(begin, next - begin);
			if (ignoreEmptyString == false || subString.Length() > 0)
				strings.Add(this->Mid(begin, next - begin));
			begin = next + dlen;
		}
		else
		{
			DKStringW subString = this->Right(begin);
			if (subString.Length() > 0)
				strings.Add(subString);
			break;
		}
	}
	return strings;
}

DKStringW::StringArray DKStringW::SplitByCharactersInSet(const CharacterSet& cs, bool ignoreEmptyString) const
{
	StringArray strings;
	size_t len = Length();
	size_t numCs = cs.Count();
	if (numCs == 0)
	{
		strings.Add(*this);
		return strings;
	}

	long begin = 0;
	while (begin < len)
	{
		long next = this->FindAnyCharactersInSet(cs, begin);
		if (next >= begin)
		{
			DKStringW subString = this->Mid(begin, next - begin);
			if (ignoreEmptyString == false || subString.Length() > 0)
				strings.Add(subString);
			begin = next + 1;
		}
		else
		{
			DKStringW subString = this->Right(begin);
			if (subString.Length() > 0)
				strings.Add(subString);
			break;
		}
	}
	return strings;
}

DKStringW::StringArray DKStringW::SplitByWhitespace() const
{
	return SplitByCharactersInSet(Private::WhitespaceCharacterSet(), true);
}

