//
//  File: DKStringW.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include <stdarg.h>		// for va_list
#include "DKObject.h"
#include "DKSet.h"
#include "DKArray.h"
#include "DKStringUE.h"

////////////////////////////////////////////////////////////////////////////////
// DKStringW
// a unicode string class with wchar_t character string.
// UTF-8, CP367 (ISO-8859, ASCII) are available also.
// (but convert and store with wchar_t string internally.)
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKData;
	class DKLIB_API DKStringW
	{
	public:
		typedef DKSet<DKUniCharW, DKDummyLock, DKSetComparison<DKUniCharW>> CharacterSet;
		typedef DKArray<long long, DKDummyLock> IntegerArray;
		typedef DKArray<unsigned long long, DKDummyLock> UnsignedIntegerArray;
		typedef DKArray<double, DKDummyLock> RealNumberArray;
		typedef DKArray<DKStringW, DKDummyLock> StringArray;

		static const DKStringW& EmptyString();
		// DKUniCharW encoding with endianness.
		static DKStringEncoding SystemEncoding(void);

		// DKStringW class
		DKStringW(void);
		DKStringW(DKStringW&& str);
		DKStringW(const DKStringW& str);
		DKStringW(const DKUniCharW* str, size_t len = (size_t)-1);
		DKStringW(const DKUniChar8* str, size_t len = (size_t)-1);
		explicit DKStringW(const void* str, size_t len, DKStringEncoding e);
		explicit DKStringW(DKUniCharW c);
		explicit DKStringW(DKUniChar8 c);
		~DKStringW(void);

		DKObject<DKData> Encode(DKStringEncoding e) const;
		// formatting string.
		// for wchar_t format string, there is some restriction described below.
		// '%s' is utf-8, cp367, iso-8859 only. For DKStringU8, should be cast of 'const char*'.
		// '%ls' is wchar_t only. For DKStringW, should be cast of 'const wchar_t*).
		// Don't use '%S' (Uppercase S), gcc, msvc does not produces same result.
		static DKStringW Format(const DKUniChar8* fmt, ...);
		static DKStringW Format(const DKUniCharW* fmt, ...);
		static DKStringW FormatV(const DKUniChar8* fmt, va_list v);
		static DKStringW FormatV(const DKUniCharW* fmt, va_list v);

		size_t Length(void) const;
		size_t Bytes(void) const;

		long Find(DKUniCharW c, long begin = 0) const;
		long Find(const DKUniCharW* str, long begin = 0) const;
		long Find(const DKStringW& str, long begin = 0) const;
		long FindWhitespaceCharacter(long begin = 0) const;
		long FindAnyCharactersInSet(const CharacterSet& cs, long begin = 0) const;

		DKStringW Right(long index) const;  // copy right-side from index.
		DKStringW Left(size_t count) const; // copy 'count' length characters from beginning.
		DKStringW Mid(long index, size_t count) const; // copy 'count' length characters from index.

		DKStringW LowercaseString(void) const;
		DKStringW UppercaseString(void) const;
				
		int Compare(const DKUniCharW* str) const;
		int Compare(const DKStringW& str) const;
		int CompareNoCase(const DKUniCharW* str) const;
		int CompareNoCase(const DKStringW& str) const;
		
		int Replace(const DKUniCharW c1, const DKUniCharW c2);
		int Replace(const DKUniCharW* strOld, const DKUniCharW* strNew);
		int Replace(const DKStringW& src, const DKStringW& dst); 
		DKStringW& Insert(long index, const DKUniCharW* str);
		DKStringW& Insert(long index, DKUniCharW ch);

		bool HasPrefix(const DKStringW& str) const;
		bool HasSuffix(const DKStringW& str) const;
		DKStringW& RemovePrefix(const DKStringW& str);
		DKStringW& RemoveSuffix(const DKStringW& str);

		// file system path string.
		// convert path separator charactor ('\\' on win32, '/' on Unix)
		DKStringW FilePathString(void) const;
		DKStringW FilePathStringByAppendingPath(const DKStringW& path) const;
		DKStringW LastPathComponent(void) const;
		StringArray PathComponents(void) const;

		// whitespaces
		bool IsWhitespaceCharacterAtIndex(long index) const;
		// trim whitespaces (with escape sequences) both side of beginning, ending.
		DKStringW& TrimWhitespaces(void);
		// remove whitespaces (with escape sequences) for all sequences in range.
		DKStringW& RemoveWhitespaces(long begin = 0, long count = -1);

		// append, set
		DKStringW& Append(const DKStringW& str);
		DKStringW& Append(const DKUniCharW* str, size_t len = (size_t)-1);
		DKStringW& Append(const DKUniChar8* str, size_t len = (size_t)-1);
		DKStringW& Append(const void* str, size_t bytes, DKStringEncoding e);
		DKStringW& SetValue(const DKStringW& str);
		DKStringW& SetValue(const DKUniCharW* str, size_t len = (size_t)-1);
		DKStringW& SetValue(const DKUniChar8* str, size_t len = (size_t)-1);
		DKStringW& SetValue(const void* str, size_t bytes, DKStringEncoding e);

		// assignment operators
		DKStringW& operator = (DKStringW&& str);
		DKStringW& operator = (const DKStringW& str);
		DKStringW& operator = (const DKUniCharW* str);
		DKStringW& operator = (const DKUniChar8* str);
		DKStringW& operator = (DKUniCharW ch);
		DKStringW& operator = (DKUniChar8 ch);

		// conversion operators
		operator const DKUniCharW* (void) const;

		// concatenation operators
		DKStringW& operator += (const DKStringW& str);
		DKStringW& operator += (const DKUniCharW* str);
		DKStringW& operator += (const DKUniChar8* str);
		DKStringW& operator += (DKUniCharW ch);
		DKStringW& operator += (DKUniChar8 ch);
		DKStringW operator + (const DKStringW& str) const;
		DKStringW operator + (const DKUniCharW* str) const;
		DKStringW operator + (const DKUniChar8* str) const;
		DKStringW operator + (DKUniCharW c) const;
		DKStringW operator + (DKUniChar8 c) const;

		// comparison operators
		bool operator > (const DKStringW& str) const			{return Compare(str) > 0;}
		bool operator > (const DKUniCharW* str) const			{return Compare(str) > 0;}
		bool operator < (const DKStringW& str) const			{return Compare(str) < 0;}
		bool operator < (const DKUniCharW* str) const			{return Compare(str) < 0;}
		bool operator >= (const DKStringW& str) const			{return Compare(str) >= 0;}
		bool operator >= (const DKUniCharW* str) const			{return Compare(str) >= 0;}
		bool operator <= (const DKStringW& str) const			{return Compare(str) <= 0;}
		bool operator <= (const DKUniCharW* str) const			{return Compare(str) <= 0;}
		bool operator == (const DKStringW& str) const			{return Compare(str) == 0;}
		bool operator == (const DKUniCharW* str) const			{return Compare(str) == 0;}
		bool operator != (const DKStringW& str) const			{return Compare(str) != 0;}
		bool operator != (const DKUniCharW* str) const			{return Compare(str) != 0;}

		// convert numeric values.
		long long ToInteger(void) const;
		unsigned long long ToUnsignedInteger(void) const;
		double ToRealNumber(void) const;

		IntegerArray ToIntegerArray(const DKStringW& delimiter, bool ignoreEmptyString = false) const;
		UnsignedIntegerArray ToUnsignedIntegerArray(const DKStringW& delimiter, bool ignoreEmptyString = false) const;
		RealNumberArray ToRealNumberArray(const DKStringW& delimiter, bool ignoreEmptyString = false) const;

		// split string into sub-string array. (StringArray)
		StringArray Split(const DKStringW& delimiter, bool ignoreEmptyString = false) const;
		StringArray SplitByCharactersInSet(const CharacterSet& cs, bool ignoreEmptyString = false) const;
		StringArray SplitByWhitespace(void) const;

	private:
		DKUniCharW* stringData;
	};
}
