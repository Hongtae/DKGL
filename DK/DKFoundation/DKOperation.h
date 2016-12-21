//
//  File: DKOperation.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// @brief an operation class.
	/// DKOperation object provides encapsulation of any kind operation. 
	/// You can obtain DKOperation from a function or value.
	/// You can also subclass DKOperation.
	///
	/// You can perform operation with this object, but you cannot retrieve result.
	/// You can use DKInvocation to retrieve result.
	///
	/// @see DKThread, DKOperationQueue, DKEventLoop
	class DKOperation
	{
	public:
		DKOperation(void) {}
		virtual ~DKOperation(void) {}
		virtual void Perform(void) const = 0;
	};
}
