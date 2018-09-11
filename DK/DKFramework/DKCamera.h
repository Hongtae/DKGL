//
//  File: DKCamera.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKPlane.h"
#include "DKSphere.h"

namespace DKFramework
{
	/**
	 @brief A 3D camera class.
	
	 @note
	    +z is inner direction of actual frustum, (-1:front, +1:back)
	    but this class set up -z is inner. (right handed)
	    coordinate system will be converted as right-handed after
	    transform applied. CCW (counter-clock-wise) is front-face.
	
	 @verbatim
	    coordinates transformed as below:
	
	          +Y
	           |
	           |
	           |_______ +X
	           /
	          /
	         /
	        +Z 
	
	 @endverbatim
	 */
	class DKGL_API DKCamera
	{
	public:
		DKCamera();
		~DKCamera();

		// view
		void SetView(const DKVector3& pos, const DKVector3& dir, const DKVector3& up);
		DKVector3 ViewPosition() const;
		DKVector3 ViewDirection() const;
		DKVector3 ViewUp() const;

		// projection
		void SetPerspective(float fov, float aspect, float nearDistance, float farDistance);
		void SetOrthographic(float width, float height, float nearDistance, float farDistance);
		bool IsPerspective() const;
		bool IsOrthographic() const;

		// frustum intersection test
		bool IsPointInside(const DKVector3& point) const;
		bool IsSphereInside(const DKVector3& center, float radius) const;
		bool IsSphereInside(const DKSphere& s) const;

		const DKPlane& TopFrustumPlane() const			{return frustumTop;}
		const DKPlane& BottomFrustumPlane() const		{return frustumBottom;}
		const DKPlane& LeftFrustumPlane() const			{return frustumLeft;}
		const DKPlane& RightFrustumPlane() const		{return frustumRight;}
		const DKPlane& NearFrustumPlane() const			{return frustumNear;}
		const DKPlane& FarFrustumPlane() const			{return frustumFar;}

		// matrix
		void SetView(const DKMatrix4& m);
		void SetProjection(const DKMatrix4& m);
		void SetViewProjection(const DKMatrix4& view, const DKMatrix4& proj);
		const DKMatrix4& ViewMatrix() const				{return viewMatrix;}
		const DKMatrix4& ProjectionMatrix() const		{return projectionMatrix;}
		DKMatrix4 ViewProjectionMatrix() const			{return viewMatrix * projectionMatrix;}

	private:
		void UpdateFrustum();

		DKPlane	frustumNear;
		DKPlane	frustumFar;
		DKPlane	frustumLeft;
		DKPlane	frustumRight;
		DKPlane	frustumTop;
		DKPlane frustumBottom;

		DKMatrix4 viewMatrix;
		DKMatrix4 projectionMatrix;
	};
}
