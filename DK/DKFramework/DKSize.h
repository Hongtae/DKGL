//
//  File: DKSize.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector2.h"

#pragma pack(push, 4)
namespace DKFramework
{
	/// @brief
	/// a size object, which has width, height. It can be used measured unit size
	/// of 2 dimensional logical coordinates.
	class DKGL_API DKSize
	{
	public:
		DKSize(void)
			: width(0), height(0)
		{
		}
		DKSize(const DKVector2& v)
			: width(v.x), height(v.y)
		{
		}
		DKSize(float w, float h)
			: width(w), height(h)
		{
		}
		bool operator != (const DKSize& s) const
		{
			return width != s.width || height != s.height;
		}
		bool operator == (const DKSize& s) const
		{
			return width == s.width && height == s.height;
		}
		DKSize operator - (const DKSize& s) const
		{
			return DKSize(width - s.width, height - s.height);
		}
		DKSize operator - (float s) const
		{
			return DKSize(width - s, height - s);
		}
		DKSize operator + (const DKSize& s) const
		{
			return DKSize(width + s.width, height + s.height);
		}
		DKSize operator + (float s) const
		{
			return DKSize(width + s, height + s);
		}
		DKSize operator * (const DKSize& s) const
		{
			return DKSize(width * s.width, height * s.height);
		}
		DKSize operator * (float s) const
		{
			return DKSize(width * s, height * s);
		}
		DKSize operator / (const DKSize& s) const
		{
			return DKSize(width / s.width, height / s.height);
		}
		DKSize operator / (float s) const
		{
			return DKSize(width / s, height / s);
		}
		DKSize& operator -= (const DKSize& s)
		{
			width -= s.width;
			height -= s.height;
			return *this;
		}
		DKSize& operator -= (float s)
		{
			width -= s;
			height -= s;
			return *this;
		}
		DKSize& operator += (const DKSize& s)
		{
			width += s.width;
			height += s.height;
			return *this;
		}
		DKSize& operator += (float s)
		{
			width += s;
			height += s;
			return *this;
		}
		DKSize& operator *= (const DKSize& s)
		{
			width *= s.width;
			height *= s.height;
			return *this;
		}
		DKSize& operator *= (float s)
		{
			width *= s;
			height *= s;
			return *this;
		}
		DKSize& operator /= (const DKSize& s)
		{
			width /= s.width;
			height /= s.height;
			return *this;
		}
		DKSize& operator /= (float s)
		{
			width /= s;
			height /= s;
			return *this;
		}
		DKVector2 Vector(void) const
		{
			return DKVector2(width, height);
		}
		
		float width;
		float height;

		static const DKSize zero;
	};
}
#pragma pack(pop)
