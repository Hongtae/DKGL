//
//  File: DKStaticPlaneShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConcaveShape.h"

namespace DKFramework
{
	/// @brief a collision shape for static plane
	class DKGL_API DKStaticPlaneShape : public DKConcaveShape
	{
	public:
		DKStaticPlaneShape(const DKVector3& planeNormal, float planeConstant);
		~DKStaticPlaneShape(void);

		DKVector3 PlaneNormal(void) const;
		float PlaneConstant(void) const;
	};
}
