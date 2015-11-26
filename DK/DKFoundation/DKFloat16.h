//
//  File: DKFloat16.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKFloat16
// Half precision floating point type.
//
// binary16 layout (IEEE 754-2008)
//  +-------+----------+---------------------+
//  | sign  | exponent | fraction (mantissa) |
//  +-------+----------+---------------------+
//  | 1 bit | 5 bit    | 10 bit              |
//  +-------+----------+---------------------+
//
////////////////////////////////////////////////////////////////////////////////


#pragma pack(push, 2)
namespace DKFoundation
{
	class DKGL_API DKFloat16
	{
	public:
		DKFloat16(void);
		DKFloat16(const DKFloat16&);
		explicit DKFloat16(float);

		operator float(void) const;
		DKFloat16& operator = (const DKFloat16&);

		DKFloat16 Abs(void) const;
		bool IsInfinity(void) const;
		bool IsPositiveInfinity(void) const;
		bool IsNegativeInfinity(void) const;
		bool IsNumeric(void) const;  // false if NaN
		bool IsPositive(void) const;

		static const DKFloat16 zero;			// 0.0 (0 for positive, 0x8000 for negative)
		static const DKFloat16 max;				// maximum positive value (65504.0)
		static const DKFloat16 min;				// minimum positive normal (2^−14)
		static const DKFloat16 maxSubnormal;	// minimum positive subnormal (2^−14 - 2^−24)
		static const DKFloat16 minSubnormal;	// minimum positive subnormal (2^−24)
		static const DKFloat16 posInfinity;		// +Inf (0x7c00)
		static const DKFloat16 negInfinity;		// -Inf (0xfc00)

	private:
		uint16_t binary16; // packed 16bit float
	};

	static_assert(sizeof(DKFloat16) == 2, "float16 should be 2 bytes!");

	inline DKFloat16 abs(DKFloat16 f) { return f.Abs(); }
}
#pragma pack(pop)
