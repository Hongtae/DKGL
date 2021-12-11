//
//  File: DKAtomicNumber64.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// @brief A number object which can be increased or decreased atomically.
	///
	/// This class does not provide any numeric operators. (except assign)
	class DKGL_API DKAtomicNumber64
	{
	public:
		typedef int64_t Value;
		DKAtomicNumber64(Value initialValue = 0);
		~DKAtomicNumber64();

		Value Increment();			///< +1, returns new value.
		Value Decrement();			///< -1, returns new value.
		Value Add(Value addend);		///< +addend, returns new value.
		Value Exchange(Value value);	///< set value, returns previous value.

		/// compare and set when equal. return true when operation succeeded.
		bool CompareAndSet(Value comparand, Value value);

		DKAtomicNumber64& operator = (Value value);
		DKAtomicNumber64& operator += (Value value);
		operator Value () const;
	private:
		volatile Value atomic;
	};
}
