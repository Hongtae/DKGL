//
//  File: DKEventLoopTimer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKEventLoopTimer.h"
#include "DKFunction.h"

using namespace DKFoundation;

DKEventLoopTimer::DKEventLoopTimer(void)
{
}

DKEventLoopTimer::~DKEventLoopTimer(void)
{
	// operation should be invalidated when destroying!
	Invalidate();
}

DKObject<DKEventLoopTimer> DKEventLoopTimer::Create(const DKOperation* operation, double interval, DKEventLoop* eventLoop)
{
	if (operation == NULL)
		return NULL;
	if (eventLoop == NULL)
		eventLoop = DKEventLoop::CurrentEventLoop();
	if (eventLoop == NULL)
		return NULL;


	struct EventLoopInvoker : public Invoker
	{
		DKObject<DKOperation>	operation;
		volatile bool			invalidated; // DKEventLoop is destroyed.
		volatile size_t			count;
		DKTimer::Tick			interval;
		DKTimer::Tick			start;
		DKEventLoop*			eventLoop; // DKEventLoop

		void Invalidate(void) override				{ invalidated = true; }
		size_t Count(void) const override			{ return count; }
		DKEventLoop* EventLoop(void) const override	{ return eventLoop; }
		bool IsRunning(void) const override			{ return !invalidated && eventLoop != NULL; }
		double Interval(void) const override
		{
			return static_cast<double>(interval) / static_cast<double>(DKTimer::SystemTickFrequency());
		}
		void Perform(void) const override
		{
			EventLoopInvoker& invoker = const_cast<EventLoopInvoker&>(*this);
			if (!invoker.invalidated)
			{
				invoker.operation->Perform();
				invoker.count++;
				invoker.Install();
			}
		}
		bool Install(void)
		{
			// Install next operation into EventLoop.
			if (DKEventLoop::IsRunning(this->eventLoop) && this->invalidated == false)
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
				return eventLoop->Post(this, d) != NULL;
			}
			return false;
		}
	};


	interval = Max(interval, 0.01);

	DKObject<EventLoopInvoker> invoker = DKObject<EventLoopInvoker>::New();
	invoker->operation = const_cast<DKOperation*>(operation);
	invoker->invalidated = false;
	invoker->count = 0;
	invoker->interval = static_cast<DKTimer::Tick>(DKTimer::SystemTickFrequency() * interval);
	invoker->start = DKTimer::SystemTick();
	invoker->eventLoop = eventLoop;

	// Use invoker(EventLoopInvoker) to install operation into DKEventLoop.
	// once operation has called, it installs next operation repeatedly.
	if (invoker->Install())
	{
		DKObject<DKEventLoopTimer> timer = DKObject<DKEventLoopTimer>::New();
		timer->invoker = invoker.SafeCast<Invoker>();
		return timer;
	}
	return NULL;
}

size_t DKEventLoopTimer::Count(void) const
{
	if (invoker)
		return invoker->Count();
	return 0;
}

DKEventLoop* DKEventLoopTimer::EventLoop(void) const
{
	if (invoker)
		return invoker->EventLoop();
	return NULL;
}

double DKEventLoopTimer::Interval(void) const
{
	if (invoker)
		return invoker->Interval();
	return 0.0;
}

bool DKEventLoopTimer::IsRunning(void) const
{
	if (invoker)
		return invoker->IsRunning();
	return false;
}

void DKEventLoopTimer::Invalidate(void)
{
	if (invoker)
		invoker->Invalidate();
}
