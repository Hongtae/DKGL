//
//  File: DKOperation.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKOperation
// an operation class, abstract class.
//
// You can perform operation with this object, but you cannot retrieve result.
// You can use DKInvocation to retrieve result.
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKOperation
	{
	public:
		DKOperation(void) {}
		virtual ~DKOperation(void) {}
		virtual void Perform(void) const = 0;
	};
}
