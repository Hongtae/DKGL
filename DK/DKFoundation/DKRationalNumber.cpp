//
//  File: DKRationalNumber.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include "DKRationalNumber.h"

namespace DKFoundation::Private
{
    // Greatest common divisor (unsigned integer)
    template <typename T> inline T GCDUInt(T a, T b)
    {
        const T zero = static_cast<T>(0);

        while (true)
        {
            if (a == zero)
            return b;
            b %= a;
            if (b == zero)
            return a;
            a %= b;
        }
        return 1;
    }
    // Greatest common divisor (signed integer)
    template <typename T> inline T GCDSInt(T a, T b)
    {
        const T zero = static_cast<T>(0);
        const T r = GCDUInt(a, b);
        return (r < zero) ? -r : r;
    }
    // Least common multiple (unsigned integer)
    template <typename T> inline T LCMUInt(T a, T b)
    {
        const T zero = static_cast<T>(0);
        const T tmp = GCDUInt(a, b);

        return (tmp != zero) ? (a / tmp * b) : zero;
    }
    // Least common multiple (signed integer)
    template <typename T> inline T LCMSInt(T a, T b)
    {
        const T zero = static_cast<T>(0);
        const T r = LCMUInt(a, b);

        return (r < zero) ? -r : r;
    }
}

using namespace DKFoundation;

DKRationalNumber::DKRationalNumber()
	: numerator(0), denominator(1)
{
}

DKRationalNumber::DKRationalNumber(Integer n, Integer d)
	: numerator(n), denominator(d)
{
	Normalize();
}

bool DKRationalNumber::IsInfinity() const
{
	return numerator != 0 && denominator == 0;
}

bool DKRationalNumber::IsPositiveInfinity() const
{
	return numerator > 0 && denominator == 0;
}

bool DKRationalNumber::IsNegativeInfinity() const
{
	return numerator < 0 && denominator == 0;
}

bool DKRationalNumber::IsIndeterminate() const
{
	return numerator == 0 && denominator == 0;
}

bool DKRationalNumber::IsNumeric() const
{
	return denominator != 0;
}

DKRationalNumber::Integer DKRationalNumber::Numerator() const
{
	return numerator;
}

DKRationalNumber::Integer DKRationalNumber::Denominator() const
{
	return denominator;
}

DKRationalNumber DKRationalNumber::operator + (const DKRationalNumber& r) const
{
	DKRationalNumber ret(this->numerator, this->denominator);
	ret += r;
	return ret;
}

DKRationalNumber DKRationalNumber::operator - (const DKRationalNumber& r) const
{
	DKRationalNumber ret(this->numerator, this->denominator);
	ret -= r;
	return ret;
}

DKRationalNumber DKRationalNumber::operator * (const DKRationalNumber& r) const
{
	DKRationalNumber ret(this->numerator, this->denominator);
	ret *= r;
	return ret;
}

DKRationalNumber DKRationalNumber::operator / (const DKRationalNumber& r) const
{
	DKRationalNumber ret(this->numerator, this->denominator);
	ret /= r;
	return ret;
}

DKRationalNumber& DKRationalNumber::operator += (const DKRationalNumber& r)
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	if (this->denominator != 0 && r.denominator != 0)
	{
		Integer num = r.numerator;
		Integer den = r.denominator;

		Integer g = Private::GCDSInt<Integer>(this->denominator, den);
		this->denominator /= g;
		this->numerator = this->numerator * (den / g) + num * this->denominator;
		g = Private::GCDSInt<Integer>(this->numerator, g);
		this->numerator /= g;
		this->denominator *= den / g;
	}
	else if (this->denominator != 0)
	{
		this->numerator = r.numerator;
		this->denominator = 0;
	}
	else if (r.denominator == 0)		// both denominators are zero.
	{
		if (this->numerator > 0 && r.numerator > 0)
			this->numerator = 1;		// +infinity
		else if (this->numerator < 0 && r.numerator < 0)
			this->numerator = -1;		// -infinity
		else
			this->numerator = 0;		// indeterminate
	}
	DKASSERT_DEBUG(this->denominator >= 0);
	return *this;
}

DKRationalNumber& DKRationalNumber::operator -= (const DKRationalNumber& r)
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	if (this->denominator != 0 && r.denominator != 0)
	{
		Integer num = r.numerator;
		Integer den = r.denominator;

		Integer g = Private::GCDSInt<Integer>(this->denominator, den);
		this->denominator /= g;
		this->numerator = this->numerator * (den / g) - num * this->denominator;
		g = Private::GCDSInt<Integer>(this->numerator, g);
		this->numerator /= g;
		this->denominator *= den / g;
	}
	else if (this->denominator != 0)
	{
		this->numerator = -r.numerator;
		this->denominator = 0;
	}
	else if (r.denominator == 0) // both denominators are 0.
	{
		if (this->numerator > 0 && r.numerator < 0)
			this->numerator = 1;		// +infinity
		else if (this->numerator < 0 && r.numerator > 0)
			this->numerator = -1;		// -infinity
		else
			this->numerator = 0;		// indeterminate
	}
	DKASSERT_DEBUG(this->denominator >= 0);
	return *this;
}

DKRationalNumber& DKRationalNumber::operator *= (const DKRationalNumber& r)
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	if (this->denominator != 0 && r.denominator != 0)
	{
		Integer num = r.numerator;
		Integer den = r.denominator;

		Integer g1 = Private::GCDSInt<Integer>(this->numerator, den);
		Integer g2 = Private::GCDSInt<Integer>(num, this->denominator);

		this->numerator = (this->numerator / g1) * (num / g2);
		this->denominator = (this->denominator / g2) * (den / g1);

		DKASSERT_DEBUG(this->denominator >= 0);
	}
	else
	{
		Integer n = this->numerator * r.numerator;
		if (n > 0)
			this->numerator = 1;
		else if (n < 0)
			this->numerator = -1;
		else
			this->numerator = 0;
		this->denominator = 0;
	}
	return *this;
}

DKRationalNumber& DKRationalNumber::operator /= (const DKRationalNumber& r)
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	if (this->denominator != 0 && r.denominator != 0)
	{
		Integer num = r.numerator;
		Integer den = r.denominator;

		if (num == 0)		// NaN, inf
		{
			if (this->numerator > 0)
				this->numerator = 1;		// +inf
			else if (this->numerator < 0)
				this->numerator = -1;		// -inf
			else
				this->numerator = 0;		// nan
			this->denominator = 0;			
		}
		else if (this->numerator == 0)
		{
		}
		else
		{
			Integer g1 = Private::GCDSInt<Integer>(this->numerator, num);
			Integer g2 = Private::GCDSInt<Integer>(den, this->denominator);

			this->numerator = (this->numerator/g1) * (den/g2);
			this->denominator = (this->denominator/g2) * (num/g1);

			if (this->denominator < 0)
			{
				this->numerator = -this->numerator;
				this->denominator = -this->denominator;
			}
		}
		DKASSERT_DEBUG(this->denominator >= 0);
	}
	else
	{
		int n1 = (this->numerator > 0) ? 1 : ((this->numerator < 0) ? -1 : 0);
		int n2 = (r.numerator > 0) ? 1 : ((r.numerator < 0) ? -1 : 0);
		int d1 = (this->denominator > 0) ? 1 : 0;
		int d2 = (r.denominator > 0) ? 1 : 0;

		int num = n1 * d2;
		int den = d1 * n2;

		if (n2 < 0)
			num = -num;
		if (den != 0)
			den = 1;

		this->numerator = num;
		this->denominator = den;

		DKASSERT_DEBUG(-1 <= this->numerator && this->numerator <= 1);
		DKASSERT_DEBUG( 0 <= this->denominator && this->denominator <= 1);
	}
	return *this;
}

DKRationalNumber& DKRationalNumber::operator = (const DKRationalNumber& r)
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	this->numerator = r.numerator;
	this->denominator = r.denominator;
	return *this;
}

bool DKRationalNumber::operator == (const DKRationalNumber& r) const
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	if (this->denominator > 0 && r.denominator > 0)
		return this->numerator == r.numerator && this->denominator == r.denominator;
	return false;
}

bool DKRationalNumber::operator != (const DKRationalNumber& r) const
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	if (this->denominator > 0 && r.denominator > 0)
		return this->numerator != r.numerator || this->denominator != r.denominator;
	return true;
}

bool DKRationalNumber::operator < (const DKRationalNumber& r) const
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	// implemented base on:
	// http://www.boost.org/doc/libs/1_50_0/boost/rational.hpp

	if (this->denominator != 0 && r.denominator != 0)
	{
		struct
		{
			Integer n, d, q, r;
		}	ts = {this->numerator, this->denominator, this->numerator / this->denominator, this->numerator % this->denominator },
			rs = {r.numerator, r.denominator, r.numerator / r.denominator, r.numerator % r.denominator };

		unsigned int reverse = 0U;

		while (ts.r < 0)	{ts.r += ts.d; --ts.q;}
		while (rs.r < 0)	{rs.r += rs.d; --rs.q;}

		while (true)
		{
			if (ts.q != rs.q)
			{
				return reverse ? ts.q > rs.q : ts.q < rs.q;
			}

			reverse ^= 1U;

			if (ts.r == 0 || rs.r == 0)
				break;

			ts.n = ts.d;         ts.d = ts.r;
			ts.q = ts.n / ts.d;  ts.r = ts.n % ts.d;
			rs.n = rs.d;         rs.d = rs.r;
			rs.q = rs.n / rs.d;  rs.r = rs.n % rs.d;
		}

		if (ts.r == rs.r)
		{
			return false;
		}
		else
		{
			return (ts.r != 0) != static_cast<bool>(reverse);
		}
	}
	else if (this->denominator != 0)	// r.denominator == 0
	{
		if (r.numerator > 0)
			return true;				// r is +inf
	}
	else if (r.denominator != 0)		// this->denominator == 0
	{
		if (this->numerator < 0)
			return true;				// this is -inf
	}
	else	// all denominators == 0
	{
		if (this->numerator != 0 && r.numerator != 0)
			return this->numerator < r.numerator;		// -inf < +inf
	}
	return false;
}

bool DKRationalNumber::operator <= (const DKRationalNumber& r) const
{
	DKASSERT_DEBUG(this->denominator >= 0);
	DKASSERT_DEBUG(r.denominator >= 0);

	if (this->denominator != 0 && r.denominator != 0)
	{
		if (this->numerator == r.numerator && this->denominator == r.denominator)
			return true;

		return this->operator < (r);
	}
	else if (this->denominator != 0)	// r.denominator == 0
	{
		if (r.numerator > 0)
			return true;				// r is +inf
	}
	else if (r.denominator != 0)		// this->denominator == 0
	{
		if (this->numerator < 0)
			return true;				// this is -inf
	}
	else	// all denominators == 0
	{
		if (this->numerator != 0 && r.numerator != 0)
			return this->numerator < r.numerator;		// -inf < +inf
	}
	return false;
}

bool DKRationalNumber::operator > (const DKRationalNumber& r) const
{
	if (this->operator!=(r))
		return r.operator < (*this);
	return false;
}

bool DKRationalNumber::operator >= (const DKRationalNumber& r) const
{
	return r.operator <= (*this);
}

DKRationalNumber& DKRationalNumber::Inverse()
{
	DKASSERT_DEBUG(this->denominator >= 0);

	Integer t = this->numerator;
	this->numerator = this->denominator;
	this->denominator = t;
	return *this;
}

double DKRationalNumber::RealNumber() const
{
	DKASSERT_DEBUG(this->denominator >= 0);

	return static_cast<double>(Numerator()) / static_cast<double>(Denominator());
}

void DKRationalNumber::Normalize()
{
	if (this->denominator == 0)
	{
		if (this->numerator > 0)
			this->numerator = 1;		// +inf
		else if (this->numerator < 0)
			this->numerator = -1;		// -inf
		// else NaN
	}
	else if (this->numerator == 0)
	{
		this->denominator = 1;
	}
	else
	{
		if (this->denominator < 0)
		{
			this->numerator = -this->numerator;
			this->denominator = -this->denominator;
		}

		Integer g = Private::GCDSInt<Integer>(this->numerator, this->denominator);

		this->numerator /= g;
		this->denominator /= g;

		DKASSERT_DEBUG(this->numerator != 0);
		DKASSERT_DEBUG(this->denominator > 0);
	}
	DKASSERT_DEBUG(this->denominator >= 0);
}
