//
//  File: DKValue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypeTraits.h"
#include "DKObject.h"
#include "DKInvocation.h"

////////////////////////////////////////////////////////////////////////////////
// DKValue
//
// Binds variable or constant to DKInvocation object. (see DKInvocation.h)
// a variable or constant bounds, will become result of invocation.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename T> class DKInvocationValue : public DKInvocation<T>
	{
	public:
		DKInvocationValue(T v) : value(v) 
		{
		}
	protected:
		T Invoke(void) const
		{
			return value;
		}
	private:
		T value;
	};
	template <typename T> class DKInvocationValue<T*> : public DKInvocation<T*>
	{
	public:
		DKInvocationValue(T* v) : value(v) 
		{
		}
	protected:
		T* Invoke(void) const
		{
			return const_cast<T*>((const T*)value);
		}
	private:
		DKObject<T> value;
	};

	template <typename T> DKObject<DKInvocation<T>> DKValue(T value)
	{
		DKObject<DKInvocationValue<T>> invocation = DKOBJECT_NEW DKInvocationValue<T>(value);
		return invocation.template SafeCast<DKInvocation<T>>();
	}
}
