//
//  File: DKColor.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKVector4.h"

////////////////////////////////////////////////////////////////////////////////
// DKColor
// color object contains a, r, g, b floats. (transferred to GPU directly)
// object can create with 32bit integer also.
////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 4)
namespace DKFramework
{
	class DKColor
	{
	public:
		union RGBA32  // 32bit int format (RGBA order).
		{
			struct {
				uint8_t r, g, b, a;
			};
			uint8_t bytes[4];
			uint32_t value;
		};
		union ARGB32  // 32bit int format (ARGB order).
		{
			struct {
				uint8_t a, r, g, b;
			};
			uint8_t bytes[4];
			uint32_t value;
		};

		DKColor(void)
			: r(0.0f), g(0.0f), b(0.0f), a(1.0f)
		{
		}
		DKColor(const DKColor& c)
			: r(c.r), g(c.g), b(c.b), a(c.a)
		{
		}
		DKColor(float red, float green, float blue, float alpha = 1.0f)
			: r(red), g(green), b(blue), a(alpha)
		{
		}
		DKColor(RGBA32 rgba)
			: r(static_cast<float>(rgba.r) / 255.0f)
			, g(static_cast<float>(rgba.g) / 255.0f)
			, b(static_cast<float>(rgba.b) / 255.0f)
			, a(static_cast<float>(rgba.a) / 255.0f)
		{
		}
		DKColor(ARGB32 argb)
			: r(static_cast<float>(argb.r) / 255.0f)
			, g(static_cast<float>(argb.g) / 255.0f)
			, b(static_cast<float>(argb.b) / 255.0f)
			, a(static_cast<float>(argb.a) / 255.0f)
		{
		}
		explicit DKColor(const DKVector3& v)
			: r(v.x), g(v.y), b(v.z), a(1.0)
		{
		}
		explicit DKColor(const DKVector4& v)
			: r(v.x), g(v.y), b(v.z), a(v.w)
		{
		}

		RGBA32 RGBA32Value(void) const
		{
			RGBA32 val = {
				static_cast<uint8_t>(Clamp<int>(r * 255.0f, 0, 0xff)),
				static_cast<uint8_t>(Clamp<int>(g * 255.0f, 0, 0xff)),
				static_cast<uint8_t>(Clamp<int>(b * 255.0f, 0, 0xff)),
				static_cast<uint8_t>(Clamp<int>(a * 255.0f, 0, 0xff)),
			};
			return val;
		}

		ARGB32 ARGB32Value(void) const
		{
			ARGB32 val = {
				static_cast<uint8_t>(Clamp<int>(a * 255.0f, 0, 0xff)),
				static_cast<uint8_t>(Clamp<int>(r * 255.0f, 0, 0xff)),
				static_cast<uint8_t>(Clamp<int>(g * 255.0f, 0, 0xff)),
				static_cast<uint8_t>(Clamp<int>(b * 255.0f, 0, 0xff)),
			};
			return val;
		}

		bool operator == (const DKColor& c)
		{
			return r == c.r && g == c.g && b == c.b && a == c.a;
		}
		bool operator != (const DKColor& c)
		{
			return r != c.r || g != c.g || b != c.b || a != c.a;
		}
		DKColor& operator = (const DKColor& c)
		{
			r = c.r;
			g = c.g;
			b = c.b;
			a = c.a;
			return *this;
		}
		DKColor& operator += (const DKColor& c)
		{
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			return *this;
		}
		DKColor& operator += (float f)
		{
			r += f;
			g += f;
			b += f;
			a += f;
			return *this;
		}
		DKColor& operator -= (const DKColor& c)
		{
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			return *this;
		}
		DKColor& operator -= (float f)
		{
			r -= f;
			g -= f;
			b -= f;
			a -= f;
			return *this;
		}
		DKColor& operator *= (const DKColor& c)
		{
			r *= c.r;
			g *= c.g;
			b *= c.b;
			a *= c.a;
			return *this;
		}
		DKColor& operator *= (float f)
		{
			r *= f;
			g *= f;
			b *= f;
			a *= f;
			return *this;
		}
		DKColor& operator /= (const DKColor& c)
		{
			r /= c.r;
			g /= c.g;
			b /= c.b;
			a /= c.a;
			return *this;
		}
		DKColor& operator /= (float f)
		{
			float inv = 1.0f / f;
			r *= inv;
			g *= inv;
			b *= inv;
			a *= inv;
			return *this;
		}

		DKColor operator + (const DKColor& c) const
		{
			return DKColor(r + c.r, g + c.g, b + c.b, a + c.a);
		}
		DKColor operator + (float f) const
		{
			return DKColor(r + f, g + f, b + f, a + f);
		}
		DKColor operator - (const DKColor& c) const
		{
			return DKColor(r - c.r, g - c.g, b - c.b, a - c.a);
		}
		DKColor operator - (float f) const
		{
			return DKColor(r - f, g - f, b - f, a - f);
		}
		DKColor operator * (const DKColor& c) const
		{
			return DKColor(r * c.r, g * c.g, b * c.b, a * c.a);
		}
		DKColor operator * (float f) const
		{
			return DKColor(r * f, g * f, b * f, a * f);
		}
		DKColor operator / (const DKColor& c) const
		{
			return DKColor(r / c.r, g / c.g, b / c.b, a / c.a);
		}
		DKColor operator / (float f) const
		{
			float inv = 1.0f / f;
			return DKColor(r * inv, g * inv, b * inv, a * inv);
		}
		DKVector3 Vector3(void) const
		{
			return DKVector3(r, g, b);
		}
		DKVector4 Vector4(void) const
		{
			return DKVector4(r, g, b, a);
		}

		union
		{
			struct {
				float r, g, b, a;
			};
			float val[4];
		};
	};
}
#pragma pack(pop)
