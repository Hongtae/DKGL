//
//  File: DKPlane.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

#pragma pack(push, 4)
namespace DKFramework
{
	class DKVector3;
	class DKVector4;
	class DKLine;
	/// @brief a plane object. useful to calculate collision detection.
	class DKGL_API DKPlane
	{
	public:
		DKPlane(void);  ///< init (a,b,c,d) = (0, 0, 0, 0)
		DKPlane(const DKVector3& v1, const DKVector3& v2, const DKVector3& v3); ///< plane from triangle
		DKPlane(const DKVector3& n, const DKVector3& p);						///< plane from normal, point
		~DKPlane(void);

		/// distance between one point to plane. (dot < 0 located behind the plane)
		float Dot(const DKVector3& v) const;
		float Dot(const DKVector4& v) const;

		DKVector3 Normal(void) const;

		bool RayTest(const DKLine& ray, DKVector3* hitPoint = NULL) const;

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
#pragma pack(pop)
