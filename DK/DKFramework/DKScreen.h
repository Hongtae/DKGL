//
//  File: DKScreen.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKWindow.h"
#include "DKPoint.h"
#include "DKSize.h"
#include "DKFrame.h"

////////////////////////////////////////////////////////////////////////////////
// DKScreen
// screen object, which render it's frames (DKFrame) to window surface.
// screen has one EventLoop internally, to process events, messages, operations
// asynchronous. (delivers event to target frame)
////////////////////////////////////////////////////////////////////////////////


namespace DKGL
{
	class DKGL_API DKScreen
	{
	public:
		DKScreen(void);
		~DKScreen(void);
	};
}
