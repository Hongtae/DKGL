﻿//
//  File: DKStringUE.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include <stdarg.h>		// for va_list

////////////////////////////////////////////////////////////////////////////////
// DKStringUE.h
// a unicode string conversion utility.
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	enum class DKStringEncoding
	{
		UTF8,
		UTF16,		// system byteorder
		UTF32,		// system byteorder
		UTF16BE,
		UTF16LE,
		UTF32BE,
		UTF32LE,
	};

#if defined(DKUNICODE_NATIVE)
	typedef char			DKUniChar8;
	typedef char16_t		DKUniChar16;
	typedef char32_t		DKUniChar32;
#elif defined(DKUNICHAR_UNSIGNED)
	typedef unsigned char	DKUniChar8;
	typedef unsigned short	DKUniChar16;
	typedef unsigned int	DKUniChar32;
#else
	typedef char			DKUniChar8;
	typedef short			DKUniChar16;
	typedef int				DKUniChar32;
#endif
	typedef wchar_t			DKUniCharW;
	
	static_assert(sizeof(char) != sizeof(wchar_t), "wchar_t must be greater than char");

	class DKBuffer;
	class DKStringU8;
	class DKStringW;

	DKGL_API DKStringEncoding DKStringWEncoding(void);
	DKGL_API DKStringEncoding DKStringU8Encoding(void);	// always return DKStringEncodingUTF8
	DKGL_API const char* DKStringEncodingCanonicalName(DKStringEncoding e);

	DKGL_API void DKStringEncode(DKBuffer* output, const DKStringU8& input, DKStringEncoding e);
	DKGL_API void DKStringEncode(DKBuffer* output, const DKStringW& input, DKStringEncoding e);

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const void* p, size_t bytes, DKStringEncoding e);
	DKGL_API bool DKStringSetValue(DKStringW& strOut, const void* p, size_t bytes, DKStringEncoding e);

	DKGL_API void DKStringFormatV(DKStringU8& strOut, const DKUniChar8* fmt, va_list v);
	DKGL_API void DKStringFormatV(DKStringU8& strOut, const DKUniCharW* fmt, va_list v);
	DKGL_API void DKStringFormatV(DKStringW& strOut, const DKUniChar8* fmt, va_list v);
	DKGL_API void DKStringFormatV(DKStringW& strOut, const DKUniCharW* fmt, va_list v);

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKStringW& strIn);
	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKStringU8& strIn);

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKUniChar8* strIn, size_t len);
	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKUniChar16* strIn, size_t len);
	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKUniChar32* strIn, size_t len);
	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKUniCharW* strIn, size_t len);
	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKUniChar8* strIn, size_t len);
	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKUniChar16* strIn, size_t len);
	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKUniChar32* strIn, size_t len);
	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKUniCharW* strIn, size_t len);
}
