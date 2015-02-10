//
//  File: DKActionController.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKModel.h"

////////////////////////////////////////////////////////////////////////////////
// DKActionController
// Abstract class, Action control node class inherited from DKModel.
// Useful for dynamics control of DKDynamicsScene. (called by DKDynamicsScene)
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
		virtual void Update(double timeDelta, DKFoundation::DKTimeTick tick) = 0;
	};
}
