//
//  File: DKActionController.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKModel.h"

////////////////////////////////////////////////////////////////////////////////
// DKActionController
// Abstract class, Action control node class inherited from DKModel.
// Useful for dynamics control of DKDynamicsWorld. (called by DKDynamicsWorld)
//
// Note:
//    Use DKAnimationController for individual node animation.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKActionController : public DKModel
	{
	public:
		DKActionController(void) : DKModel(TypeAction) {}
		virtual ~DKActionController(void) {}
		virtual void Update(double timeDelta, DKTimeTick tick) = 0;
	};
}
