//
//  File: DKStringUE.cpp
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
#include "DKBuffer.h"
#include "DKEndianness.h"
#include "DKString.h"
#include "DKStringUE.h"
#include "DKStringU8.h"
#include "DKStringW.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

namespace DKFoundation
{
	namespace Private
	{
		template <size_t> struct WCharTraits;
		template <> struct WCharTraits<4>	
		{
#if		defined(__BIG_ENDIAN__)
			const static DKStringEncoding encoding = DKStringEncoding::UTF32BE;

#elif	defined(__LITTLE_ENDIAN__)
			const static DKStringEncoding encoding = DKStringEncoding::UTF32LE;
#else
#error System endianness not defined.
#endif
			typedef DKUniChar32 BaseCharT;
		};
		template <> struct WCharTraits<2>
		{
#if		defined(__BIG_ENDIAN__)
			const static DKStringEncoding encoding = DKStringEncoding::UTF16BE;
#elif	defined(__LITTLE_ENDIAN__)
			const static DKStringEncoding encoding = DKStringEncoding::UTF16LE;
#else
#error System endianness not defined.
#endif
			typedef DKUniChar16 BaseCharT;
		};
		typedef WCharTraits<sizeof(wchar_t)> StringWTraits;
		static_assert(sizeof(StringWTraits::BaseCharT) == sizeof(wchar_t), "size should be equal.");


		////////////////////////////////////////////////////////////////////////////////
		// UNICODE Conversion sources based on:
		// http://clang.llvm.org/doxygen/ConvertUTF_8c_source.html
		////////////////////////////////////////////////////////////////////////////////

		// using 'unsigned' for internal processes.
		typedef uint8_t UIntUTF8;
		typedef uint16_t UIntUTF16;
		typedef uint32_t UIntUTF32;

		static const char trailingBytesForUTF8[256] = {
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5				
		};
		static const UIntUTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };
		static const UIntUTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

		static bool IsLegalUTF8(const UIntUTF8* str, size_t len)
		{
			UIntUTF8 ch;
			const UIntUTF8* p = &str[len];
			switch (len)
			{
			default:
				return false;
			case 4:	if ((ch = (*--p)) < 0x80 || ch > 0xbf)	return false;
			case 3: if ((ch = (*--p)) < 0x80 || ch > 0xbf)	return false;
			case 2: if ((ch = (*--p)) > 0xbf) return false;
				switch (*str & 0xff)
				{
				case 0xe0: if (ch < 0xa0) return false; break;
				case 0xed: if (ch > 0x9f) return false; break;
				case 0xf0: if (ch < 0x90) return false; break;
				case 0xf4: if (ch < 0x80) return false;
				}
			case 1: if (static_cast<unsigned char>(*str & 0xff) >= 0x80 && static_cast<unsigned char>(*str & 0xff) < 0xc2) return false;
			}
			if ((*str & 0xff) > 0xf4) return false;
			return true;
		}
		bool IsLegalUTF8String(const DKUniChar8* input, size_t inputLen)
		{
			const UIntUTF8* inputBegin = reinterpret_cast<const UIntUTF8*>(input);
			const UIntUTF8* inputEnd = reinterpret_cast<const UIntUTF8*>(&input[inputLen]);
			while (inputBegin != inputEnd)
			{
				size_t length = trailingBytesForUTF8[(*inputBegin) & 0xff] + 1;
				if (length > (inputEnd - inputBegin) || !IsLegalUTF8(inputBegin, length))
					return false;
				inputBegin += length;
			}
			return true;
		}

#define UNICODE_HIGH_SURROGATE_BEGIN	0xD800
#define UNICODE_HIGH_SURROGATE_END		0xDBFF
#define UNICODE_LOW_SURROGATE_BEGIN		0xDC00
#define UNICODE_LOW_SURROGATE_END		0xDFFF

#define UNICODE_REPLACEMENT_CHAR		0x0000FFFD
#define UNICODE_MAX_UTF16				0x0010FFFF
#define UNICODE_MAX_UTF32				0x7FFFFFFF
#define UNICODE_MAX_LEGAL_UTF32			0x0010FFFF
#define UNICODE_HALF_BASE				0x10000
#define UNICODE_HALF_MASK				0x3FFU
#define UNICODE_HALF_SHIFT				10

		template <typename UTF16Setter> bool ConvertUTF8toUTF16(const UIntUTF8* input, const UIntUTF8* inputEnd, bool strict, UTF16Setter&& setter)
		{
			while (input < inputEnd)
			{
				UIntUTF32 ch = 0;
				unsigned short extraBytesToRead = trailingBytesForUTF8[*input & 0xff];
				if (extraBytesToRead >= inputEnd - input)
					return false;
				if (!IsLegalUTF8(input, extraBytesToRead+1))
					return false;

				switch (extraBytesToRead)
				{
				case 5: ch += (*input++) & 0xff; ch <<= 6;		// illegal utf8
				case 4: ch += (*input++) & 0xff; ch <<= 6;		// illegal utf8
				case 3: ch += (*input++) & 0xff; ch <<= 6;
				case 2: ch += (*input++) & 0xff; ch <<= 6;
				case 1: ch += (*input++) & 0xff; ch <<= 6;
				case 0: ch += (*input++) & 0xff;
				}
				ch -= offsetsFromUTF8[extraBytesToRead];
				if (ch <= 0xFFFF)
				{
					// surrogate pair is not valid on UTF-32
					if (ch >= UNICODE_HIGH_SURROGATE_BEGIN && ch <= UNICODE_LOW_SURROGATE_END)
					{
						if (strict)
							return false;
						else
							setter((UIntUTF16)UNICODE_REPLACEMENT_CHAR);
					}
					else
						setter((UIntUTF16)ch); // valid character.
				}
				else if (ch > UNICODE_MAX_UTF16) // out of range.
				{
					if (strict)
						return false;
					else
						setter((UIntUTF16)UNICODE_REPLACEMENT_CHAR);
				}
				else // 0xFFFF ~ 0x10FFFF is valid range.
				{
					ch -= UNICODE_HALF_BASE;
					setter((UIntUTF16)((ch >> UNICODE_HALF_SHIFT) + UNICODE_HIGH_SURROGATE_BEGIN));
					setter((UIntUTF16)((ch & UNICODE_HALF_MASK) + UNICODE_LOW_SURROGATE_BEGIN));
				}
			}
			return true;
		}
		template <typename UTF32Setter> bool ConvertUTF8toUTF32(const UIntUTF8* input, const UIntUTF8* inputEnd, bool strict, UTF32Setter&& setter)
		{
			while (input < inputEnd)
			{
				unsigned short extraBytesToRead = trailingBytesForUTF8[*input & 0xff];
				if (extraBytesToRead >= inputEnd - input)
					return false;
				if (!IsLegalUTF8(input, extraBytesToRead+1))
					return false;

				UIntUTF32 ch = 0;	
				switch (extraBytesToRead)
				{
				case 5: ch += (*input++) & 0xff; ch <<= 6;
				case 4: ch += (*input++) & 0xff; ch <<= 6;
				case 3: ch += (*input++) & 0xff; ch <<= 6;
				case 2: ch += (*input++) & 0xff; ch <<= 6;
				case 1: ch += (*input++) & 0xff; ch <<= 6;
				case 0: ch += (*input++) & 0xff;
				}
				ch -= offsetsFromUTF8[extraBytesToRead];

				if (ch <= UNICODE_MAX_LEGAL_UTF32)
				{
					// UTF-16 surrogate is not valid on UTF-32
					// not valid character which bigger than 0x10FFFF
					if (ch >= UNICODE_HIGH_SURROGATE_BEGIN && ch <= UNICODE_LOW_SURROGATE_END)
					{
						if (strict)
							return false;
						else
							setter((UIntUTF32)UNICODE_REPLACEMENT_CHAR);
					}
					else
						setter((UIntUTF32)ch);
				}
				else // out of range.
				{
					if (strict)
						return false;
					else
						setter((UIntUTF32)UNICODE_REPLACEMENT_CHAR);
				}
			}
			return true;
		}
		template <typename UTF8Setter> bool ConvertUTF16toUTF8(const UIntUTF16* input, const UIntUTF16* inputEnd, bool strict, UTF8Setter&& setter)
		{
			while (input < inputEnd)
			{
				UIntUTF32 ch = (*input++) & 0xffff;
				if (ch >= UNICODE_HIGH_SURROGATE_BEGIN && ch <= UNICODE_HIGH_SURROGATE_END)
				{
					// UTF-16 surrogate pair, convert UTF-32 first.
					if (input < inputEnd)
					{
						UIntUTF32 ch2 = (*input) & 0xffff;
						if (ch2 >= UNICODE_LOW_SURROGATE_BEGIN && ch2 <= UNICODE_LOW_SURROGATE_END)
						{
							// low-surrogate, convert UTF-32
							ch = ((ch - UNICODE_HIGH_SURROGATE_BEGIN) << UNICODE_HALF_SHIFT) + (ch2 - UNICODE_LOW_SURROGATE_BEGIN) + UNICODE_HALF_BASE;
							++input;
						}
						else if (strict) // high-surrogate, mismatched pair.
							return false;
					}
					else // string is too short.
					{
						return false;
					}
				}
				else if (strict)
				{
					// UTF-16 surrogate is not valid on UTF-32.
					if (ch >= UNICODE_LOW_SURROGATE_BEGIN && ch <= UNICODE_LOW_SURROGATE_END)
						return false;
				}

				// calculate result.
				unsigned short bytesToWrite = 0;
				if (ch < 0x80)							bytesToWrite = 1;
				else if (ch < 0x800)					bytesToWrite = 2;
				else if (ch < 0x10000)					bytesToWrite = 3;
				else if (ch < 0x110000)					bytesToWrite = 4;
				else { ch = UNICODE_REPLACEMENT_CHAR;	bytesToWrite = 3;}

				UIntUTF8 data[4];
				switch (bytesToWrite)
				{
				case 4:	data[3] = (UIntUTF8)((ch | 0x80) & 0xBF);	ch >>= 6;
				case 3: data[2] = (UIntUTF8)((ch | 0x80) & 0xBF);	ch >>= 6;
				case 2: data[1] = (UIntUTF8)((ch | 0x80) & 0xBF);	ch >>= 6;
				case 1: data[0] = (UIntUTF8)(ch | firstByteMark[bytesToWrite]);
					for (unsigned int i = 0; i < bytesToWrite; ++i)
						setter(data[i]);
				}
			}
			return true;
		}
		template <typename UTF32Setter> bool ConvertUTF16toUTF32(const UIntUTF16* input, const UIntUTF16* inputEnd, bool strict, UTF32Setter&& setter)
		{
			while (input < inputEnd)
			{
				UIntUTF32 ch = (*input++) & 0xffff;
				if (ch >= UNICODE_HIGH_SURROGATE_BEGIN && ch <= UNICODE_HIGH_SURROGATE_END)
				{
					// convert surrogate pair to UTF-32.
					if (input < inputEnd)
					{
						UIntUTF32 ch2 = (*input) & 0xffff;
						// convert UTF-32, if value is in low-surrogate range.
						if (ch2 >= UNICODE_LOW_SURROGATE_BEGIN && ch2 <= UNICODE_LOW_SURROGATE_END)
						{
							ch = ((ch - UNICODE_HIGH_SURROGATE_BEGIN) << UNICODE_HALF_SHIFT) + (ch2 - UNICODE_LOW_SURROGATE_BEGIN) + UNICODE_HALF_BASE;
							++input;
						}
						else if (strict) // no value for high-surrogate.
							return false;
					}						
				}
				else if (strict)
				{
					if (ch >= UNICODE_LOW_SURROGATE_BEGIN && ch <= UNICODE_LOW_SURROGATE_END)	// UTF-32 cannot have surrogate pair
						return false;
				}					
				setter((UIntUTF32)ch);
			}
			return true;
		}
		template <typename UTF8Setter> bool ConvertUTF32toUTF8(const UIntUTF32* input, const UIntUTF32* inputEnd, bool strict, UTF8Setter&& setter)
		{
			while (input < inputEnd)
			{
				UIntUTF32 ch = *input++;
				if (strict)
				{
					if (ch >= UNICODE_HIGH_SURROGATE_BEGIN && ch <= UNICODE_LOW_SURROGATE_END)	// UTF-16 surrogate pair is not valid on UTF-32
						return false;
				}

				// calculate result.
				unsigned short bytesToWrite = 0;
				if (ch < 0x80)								bytesToWrite = 1;
				else if (ch < 0x800)						bytesToWrite = 2;
				else if (ch < 0x10000)						bytesToWrite = 3;
				else if (ch <= UNICODE_MAX_LEGAL_UTF32)		bytesToWrite = 4;
				else { ch = UNICODE_REPLACEMENT_CHAR;		bytesToWrite = 3;}

				UIntUTF8 data[4];
				switch (bytesToWrite)
				{
				case 4:	data[3] = (UIntUTF8)((ch | 0x80) & 0xBF);	ch >>= 6;
				case 3: data[2] = (UIntUTF8)((ch | 0x80) & 0xBF);	ch >>= 6;
				case 2: data[1] = (UIntUTF8)((ch | 0x80) & 0xBF);	ch >>= 6;
				case 1: data[0] = (UIntUTF8)(ch | firstByteMark[bytesToWrite]);
					for (unsigned int i = 0; i < bytesToWrite; ++i)
						setter(data[i]);
				}
			}
			return true;
		}
		template <typename UTF16Setter> bool ConvertUTF32toUTF16(const UIntUTF32* input, const UIntUTF32* inputEnd, bool strict, UTF16Setter&& setter)
		{
			while (input < inputEnd)
			{
				UIntUTF32 ch = *input++;
				if (ch <= 0xFFFF)
				{
					if (ch >= UNICODE_HIGH_SURROGATE_BEGIN && ch <= UNICODE_LOW_SURROGATE_END)
					{
						if (strict)
							return false;
						else
							setter((UIntUTF16)UNICODE_REPLACEMENT_CHAR);
					}
					else
						setter((UIntUTF16)ch); // valid character.
				}
				else if (ch > UNICODE_MAX_LEGAL_UTF32)
				{
					if (strict)
						return false;
					else
						setter((UIntUTF16)UNICODE_REPLACEMENT_CHAR);
				}
				else // value is in range of 0xFFFF ~ 0x10FFFF
				{
					ch -= UNICODE_HALF_BASE;
					setter((UIntUTF16)((ch >> UNICODE_HALF_SHIFT) + UNICODE_HIGH_SURROGATE_BEGIN));
					setter((UIntUTF16)((ch & UNICODE_HALF_MASK) + UNICODE_LOW_SURROGATE_BEGIN));
				}
			}
			return true;
		}

		template <typename T> size_t UniCharLength(const T* str)
		{
			size_t len = 0;
			while (str[len])
				++len;
			return len;
		}
		template <typename T> size_t UniCharLength(const T* str, size_t maxLen)
		{
			for (size_t i = 0; i < maxLen; ++i)
			{
				if (str[i] == 0)
					return i;
			}
			return maxLen;
		}

		template <typename IN, typename OUT> struct UniCharArraySetter
		{
			UniCharArraySetter(DKArray<OUT>& arr) : output(arr) {}
			void operator () (IN ch) { output.Add(static_cast<OUT>(ch)); }
			DKArray<OUT>& output;
		};
		bool ConvertUniChars(const DKUniChar8* input, size_t length, DKArray<DKUniChar16>& output)
		{
			if (input && length > 0)
				return ConvertUTF8toUTF16(reinterpret_cast<const UIntUTF8*>(input), reinterpret_cast<const UIntUTF8*>(&input[length]), true, UniCharArraySetter<UIntUTF16, DKUniChar16>(output));
			return false;
		}
		bool ConvertUniChars(const DKUniChar8* input, size_t length, DKArray<DKUniChar32>& output)
		{
			if (input && length > 0)
				return ConvertUTF8toUTF32(reinterpret_cast<const UIntUTF8*>(input), reinterpret_cast<const UIntUTF8*>(&input[length]), true, UniCharArraySetter<UIntUTF32, DKUniChar32>(output));
			return false;
		}
		bool ConvertUniChars(const DKUniChar16* input, size_t length, DKArray<DKUniChar8>& output)
		{
			if (input && length > 0)
				return ConvertUTF16toUTF8(reinterpret_cast<const UIntUTF16*>(input), reinterpret_cast<const UIntUTF16*>(&input[length]), true, UniCharArraySetter<UIntUTF8, DKUniChar8>(output));
			return false;
		}
		bool ConvertUniChars(const DKUniChar16* input, size_t length, DKArray<DKUniChar32>& output)
		{
			if (input && length > 0)
				return ConvertUTF16toUTF32(reinterpret_cast<const UIntUTF16*>(input), reinterpret_cast<const UIntUTF16*>(&input[length]), true, UniCharArraySetter<UIntUTF32, DKUniChar32>(output));
			return false;
		}
		bool ConvertUniChars(const DKUniChar32* input, size_t length, DKArray<DKUniChar8>& output)
		{
			if (input && length > 0)
				return ConvertUTF32toUTF8(reinterpret_cast<const UIntUTF32*>(input), reinterpret_cast<const UIntUTF32*>(&input[length]), true, UniCharArraySetter<UIntUTF8, DKUniChar8>(output));
			return false;
		}
		bool ConvertUniChars(const DKUniChar32* input, size_t length, DKArray<DKUniChar16>& output)
		{
			if (input && length > 0)
				return ConvertUTF32toUTF16(reinterpret_cast<const UIntUTF32*>(input), reinterpret_cast<const UIntUTF32*>(&input[length]), true, UniCharArraySetter<UIntUTF16, DKUniChar16>(output));
			return false;
		}
		size_t NumberOfCharactersInUTF8(const DKUniChar8* input, size_t length)
		{
			size_t count = 0;
			if (input && length > 0)
			{
				auto counter = [&count](UIntUTF32)
				{
					count++;
				};
				if (ConvertUTF8toUTF32(reinterpret_cast<const UIntUTF8*>(input), reinterpret_cast<const UIntUTF8*>(&input[length]), true, counter) == false)
					count = 0;
			}
			return count;
		}
		bool EncodeString(const void* p, size_t len, DKStringEncoding from, DKStringEncoding to, DKBuffer* output)
		{
			if (p == NULL || len == 0 || output == NULL)
				return false;

			auto getEncoding = [](DKStringEncoding e) -> DKStringEncoding
			{
#if	defined(__BIG_ENDIAN__)
				if (e == DKStringEncoding::UTF16)		return DKStringEncoding::UTF16BE;
				else if (e == DKStringEncoding::UTF32)	return DKStringEncoding::UTF32BE;
#elif	defined(__LITTLE_ENDIAN__)
				if (e == DKStringEncoding::UTF16)		return DKStringEncoding::UTF16LE;
				else if (e == DKStringEncoding::UTF32)	return DKStringEncoding::UTF32LE;
#else
#error System endianness not defined.
#endif
				return e;
			};
			auto isNativeOrder = [](DKStringEncoding e) -> bool
			{
#if	defined(__BIG_ENDIAN__)
				if (e == DKStringEncoding::UTF16LE || e == DKStringEncoding::UTF32LE)	return false;
#elif	defined(__LITTLE_ENDIAN__)
				if (e == DKStringEncoding::UTF16BE || e == DKStringEncoding::UTF32BE)	return false;
#else
#error System endianness not defined.
#endif
				return true;
			};
			auto unitSize = [](DKStringEncoding e) -> size_t
			{
				switch (e)
				{
				case DKStringEncoding::UTF16:
				case DKStringEncoding::UTF16BE:
				case DKStringEncoding::UTF16LE:
					return sizeof(UIntUTF16);
				case DKStringEncoding::UTF32:
				case DKStringEncoding::UTF32BE:
				case DKStringEncoding::UTF32LE:
					return sizeof(UIntUTF32);
				}
				return sizeof(UIntUTF8);
			};

			DKStringEncoding inputEnc = getEncoding(from);
			DKStringEncoding outputEnc = getEncoding(to);
			if (inputEnc == outputEnc)
			{
				output->SetContent(p, len);
				return true;
			}

			size_t inputUnitSize = unitSize(inputEnc);
			if (isNativeOrder(inputEnc))
			{
				size_t outputUnitSize = unitSize(outputEnc);

				DKArray<char> buffer;
				auto outputSetter = [&buffer](const void* p, size_t s)
				{
					size_t count = buffer.Count();
					size_t cap = buffer.Capacity();
					if (cap - count < s)
						buffer.Reserve(cap + 1024);
					buffer.Add((const char*)p, s);
				};

				bool result = false;
				if (outputUnitSize == 1)		// to UTF8
				{
					auto tmp = [&](UIntUTF8 ch) {outputSetter(&ch, 1);};
					switch (inputUnitSize)
					{
					case 2:			// from UTF16
						{
							const UIntUTF16* inputBegin = reinterpret_cast<const UIntUTF16*>(p);
							const UIntUTF16* inputEnd = &inputBegin[len / inputUnitSize];
							result = ConvertUTF16toUTF8(inputBegin, inputEnd, true, tmp);
						}
						break;								
					case 4:			// from UTF32
						{
							const UIntUTF32* inputBegin = reinterpret_cast<const UIntUTF32*>(p);
							const UIntUTF32* inputEnd = &inputBegin[len / inputUnitSize];
							result = ConvertUTF32toUTF8(inputBegin, inputEnd, true, tmp);
						}
						break;
					}
				}
				else if (outputUnitSize == 2)		// to UTF16
				{
					static_assert( sizeof(UIntUTF16) == 2, "wrong size");
					auto tmp1 = [&](UIntUTF16 ch) {outputSetter(&ch, 2);};
					auto tmp2 = [&](UIntUTF16 ch) { ch = DKSwitchIntegralByteOrder(ch); outputSetter(&ch, sizeof(ch));};
					switch (inputUnitSize)
					{
					case 1:			// from UTF8
						{
							const UIntUTF8* inputBegin = reinterpret_cast<const UIntUTF8*>(p);
							const UIntUTF8* inputEnd = &inputBegin[len];
							if (isNativeOrder(outputEnc))
								result = ConvertUTF8toUTF16(inputBegin, inputEnd, true, tmp1);
							else
								result = ConvertUTF8toUTF16(inputBegin, inputEnd, true, tmp2);
						}
						break;								
					case 4:			// from UTF32
						{
							const UIntUTF32* inputBegin = reinterpret_cast<const UIntUTF32*>(p);
							const UIntUTF32* inputEnd = &inputBegin[len / inputUnitSize];
							if (isNativeOrder(outputEnc))
								result = ConvertUTF32toUTF8(inputBegin, inputEnd, true, tmp1);
							else
								result = ConvertUTF32toUTF8(inputBegin, inputEnd, true, tmp2);
						}
						break;
					}
				}
				else if (outputUnitSize == 4)		// to UTF32
				{
					static_assert( sizeof(UIntUTF32) == 4, "wrong size");
					auto tmp1 = [&](UIntUTF32 ch) {outputSetter(&ch, sizeof(ch));};
					auto tmp2 = [&](UIntUTF32 ch) {ch = DKSwitchIntegralByteOrder(ch); outputSetter(&ch, sizeof(ch));};
					switch (inputUnitSize)
					{
					case 1:			// from UTF8
						{
							const UIntUTF8* inputBegin = reinterpret_cast<const UIntUTF8*>(p);
							const UIntUTF8* inputEnd = &inputBegin[len];
							if (isNativeOrder(outputEnc))
								result = ConvertUTF8toUTF32(inputBegin, inputEnd, true, tmp1);
							else
								result = ConvertUTF8toUTF32(inputBegin, inputEnd, true, tmp2);
						}
						break;
					case 2:			// from UTF16
						{
							const UIntUTF16* inputBegin = reinterpret_cast<const UIntUTF16*>(p);
							const UIntUTF16* inputEnd = &inputBegin[len / inputUnitSize];
							if (isNativeOrder(outputEnc))
								result = ConvertUTF16toUTF32(inputBegin, inputEnd, true, tmp1);
							else
								result = ConvertUTF16toUTF32(inputBegin, inputEnd, true, tmp2);
						}
						break;
					}
				}
				if (result)
				{
					output->SetContent( (char*)buffer, buffer.Count() );						
				}
				return result;
			}
			else
			{
				if (inputUnitSize == 2)
				{
					UIntUTF16* p2 = reinterpret_cast<UIntUTF16*>(malloc(len));
					const UIntUTF16* pc = reinterpret_cast<const UIntUTF16*>(p);
					size_t s = len / inputUnitSize;
					for (size_t i = 0; i < s; ++i )
						p2[i] = DKSwitchIntegralByteOrder(pc[i]);

					bool result = EncodeString(p2, len, DKStringEncoding::UTF16, outputEnc, output);
					free(p2);
					return result;
				}
				else if (inputUnitSize == 4)
				{
					UIntUTF32* p2 = reinterpret_cast<UIntUTF32*>(malloc(len));
					const UIntUTF32* pc = reinterpret_cast<const UIntUTF32*>(p);
					size_t s = len / inputUnitSize;
					for (size_t i = 0; i < s; ++i )
						p2[i] = DKSwitchIntegralByteOrder(pc[i]);

					bool result = EncodeString(p2, len, DKStringEncoding::UTF32, outputEnc, output);
					free(p2);
					return result;
				}
				return false;
			}
			return false;
		}

		// IEEE printf specification
		// http://www.opengroup.org/onlinepubs/009695399/functions/printf.html

#define DKSTRING_FORMAT_FLAG_LEFTALIGN			0x01
#define DKSTRING_FORMAT_FLAG_WITHSIGN			0x02
#define DKSTRING_FORMAT_FLAG_ZEROPADDING		0x04
#define DKSTRING_FORMAT_FLAG_BLANKSIGN			0x08
#define DKSTRING_FORMAT_FLAG_PREFIX				0x10

		struct ArgumentInfo
		{
			unsigned char flags;
			int width;
			int precision;
			bool widthFromArg;
			bool precisionFromArg;
			unsigned char length;
			char lengthFmt[2];		// hh, h, l, ll, j,z,t,L
			char type;
		};

		// ParseArgument: paring specifier(%) in format string.
		// %[flags][width][.precision][length]type
		size_t ParseArgument(ArgumentInfo& info, char* fmt)
		{
			size_t offset = 0;
			// flag
			info.flags = 0;
			while (fmt[offset])
			{
				if (fmt[offset] == '-')
					info.flags |= DKSTRING_FORMAT_FLAG_LEFTALIGN;
				else if (fmt[offset] == '+')
					info.flags |= DKSTRING_FORMAT_FLAG_WITHSIGN;
				else if (fmt[offset] == '0')
					info.flags |= DKSTRING_FORMAT_FLAG_ZEROPADDING;
				else if (fmt[offset] == ' ')
					info.flags |= DKSTRING_FORMAT_FLAG_BLANKSIGN;
				else if (fmt[offset] == '#')
					info.flags |= DKSTRING_FORMAT_FLAG_PREFIX;
				else
					break;
				offset++;
			}
			// width
			if (fmt[offset] == '*')
			{
				info.width = 0;
				info.widthFromArg = true;
				offset++;
			}
			else
			{
				info.widthFromArg = false;
				long exp = 1;
				info.width = 0;
				while (fmt[offset])
				{
					if (fmt[offset] >= '0' && fmt[offset] <= '9')
					{
						int num = fmt[offset] - '0';
						info.width *= exp;
						info.width += num;
					}
					else
						break;
					exp *= 10;
					offset++;
				}
				if (exp == 1)
					info.width = -1; // width not defined.
			}
			// precision
			if (fmt[offset] == '.')
			{
				if (fmt[++offset] == '*')
				{
					info.precision = 0;
					info.precisionFromArg = true;
					offset++;
				}
				else
				{
					info.precisionFromArg = false;
					long exp = 1;
					info.precision = 0;
					while (fmt[offset])
					{
						if (fmt[offset] >= '0' && fmt[offset] <= '9')
						{
							int num = fmt[offset] - '0';
							info.precision *= exp;
							info.precision += num;
						}
						else
							break;
						exp *= 10;
						offset++;
					}
					if (exp == 1)
						info.precision = -1;
				}
			}
			else
			{
				info.precisionFromArg = false;
				info.precision = -1; // precision not defined.
			}
			// length modifier (hh, h, l, ll, j, z, t, L)
			info.length = 0;
			info.lengthFmt[0] = info.lengthFmt[1] = 0;
			while (fmt[offset])
			{
				if (fmt[offset] == 'h')
				{
					if (info.lengthFmt[0] == fmt[offset])
					{
						info.length = sizeof(char);
						info.lengthFmt[1] = fmt[offset];
					}
					else
					{
						info.length = sizeof(short);
						info.lengthFmt[0] = fmt[offset];
						info.lengthFmt[1] = 0;
					}
				}
				else if (fmt[offset] == 'l')
				{
					if (info.lengthFmt[0] == fmt[offset])
					{
						info.length = sizeof(long long);
						info.lengthFmt[1] = fmt[offset];
					}
					else
					{
						info.length = sizeof(long);
						info.lengthFmt[0] = fmt[offset];
						info.lengthFmt[1] = 0;
					}
				}
				else if (fmt[offset] == 'j')
				{
					info.length = sizeof(long);
					info.lengthFmt[0] = fmt[offset];
					info.lengthFmt[1] = 0;
				}
				else if (fmt[offset] == 'z')
				{
					info.length = sizeof(size_t);
					info.lengthFmt[0] = fmt[offset];
					info.lengthFmt[1] = 0;
				}
				else if (fmt[offset] == 't')
				{
					info.length = sizeof(ptrdiff_t);
					info.lengthFmt[0] = fmt[offset];
					info.lengthFmt[1] = 0;
				}
				else if (fmt[offset] == 'L')
				{
					info.length = sizeof(long double);
					info.lengthFmt[0] = fmt[offset];
					info.lengthFmt[1] = 0;
				}
				else
					break;
				offset++;
			}
			// type (d,i,o,u,x,X,f,F,e,E,g,G,a,A,p,n,c,C,s,S,%)
			info.type = 0;
			switch (fmt[offset])
			{
			case 'd':case 'i':case 'o':case 'u':case 'x':case 'X':                   // integers
			case 'f':case 'F':case 'e':case 'E':case 'g':case 'G':case 'a':case 'A': // real numbers
			case 'p':case 'n':case 'c':case 'C':case 's':case 'S':case '%':          // etc.
				info.type = fmt[offset++];
				break;
			}
			return offset;
		}

		template <typename UTF8StringPrinter> void PrintV(UTF8StringPrinter& printer, const char* fmt, va_list ap)
		{
			DKArray<char> tmp;
			tmp.Resize(1024);
			char format[100];

			DKArray<DKUniChar8> utf8buff;
			utf8buff.Reserve(256);

			const char* nullStr = "(null)";
			const size_t nullStrLen = strlen(nullStr);

			char* pch = (char*)fmt;
			char ch;
			while ((ch = *(pch++)))
			{
				if (ch != '%')
				{
					utf8buff.Add(ch);
				}
				else
				{
					if (utf8buff.Count() > 0)
					{
						printer((const DKUniChar8*)utf8buff, utf8buff.Count());
					}
					utf8buff.Clear();
					tmp.Value(0) = 0;

					ArgumentInfo info;
					pch += ParseArgument(info, pch);

					size_t formatLen = 0;
					format[formatLen++] = '%';
					if (info.flags & DKSTRING_FORMAT_FLAG_LEFTALIGN)
						format[formatLen++] = '-';
					if (info.flags & DKSTRING_FORMAT_FLAG_WITHSIGN)
						format[formatLen++] = '+';
					if (info.flags & DKSTRING_FORMAT_FLAG_ZEROPADDING)
						format[formatLen++] = '0';
					if (info.flags & DKSTRING_FORMAT_FLAG_BLANKSIGN)
						format[formatLen++] = ' ';
					if (info.flags & DKSTRING_FORMAT_FLAG_PREFIX)
						format[formatLen++] = '#';

					if (info.widthFromArg)
						info.width = va_arg(ap, int);
					if (info.width >= 0)
						formatLen += snprintf(&format[formatLen], sizeof(format) - formatLen, "%u", info.width);
					if (info.precisionFromArg)
						info.precision = va_arg(ap, int);
					if (info.precision >= 0)
						formatLen += snprintf(&format[formatLen], sizeof(format) - formatLen, ".%u", info.precision);

					if (info.lengthFmt[0])
					{
						format[formatLen++] = info.lengthFmt[0];
						if (info.lengthFmt[1])
							format[formatLen++] = info.lengthFmt[1];
					}

					if (info.type == 'C')
						format[formatLen++] = 'c'; // convert to utf-8 and print.
					else if (info.type == 'S')
						format[formatLen++] = 's'; // convert to utf-8 and print.
					else
						format[formatLen++] = info.type;
					format[formatLen++] = 0;

					switch (info.type)
					{
					case 'd':case 'i':case 'o':case 'u':case 'x':case 'X': // integers
						{
							int n = 0;
							if (info.length == sizeof(long long))
								n = snprintf(&tmp.Value(0), tmp.Count(), format, va_arg(ap, long long));
							else if (info.length == sizeof(long))
								n = snprintf(&tmp.Value(0), tmp.Count(), format, va_arg(ap, long));
							else
								n = snprintf(&tmp.Value(0), tmp.Count(), format, va_arg(ap, int));
							if (n > 0)
								utf8buff.Add(&tmp.Value(0), n);
						}
						break;
					case 'f':case 'F':case 'e':case 'E':case 'g':case 'G':case 'a':case 'A': // real numbers
						{
							int n = 0;
							if (info.length == sizeof(long double))
								n = snprintf(&tmp.Value(0), tmp.Count(), format, va_arg(ap, long double));
							else
								n = snprintf(&tmp.Value(0), tmp.Count(), format, va_arg(ap, double));
							if (n > 0)
								utf8buff.Add(&tmp.Value(0), n);
						}
						break;
					case 'p':	// pointer
						{
							int n = snprintf(&tmp.Value(0), tmp.Count(), format, va_arg(ap, void*));
							if (n > 0)
								utf8buff.Add(&tmp.Value(0), n);
						}
						break;
					case 'n':	// (ignore)
						{
						}
						break;
					case 'c':
						if (info.length < sizeof(long))
						{
							int n = snprintf(&tmp.Value(0), tmp.Count(), format, va_arg(ap, int));
							if (n > 0)
								utf8buff.Add(&tmp.Value(0), n);
							break;
						}
					case 'C':		// wchar_t -> utf-8
						{
							wchar_t str[2] = { va_arg(ap, wchar_t), 0 };
							ConvertUniChars((const StringWTraits::BaseCharT*)str, 1, utf8buff);
						}
						break;
					case 's':
						if (info.length < sizeof(long))
						{
							char* str = va_arg(ap, char*);
							if (str)
							{
								size_t len;
								for (len = 0; str[len]; len++) {}
								utf8buff.Add(str, len);
							}
							else
							{
								utf8buff.Add(nullStr, nullStrLen);		// (null)
							}
							break;
						}
					case 'S':		// wchar_t -> utf-8
						{
							wchar_t* str = va_arg(ap, wchar_t*);
							if (str)
							{
								ConvertUniChars((const StringWTraits::BaseCharT*)str, UniCharLength(str), utf8buff);
							}
							else
							{
								utf8buff.Add(nullStr, nullStrLen);		// (null)
							}
						}
						break;
					case '%':
						{
							utf8buff.Add('%');
						}
						break;
					}

					if (utf8buff.Count() > 0)
					{
						printer((const DKUniChar8*)utf8buff, utf8buff.Count());
						utf8buff.Clear();
					}
				}
			}
			if (utf8buff.Count() > 0)
			{
				printer((const DKUniChar8*)utf8buff, utf8buff.Count());
				utf8buff.Clear();
			}
		}
	}

	DKGL_API DKStringEncoding DKStringWEncoding(void)
	{
		return Private::StringWTraits::encoding;
	}

	DKGL_API DKStringEncoding DKStringU8Encoding(void)
	{
		return DKStringEncoding::UTF8;
	}

	DKGL_API const char* DKStringEncodingCanonicalName(DKStringEncoding e)
	{
		switch (e)
		{
		case DKStringEncoding::UTF8:		return "UTF-8";
		case DKStringEncoding::UTF16:		return "UTF-16";
		case DKStringEncoding::UTF16LE:		return "UTF-16LE";
		case DKStringEncoding::UTF16BE:		return "UTF-16BE";
		case DKStringEncoding::UTF32:		return "UTF-32";
		case DKStringEncoding::UTF32LE:		return "UTF-32LE";
		case DKStringEncoding::UTF32BE:		return "UTF-32BE";
		}
		return "";
	}

	DKGL_API void DKStringEncode(DKBuffer* output, const DKStringU8& input, DKStringEncoding e)
	{
		if (output && !Private::EncodeString((const DKUniChar8*)input, input.Bytes(), DKStringEncoding::UTF8, e, output))
		{
			output->SetContent(0,0);
		}
	}

	DKGL_API void DKStringEncode(DKBuffer* output, const DKStringW& input, DKStringEncoding e)
	{
		if (output && !Private::EncodeString((const DKUniCharW*)input, input.Bytes(), Private::StringWTraits::encoding, e, output))
		{
			output->SetContent(0,0);
		}
	}

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const void* p, size_t bytes, DKStringEncoding e)
	{
		if (e == DKStringEncoding::UTF8)
		{
			strOut.SetValue(reinterpret_cast<const DKUniChar8*>(p), bytes / sizeof(DKUniChar8));
			return true;
		}
		DKBuffer buffer;
		if (Private::EncodeString(p, bytes, e, DKStringEncoding::UTF8, &buffer))
		{
			strOut.SetValue(reinterpret_cast<const DKUniChar8*>(buffer.LockShared()), buffer.Length() / sizeof(DKUniChar8));
			buffer.UnlockShared();
			return true;
		}		
		return false;
	}

	DKGL_API bool DKStringSetValue(DKStringW& strOut, const void* p, size_t bytes, DKStringEncoding e)
	{
		if (e == Private::StringWTraits::encoding)
		{
			strOut.SetValue(reinterpret_cast<const DKUniCharW*>(p), bytes / sizeof(DKUniCharW));
			return true;
		}
		else
		{
			DKBuffer buffer;
			if (Private::EncodeString(p, bytes, e, Private::StringWTraits::encoding, &buffer))
			{
				strOut.SetValue(reinterpret_cast<const DKUniCharW*>(buffer.LockShared()), buffer.Length() / sizeof(DKUniCharW));
				buffer.UnlockShared();
				return true;
			}
		}
		return false;
	}

	DKGL_API void DKStringFormatV(DKStringU8& strOut, const DKUniChar8* fmt, va_list v)
	{
		strOut = (const DKUniChar8*)NULL;
		if (fmt && fmt[0])
		{
			auto printer = [&strOut](const DKUniChar8* str, size_t len)
			{
				strOut.Append(str, len);
			};
			Private::PrintV(printer, fmt, v);
		}
		else
			strOut.SetValue(DKStringU8::empty);
	}

	DKGL_API void DKStringFormatV(DKStringU8& strOut, const DKUniCharW* fmt, va_list v)
	{
		strOut = (const DKUniChar8*)NULL;
		if (fmt && fmt[0])
		{
			DKArray<DKUniChar8> buff;
			size_t len = Private::UniCharLength(fmt);
			buff.Reserve(len * 4 + 2);

			if (Private::ConvertUniChars(reinterpret_cast<const Private::StringWTraits::BaseCharT*>(fmt), len, buff))
			{
				buff.Add(0x00);		// null-terminated
				return DKStringFormatV(strOut, (const DKUniChar8*)buff, v);
			}
		}
		strOut.SetValue(DKStringU8::empty);
	}

	DKGL_API void DKStringFormatV(DKStringW& strOut, const DKUniChar8* fmt, va_list v)
	{
		strOut = (const DKUniCharW*)NULL;
		if (fmt && fmt[0])
		{
			auto printer = [&strOut](const DKUniChar8* str, size_t len)
			{
				strOut.Append(str, len);
			};
			Private::PrintV(printer, fmt, v);
		}
		else
			strOut.SetValue(DKStringW::empty);
	}

	DKGL_API void DKStringFormatV(DKStringW& strOut, const DKUniCharW* fmt, va_list v)
	{
		strOut = (const DKUniCharW*)NULL;
		if (fmt && fmt[0])
		{
			DKArray<DKUniChar8> buff;
			size_t len = Private::UniCharLength(fmt);
			buff.Reserve(len * 4 + 2);

			if (Private::ConvertUniChars(reinterpret_cast<const Private::StringWTraits::BaseCharT*>(fmt), len, buff))
			{
				buff.Add(0x00);		// null-terminated
				return DKStringFormatV(strOut, (const DKUniChar8*)buff, v);
			}
		}
		strOut.SetValue(DKStringW::empty);
	}

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKStringW& strIn)
	{
		const DKUniCharW* s = strIn;
		DKArray<DKUniChar8> utf8bytes;
		size_t len = Private::UniCharLength(s);
		utf8bytes.Reserve(len * 4 + 2);

		if (sizeof(DKUniCharW) == 4)
		{
			if (Private::ConvertUniChars(reinterpret_cast<const DKUniChar32*>(s), len, utf8bytes))
			{
				strOut.SetValue((DKUniChar8*)utf8bytes, utf8bytes.Count());
				return true;
			}
		}
		else
		{
			if (Private::ConvertUniChars(reinterpret_cast<const DKUniChar16*>(s), len, utf8bytes))
			{
				strOut.SetValue((DKUniChar8*)utf8bytes, utf8bytes.Count());
				return true;
			}
		}
		strOut.SetValue(DKStringU8::empty);
		return false;
	}

	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKStringU8& strIn)
	{
		const DKUniChar8* s = strIn;
		if (sizeof(DKUniCharW) == 4)
		{
			DKArray<DKUniChar32> utf32bytes;
			size_t len = Private::UniCharLength(s);
			utf32bytes.Reserve(len + 2);

			if (Private::ConvertUniChars(reinterpret_cast<const DKUniChar8*>(s), len, utf32bytes))
			{
				strOut.SetValue(reinterpret_cast<const DKUniCharW*>((DKUniChar32*)utf32bytes), utf32bytes.Count());
				return true;
			}
		}
		else
		{
			DKArray<DKUniChar16> utf16bytes;
			size_t len = Private::UniCharLength(s);
			utf16bytes.Reserve(len + 2);

			if (Private::ConvertUniChars(reinterpret_cast<const DKUniChar8*>(s), len, utf16bytes))
			{
				strOut.SetValue(reinterpret_cast<const DKUniCharW*>((DKUniChar16*)utf16bytes), utf16bytes.Count());
				return true;
			}
		}
		strOut.SetValue(DKStringW::empty);
		return false;
	}

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKUniChar8* strIn, size_t len)
	{
		strOut.SetValue(strIn, len);
		return true;
	}

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKUniChar16* strIn, size_t len)
	{
		if (strIn && len > 0)
		{
			DKArray<DKUniChar8> buff;
			len = Private::UniCharLength(strIn, len);
			buff.Reserve(len * 4 + 2);

			if (Private::ConvertUniChars(strIn, len, buff))
			{
				strOut.SetValue(buff, buff.Count());
				return true;
			}
		}
		strOut.SetValue(DKStringU8::empty);
		return false;
	}

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKUniChar32* strIn, size_t len)
	{
		if (strIn && len > 0)
		{
			DKArray<DKUniChar8> buff;
			len = Private::UniCharLength(strIn, len);
			buff.Reserve(len * 4 + 2);

			if (Private::ConvertUniChars(strIn, len, buff))
			{
				strOut.SetValue(buff, buff.Count());
				return true;
			}
		}
		strOut.SetValue(DKStringU8::empty);
		return false;
	}

	DKGL_API bool DKStringSetValue(DKStringU8& strOut, const DKUniCharW* strIn, size_t len)
	{
		return DKStringSetValue(strOut, reinterpret_cast<const Private::StringWTraits::BaseCharT*>(strIn), Private::UniCharLength(strIn, len));
	}

	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKUniChar8* strIn, size_t len)
	{
		if (strIn && len > 0)
		{
			DKArray<Private::StringWTraits::BaseCharT> buff;
			len = Private::UniCharLength(strIn, len);
			buff.Reserve(len + 2);

			if (Private::ConvertUniChars(strIn, len, buff))
			{
				strOut.SetValue(reinterpret_cast<DKUniCharW*>((Private::StringWTraits::BaseCharT*)buff), buff.Count());
				return true;
			}
		}
		strOut.SetValue(DKStringW::empty);
		return false;
	}

	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKUniChar16* strIn, size_t len)
	{
		if (sizeof(DKUniCharW) == 4)
		{
			if (strIn && len > 0)
			{
				DKArray<DKUniChar32> buff;
				len = Private::UniCharLength(strIn, len);
				buff.Reserve(len + 2);

				if (Private::ConvertUniChars(strIn, len, buff))
				{
					strOut.SetValue(reinterpret_cast<const DKUniCharW*>((DKUniChar32*)buff), buff.Count());
					return true;
				}
			}
		}
		else
		{
			strOut.SetValue(reinterpret_cast<const DKUniCharW*>(strIn), len);
			return true;
		}
		strOut.SetValue(DKStringW::empty);
		return false;
	}

	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKUniChar32* strIn, size_t len)
	{
		if (sizeof(DKUniCharW) == 4)
		{
			strOut.SetValue(reinterpret_cast<const DKUniCharW*>(strIn), len);
			return true;
		}
		else
		{
			if (strIn && len > 0)
			{
				DKArray<DKUniChar16> buff;
				for (size_t i = 0; i < len; ++i)
				{
					if (strIn[i] == 0)
					{
						len = i;
						break;
					}
				}
				len = Private::UniCharLength(strIn, len);
				buff.Reserve(len + 2);

				if (Private::ConvertUniChars(strIn, len, buff))
				{
					strOut.SetValue(reinterpret_cast<const DKUniCharW*>((DKUniChar16*)buff), buff.Count());
					return true;
				}
			}
		}
		strOut.SetValue(DKStringW::empty);
		return false;
	}

	DKGL_API bool DKStringSetValue(DKStringW& strOut, const DKUniCharW* strIn, size_t len)
	{
		strOut.SetValue(strIn, Private::UniCharLength(strIn, len));
		return true;
	}
}
