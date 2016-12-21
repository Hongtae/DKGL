//
//  File: DKInvocation.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypes.h"
#include "DKOperation.h"

namespace DKFoundation
{
	/// @brief Interface class for encapsulating an action and collecting result
	/// You can use this class as DKOperation type.
	/// and you can query result (type T) with this Invocation object.
	/// @tparam T result type
	template <typename T> struct DKInvocation : public DKOperation
	{
		virtual ~DKInvocation(void) {}

		void Perform(void) const {Invoke();}		
		virtual T Invoke(void) const = 0;

		/// test type T is convertible to U
		template <typename U> constexpr static bool CanAcceptResultTypeAs(void)
		{
			return DKTypeConversionTest<T, U>();
		};
	};
}
