//
//  File: DKEndianness.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

#define SWITCH_BYTE_ORDER_UINT16(n) ((unsigned short)((((n) & 0xff00) >> 8) | (((n) & 0x00ff) << 8)))
#define SWITCH_BYTE_ORDER_UINT32(n)	((unsigned int)((( (n) & 0xff000000) >> 24) | (( (n) & 0x00ff0000) >> 8) | (( (n) & 0x0000ff00) << 8) | (( (n) & 0x000000ff) << 24)))
#define SWITCH_BYTE_ORDER_UINT64(n)	((unsigned long long)((((n) & 0xff00000000000000ULL) >> 56) | (((n) & 0x00ff000000000000ULL) >> 40) | (((n) & 0x0000ff0000000000ULL) >> 24) | (((n) & 0x000000ff00000000ULL) >> 8) | (((n) & 0x00000000ff000000ULL) << 8) | (((n) & 0x0000000000ff0000ULL) << 24) | (((n) & 0x000000000000ff00ULL) << 40) | (((n) & 0x00000000000000ffULL) << 56)))


#if		defined(__BIG_ENDIAN__)			// BIG_ENDIAN

#define SYSTEM_TO_LITTLE_ENDIAN_UINT16(n)			SWITCH_BYTE_ORDER_UINT16(n)
#define SYSTEM_TO_BIG_ENDIAN_UINT16(n)				((unsigned short)(n))
#define LITTLE_ENDIAN_TO_SYSTEM_UINT16(n)			SWITCH_BYTE_ORDER_UINT16(n)
#define BIG_ENDIAN_TO_SYSTEM_UINT16(n)				((unsigned short)(n))

#define SYSTEM_TO_LITTLE_ENDIAN_UINT32(n)			SWITCH_BYTE_ORDER_UINT32(n)
#define SYSTEM_TO_BIG_ENDIAN_UINT32(n)				((unsigned int)(n))
#define LITTLE_ENDIAN_TO_SYSTEM_UINT32(n)			SWITCH_BYTE_ORDER_UINT32(n)
#define BIG_ENDIAN_TO_SYSTEM_UINT32(n)				((unsigned int)(n))

#define SYSTEM_TO_LITTLE_ENDIAN_UINT64(n)			SWITCH_BYTE_ORDER_UINT64(n)
#define SYSTEM_TO_BIG_ENDIAN_UINT64(n)				((unsigned long long)(n))
#define LITTLE_ENDIAN_TO_SYSTEM_UINT64(n)			SWITCH_BYTE_ORDER_UINT64(n)
#define BIG_ENDIAN_TO_SYSTEM_UINT64(n)				((unsigned long long)(n))

#elif	defined(__LITTLE_ENDIAN__)		// LITTLE_ENDIAN

#define SYSTEM_TO_LITTLE_ENDIAN_UINT16(n)			((unsigned short)(n))
#define SYSTEM_TO_BIG_ENDIAN_UINT16(n)				SWITCH_BYTE_ORDER_UINT16(n)
#define LITTLE_ENDIAN_TO_SYSTEM_UINT16(n)			((unsigned short)(n))
#define BIG_ENDIAN_TO_SYSTEM_UINT16(n)				SWITCH_BYTE_ORDER_UINT16(n)

#define SYSTEM_TO_LITTLE_ENDIAN_UINT32(n)			((unsigned int)(n))
#define SYSTEM_TO_BIG_ENDIAN_UINT32(n)				SWITCH_BYTE_ORDER_UINT32(n)
#define LITTLE_ENDIAN_TO_SYSTEM_UINT32(n)			((unsigned int)(n))
#define BIG_ENDIAN_TO_SYSTEM_UINT32(n)				SWITCH_BYTE_ORDER_UINT32(n)

#define SYSTEM_TO_LITTLE_ENDIAN_UINT64(n)			((unsigned long long)(n))
#define SYSTEM_TO_BIG_ENDIAN_UINT64(n)				SWITCH_BYTE_ORDER_UINT64(n)
#define LITTLE_ENDIAN_TO_SYSTEM_UINT64(n)			((unsigned long long)(n))
#define BIG_ENDIAN_TO_SYSTEM_UINT64(n)				SWITCH_BYTE_ORDER_UINT64(n)

#else
#error System endianness not defined.
#endif


#ifdef __cplusplus
namespace DKFoundation
{
	////////////////////////////////////////////////////////////////////////////////
	// byteorder swap template functions.
	template <typename T> inline T DKSystemToBigEndian(T val)
	{
		if (sizeof(T) == 2)	{
			unsigned short r = SYSTEM_TO_BIG_ENDIAN_UINT16( *(unsigned short*)&val );
			return *(T*)&r;
		} else if (sizeof(T) == 4) {
			unsigned int r = SYSTEM_TO_BIG_ENDIAN_UINT32( *(unsigned int*)&val );
			return *(T*)&r;
		} else if (sizeof(T) == 8) {
			unsigned long long r = SYSTEM_TO_BIG_ENDIAN_UINT64( *(unsigned long long*)&val );
			return *(T*)&r;
		}
		return val;
	};
	template <typename T> inline T DKBigEndianToSystem(T val)
	{
		if (sizeof(T) == 2)	{
			unsigned short r = BIG_ENDIAN_TO_SYSTEM_UINT16( *(unsigned short*)&val );
			return *(T*)&r;
		} else if (sizeof(T) == 4) {
			unsigned int r = BIG_ENDIAN_TO_SYSTEM_UINT32( *(unsigned int*)&val );
			return *(T*)&r;
		} else if (sizeof(T) == 8) {
			unsigned long long r = BIG_ENDIAN_TO_SYSTEM_UINT64( *(unsigned long long*)&val );
			return *(T*)&r;
		}
		return val;
	};
	template <typename T> inline T DKSystemToLittleEndian(T val)
	{
		if (sizeof(T) == 2)	{
			unsigned short r = SYSTEM_TO_LITTLE_ENDIAN_UINT16( *(unsigned short*)&val );
			return *(T*)&r;
		} else if (sizeof(T) == 4) {
			unsigned int r = SYSTEM_TO_LITTLE_ENDIAN_UINT32( *(unsigned int*)&val );
			return *(T*)&r;
		} else if (sizeof(T) == 8) {
			unsigned long long r = SYSTEM_TO_LITTLE_ENDIAN_UINT64( *(unsigned long long*)&val );
			return *(T*)&r;
		}
		return val;
	};
	template <typename T> inline T DKLittleEndianToSystem(T val)
	{
		if (sizeof(T) == 2)	{
			unsigned short r = LITTLE_ENDIAN_TO_SYSTEM_UINT16( *(unsigned short*)&val );
			return *(T*)&r;
		} else if (sizeof(T) == 4) {
			unsigned int r = LITTLE_ENDIAN_TO_SYSTEM_UINT32( *(unsigned int*)&val );
			return *(T*)&r;
		} else if (sizeof(T) == 8) {
			unsigned long long r = LITTLE_ENDIAN_TO_SYSTEM_UINT64( *(unsigned long long*)&val );
			return *(T*)&r;
		}
		return val;
	};

	////////////////////////////////////////////////////////////////////////////////
	// byte order test.
	// using preprocessor macros at compile-time and validate in run-time.
	enum RTByteOrder
	{
		RTByteOrderUnknown,
		RTByteOrderBigEndian,
		RTByteOrderLittleEndian,
	};
	static inline RTByteOrder RuntimeEndianness(void)
	{
		union
		{
			unsigned int s;
			char n[4];
		} val = {(unsigned int)'RTBO'};
		if (val.n[0] == 'R' && val.n[1] == 'T' && val.n[2] == 'B' && val.n[3] == 'O')
			return RTByteOrderBigEndian;
		if (val.n[0] == 'O' && val.n[1] == 'B' && val.n[2] == 'T' && val.n[3] == 'R')
			return RTByteOrderLittleEndian;
		return RTByteOrderUnknown;
	}
	static inline bool VerifyRuntimeByteOrderMacro(void)
	{
#ifdef __BIG_ENDIAN__
		return RuntimeEndianness() == RTByteOrderBigEndian;
#endif
#ifdef __LITTLE_ENDIAN__
		return RuntimeEndianness() == RTByteOrderLittleEndian;
#endif
		return false;
	}
}

#define DEBUG_CHECK_RUNTIME_ENDIANNESS          DKASSERT_DESC_DEBUG(DKFoundation::VerifyRuntimeByteOrderMacro(), "System Byte-Order Mismatch!")


#endif	// #ifdef __cplusplus
