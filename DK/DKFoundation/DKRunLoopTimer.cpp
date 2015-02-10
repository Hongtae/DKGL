//
//  File: DKRunLoopTimer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKRunLoopTimer.h"
#include "DKFunction.h"

using namespace DKFoundation;

DKRunLoopTimer::DKRunLoopTimer(void)
{
}

DKRunLoopTimer::~DKRunLoopTimer(void)
{
	// operation should be invalidated when destroying!
	Invalidate();
}

DKObject<DKRunLoopTimer> DKRunLoopTimer::Create(const DKOperation* operation, double interval, DKRunLoop* runLoop)
{
	if (operation == NULL)
		return NULL;
	if (runLoop == NULL)
		runLoop = DKRunLoop::CurrentRunLoop();
	if (runLoop == NULL)
		return NULL;


	struct RunLoopInvoker : public Invoker
	{
		DKObject<DKOperation>	operation;
		volatile bool			invalidated; // DKRunLoop is destroyed.
		volatile size_t			count;
		DKTimer::Tick			interval;
		DKTimer::Tick			start;
		DKRunLoop*				runloop; // DKRunLoop

		void Invalidate(void) override				{ invalidated = true; }
		size_t Count(void) const override			{ return count; }
		DKRunLoop* RunLoop(void) const override		{ return runloop; }
		bool IsRunning(void) const override			{ return !invalidated && runloop != NULL; }
		double Interval(void) const override
		{
			return static_cast<double>(interval) / static_cast<double>(DKTimer::SystemTickFrequency());
		}
		void Perform(void) const override
		{
			RunLoopInvoker& invoker = const_cast<RunLoopInvoker&>(*this);
			if (!invoker.invalidated)
			{
				invoker.operation->Perform();
				invoker.count++;
				invoker.Install();
			}
		}
		bool Install(void)
		{
			// Install next operation into RunLoop.
			if (DKRunLoop::IsRunning(this->runloop) && this->invalidated == false)
			{
				// calculate multiple of interval with invoker->next.
				// cannot depend on calling time accuracy.
				const DKTimer::Tick currentTick = DKTimer::SystemTick(); // current time.
				const DKTimer::Tick startTick = this->start;
				const DKTimer::Tick interval = this->interval;
				const DKTimer::Tick elapsed = currentTick - startTick; // time elapsed since last call.

				// set next calling time to multiple of interval.
				DKTimer::Tick nextTick = currentTick + interval - (elapsed % interval);

				if (elapsed < interval) // called earlier then expected. skip next one.
					nextTick += interval;

				this->start = nextTick - interval; // save result. minus interval to make high accuracy.

				double d = static_cast<double>(nextTick - currentTick) / static_cast<double>(DKTimer::SystemTickFrequency());
				return runloop->PostOperation(this, d) != NULL;
			}
			return false;
		}
	};


	interval = Max<double>(interval, 0.01);

	DKObject<RunLoopInvoker> invoker = DKObject<RunLoopInvoker>::New();
	invoker->operation = const_cast<DKOperation*>(operation);
	invoker->invalidated = false;
	invoker->count = 0;
	invoker->interval = static_cast<DKTimer::Tick>(DKTimer::SystemTickFrequency() * interval);
	invoker->start = DKTimer::SystemTick();
	invoker->runloop = runLoop;

	// Use invoker(RunLoopInvoker) to install operation into DKRunLoop.
	// once operation has called, it installs next operation repeatedly.
	if (invoker->Install())
	{
		DKObject<DKRunLoopTimer> timer = DKObject<DKRunLoopTimer>::New();
		timer->invoker = invoker.SafeCast<Invoker>();
		return timer;
	}
	return NULL;
}

size_t DKRunLoopTimer::Count(void) const
{
	if (invoker)
		return invoker->Count();
	return 0;
}

DKRunLoop* DKRunLoopTimer::RunLoop(void) const
{
	if (invoker)
		return invoker->RunLoop();
	return NULL;
}

double DKRunLoopTimer::Interval(void) const
{
	if (invoker)
		return invoker->Interval();
	return 0.0;
}

bool DKRunLoopTimer::IsRunning(void) const
{
	if (invoker)
		return invoker->IsRunning();
	return false;
}

void DKRunLoopTimer::Invalidate(void)
{
	if (invoker)
		invoker->Invalidate();
}
