//
//  File: DKLine.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"

#pragma pack(push, 4)
namespace DKFramework
{
	/// @brief Line segment object.
	class DKGL_API DKLine
	{
	public:
		DKLine();
		DKLine(const DKVector3& b, const DKVector3& e);

		DKLine operator * (const DKMatrix4& m) const;
		DKLine operator * (const DKQuaternion& q) const;

		DKVector3 Direction() const;
		float Length() const;
		float LengthSq() const;
		bool IsPointInside(const DKVector3& pos) const;

		bool RayTest(const DKLine& ray, DKVector3* hitPoint = NULL, float epsilon = 0.000001f) const;

		/// compute shortest line segment between two line segments.
		/// result line's begin-point located on line1, end-point located on line2.
		static DKLine LineBetween(const DKLine& line1, const DKLine& line2, float epsilon = 0.000001f);

		DKVector3	begin;
		DKVector3	end;
	};
}
#pragma pack(pop)
