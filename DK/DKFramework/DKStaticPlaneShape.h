//
//  File: DKStaticPlaneShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKConcaveShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKStaticPlaneShape
// a collision shape for static plane
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKStaticPlaneShape : public DKConcaveShape
	{
	public:
		DKStaticPlaneShape(const DKVector3& planeNormal, float planeConstant);
		~DKStaticPlaneShape(void);

		DKVector3 PlaneNormal(void) const;
		float PlaneConstant(void) const;
	};
}
