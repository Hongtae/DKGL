//
//  File: DKFrame.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKMatrix3.h"
#include "DKPoint.h"
#include "DKSize.h"
#include "DKRect.h"
#include "DKColor.h"

namespace DKFramework
{
	/// @brief
	/// a frame class, used by screen for drawing behaviors.
	/// @details
	/// frame objects can be tree-structured as children and parent. But one screen
	/// can have one root-frame only.
	/// You need subclass of this, and override event handlers and drawing functions.
	///
	/// A frame can capture keyboard or mouse exclusively, then no other frames will
	/// not receive event. some events require to be captured exclusively.
	///
	/// Every input events (keyboard, mouse) processed by screen before propagates to
	/// sub frames. All events can be intercepted by frame's ancestors.
	///
	/// A frame can have its own coordinates space as local content transform, it
	/// does not need to be pixel space and you can have desired aspect ratio.
	///
	/// default coordinates origin(0,0) is top-left.
	///
	/// You can use frame as off-screen, then frame does not need to be structured
	/// hierarchically. (off-screen frame can not receive non-captured events.)
	/// Using DKRenderTarget or DKRenderer instead of off-screen frame, is
	/// recommended if you don't need frame's features. (on/off screen with events)
	///
	/// @note
	///    frame hierarchical update order is, parent -> child.
	///    - parent can pass values to child
	///    frame hierarchical drawing order is, childen -> parent.
	///    - parent can use children surface(textures) on render.
	///
	///    frame must be loaded by calling Load() with screen object before use.
	class DKGL_API DKFrame
	{
	public:
		DKFrame(void);
		virtual ~DKFrame(void);
	};
}
