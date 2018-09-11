//
//  File: DKEventLoopTimer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKOperation.h"
#include "DKEventLoop.h"
#include "DKDateTime.h"
#include "DKTimer.h"

namespace DKFoundation
{
	/// @brief Installs scheduled repetition of operation with interval into DKEventLoop.
	class DKGL_API DKEventLoopTimer
	{
	public:
		~DKEventLoopTimer();

		static DKObject<DKEventLoopTimer> Create(const DKOperation* operation, double interval, DKEventLoop* eventLoop = NULL);

		size_t Count() const;		   ///< number of timer did fired
		DKEventLoop* EventLoop() const;	///< return event-loop that the timer installed on
		double Interval() const;
		bool IsRunning() const;

		void Invalidate();

	private:
		struct Invoker : public DKOperation
		{
			virtual void Invalidate() = 0;
			virtual size_t Count() const = 0;
			virtual DKEventLoop* EventLoop() const = 0;
			virtual bool IsRunning() const = 0;
			virtual double Interval() const = 0;
		};
		DKObject<Invoker> invoker;

		DKEventLoopTimer();
		friend class DKObject<DKEventLoopTimer>;
	};
}
