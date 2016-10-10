//
//  File: DKFloat16.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.


#include "DKFloat16.h"
using namespace DKGL;

static inline DKFloat16 UInt16ToFloat16(uint16_t val)
{
	return reinterpret_cast<DKFloat16&>(val);
}

const DKFloat16 DKFloat16::zero = UInt16ToFloat16(0x0U);
const DKFloat16 DKFloat16::max = UInt16ToFloat16(0x7bffU);
const DKFloat16 DKFloat16::min = UInt16ToFloat16(0x400U);
const DKFloat16 DKFloat16::maxSubnormal = UInt16ToFloat16(0x3ffU);
const DKFloat16 DKFloat16::minSubnormal = UInt16ToFloat16(0x1U);
const DKFloat16 DKFloat16::posInfinity = UInt16ToFloat16(0x7c00U);
const DKFloat16 DKFloat16::negInfinity = UInt16ToFloat16(0xfc00U);


DKFloat16::DKFloat16(void)
	: binary16(0U)
{
}

DKFloat16::DKFloat16(const DKFloat16& f)
	: binary16(f.binary16)
{
}

DKFloat16::DKFloat16(float val)
{
	uint32_t n = reinterpret_cast<uint32_t&>(val);

	uint16_t sign = uint16_t((n >> 16) & 0x00008000U);
	uint32_t exponent = n & 0x7f800000U;
	uint32_t mantissa = n & 0x007fffffU;

	if (exponent >= 0x47800000U) // overflow / NaN
	{
		if (exponent == 0x7f800000U && mantissa) // NaN
		{
			mantissa >>= 13;
			if (mantissa == 0) mantissa = 1;

			binary16 = sign | uint16_t(0x7c00U) | uint16_t(mantissa);
		}
		else // Inf / Overflow
		{
			binary16 = sign | uint16_t(0x7c00U);
		}
	}
	else if (exponent <= 0x38000000U) // underflow
	{
		if (exponent < 0x33000000U)
		{
			binary16 = sign;
		}
		else
		{
			exponent >>= 23;
			mantissa |= 0x00800000U;
			mantissa >>= (113 - exponent);
			mantissa |= 0x00001000U;
			binary16 = sign | uint16_t(mantissa >> 13);
		}
	}
	else // regular
	{
		exponent = exponent - 0x38000000U;
		mantissa |= 0x00001000U;
		binary16 = sign | uint16_t(exponent >> 13) | uint16_t(mantissa >> 13);
	}
}

DKFloat16::operator float(void) const
{
	uint32_t sign = (binary16 >> 15) & 0x1U;
	uint32_t exponent = (binary16 >> 10) & 0x1fU;
	uint32_t mantissa = binary16 & 0x3ffU;

	if (exponent == 0)
	{
		if (mantissa)	// subnormal
		{
			exponent = 0x70U;
			mantissa <<= 1;
			while ((mantissa & 0x400U) == 0)
			{
				mantissa <<= 1;
				exponent -= 1;
			}
			mantissa &= 0x3ff;	// Clamp to 10 bits.
			mantissa = mantissa << 13;
		}
	}
	else if (exponent == 0x1fU)	// NaN or Inf
	{
		exponent = 0xffU;
		if (mantissa)	// NaN
			mantissa = mantissa << 13 | 0x1fffU;
	}
	else // Normalized
	{
		exponent = exponent + 0x70;
		mantissa = mantissa << 13;
	}

	union { float f; uint32_t n; };
	n = (sign << 31) | (exponent << 23) | mantissa;
	return f;
}

DKFloat16& DKFloat16::operator = (const DKFloat16& v)
{
	binary16 = v.binary16;
	return *this;
}

DKFloat16 DKFloat16::Abs(void) const
{
	return static_cast<DKFloat16>(uint16_t(binary16 & 0x7fffU));
}

bool DKFloat16::IsInfinity(void) const
{
	uint32_t exponent = (binary16 >> 10) & 0x1fU;
	uint32_t mantissa = binary16 & 0x3ffU;
	return exponent == 0x1fU && mantissa == 0U;
}

bool DKFloat16::IsPositiveInfinity(void) const
{
	if (IsInfinity())
		return IsPositive();
	return false;
}

bool DKFloat16::IsNegativeInfinity(void) const
{
	if (IsInfinity())
		return !IsPositive();
	return false;
}

bool DKFloat16::IsNumeric(void) const
{
	uint32_t exponent = (binary16 >> 10) & 0x1fU;
	if (exponent == 0x1fU)
	{
		uint32_t mantissa = binary16 & 0x3ffU;
		if (mantissa)
			return false;
	}
	return true;
}

bool DKFloat16::IsSubnormalNumber(void) const
{
	uint32_t exponent = (binary16 >> 10) & 0x1fU;
	if (exponent == 0U)
	{
		uint32_t mantissa = binary16 & 0x3ffU;
		if (mantissa)
			return true;
	}
	return false;
}

bool DKFloat16::IsPositive(void) const
{
	return ((binary16 >> 15) & 0x1U) == 0;
}

bool DKFloat16::IsZero(void) const
{
	return (binary16 & 0x7fffU) == 0;
}

int DKFloat16::Compare(const DKFloat16& rhs) const
{
	int sign1 = this->binary16 & 0x8000U;
	int sign2 = rhs.binary16 & 0x8000U;
	int v1 = this->binary16 & 0x7fffU;
	int v2 = rhs.binary16 & 0x7fffU;

	if (sign1)
		v1 = -v1;
	if (sign2)
		v2 = -v2;

	return v1 - v2;
}
