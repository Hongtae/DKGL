//
//  File: DKRunLoopTimer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKOperation.h"
#include "DKRunLoop.h"
#include "DKDateTime.h"
#include "DKTimer.h"

////////////////////////////////////////////////////////////////////////////////
// DKRunLoopTimer
// Installs scheduled repetition of operation with interval into RunLoop.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKRunLoopTimer
	{
	public:
		~DKRunLoopTimer(void);

		static DKObject<DKRunLoopTimer> Create(const DKOperation* operation, double interval, DKRunLoop* runLoop = NULL);

		size_t Count(void) const;		
		DKRunLoop* RunLoop(void) const;
		double Interval(void) const;
		bool IsRunning(void) const;

		void Invalidate(void);

	private:
		struct Invoker : public DKOperation
		{
			virtual void Invalidate(void) = 0;
			virtual size_t Count(void) const = 0;
			virtual DKRunLoop* RunLoop(void) const = 0;
			virtual bool IsRunning(void) const = 0;
			virtual double Interval(void) const = 0;
		};
		DKObject<Invoker> invoker;

		DKRunLoopTimer(void);
		friend class DKObject<DKRunLoopTimer>;
	};
}
