//
//  File: DKRationalNumber.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// @brief A rational number class.
	/// Use this class if you need something calculated with high-precision.
	///
	/// To convert real-number: numerator / denominator. (call RealNumber())
	class DKGL_API DKRationalNumber
	{
	public:
		typedef int64_t Integer;

		DKRationalNumber();
		DKRationalNumber(Integer num, Integer den = 1);

		bool IsInfinity() const;
		bool IsPositiveInfinity() const;
		bool IsNegativeInfinity() const;
		bool IsIndeterminate() const;
		bool IsNumeric() const;

		Integer Numerator() const;
		Integer Denominator() const;

		DKRationalNumber operator + (const DKRationalNumber&) const;
		DKRationalNumber operator - (const DKRationalNumber&) const;
		DKRationalNumber operator * (const DKRationalNumber&) const;
		DKRationalNumber operator / (const DKRationalNumber&) const;
		DKRationalNumber& operator += (const DKRationalNumber&);
		DKRationalNumber& operator -= (const DKRationalNumber&);
		DKRationalNumber& operator *= (const DKRationalNumber&);
		DKRationalNumber& operator /= (const DKRationalNumber&);

		DKRationalNumber& operator = (const DKRationalNumber&);

		bool operator == (const DKRationalNumber&) const;
		bool operator != (const DKRationalNumber&) const;
		bool operator < (const DKRationalNumber&) const;
		bool operator <= (const DKRationalNumber&) const;
		bool operator > (const DKRationalNumber&) const;
		bool operator >= (const DKRationalNumber&) const;

		DKRationalNumber& Inverse();
		double RealNumber() const;

	private:
		void Normalize();
		Integer numerator;
		Integer denominator;
	};
}
