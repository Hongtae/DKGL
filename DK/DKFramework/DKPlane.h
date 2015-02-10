//
//  File: DKPlane.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKPlane
// a plane object. useful to calculate collision detection.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKVector3;
	class DKVector4;
	class DKLine;
	class DKLIB_API DKPlane
	{
	public:
		DKPlane(void);  // 0, 0, 0, 0
		DKPlane(const DKVector3& v1, const DKVector3& v2, const DKVector3& v3); // plane from triangle
		DKPlane(const DKVector3& n, const DKVector3& p);						// plane from normal, point
		~DKPlane(void);

		// distance between one point to plane. (dot < 0 located behind the plane)
		float Dot(const DKVector3& v) const;
		float Dot(const DKVector4& v) const;

		DKVector3 Normal(void) const;

		bool Intersect(const DKLine& line, DKVector3* p = NULL) const;

		union
		{
			struct
			{
				float a, b, c, d;
			};
			float val[4];
		};
	};
}


