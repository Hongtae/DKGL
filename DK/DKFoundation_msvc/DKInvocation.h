//
//  File: DKInvocation.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypes.h"
#include "DKOperation.h"

////////////////////////////////////////////////////////////////////////////////
// DKInvocation<T>
// You can use this class as DKOperation type.
// and you can query result (type T) with this Invocation object.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename T> struct DKInvocation : public DKOperation
	{
		virtual ~DKInvocation(void) {}

		void Perform(void) const {Invoke();}		
		virtual T Invoke(void) const = 0;

		// test type T is convertible to U
		template <typename U> static bool CanAcceptResultTypeAs(void)
		{
			return DKTypeConversionTest<T, U>::Result;
		};
	};
}
