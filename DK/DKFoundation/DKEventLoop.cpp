//
//  File: DKEventLoop.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKObject.h"
#include "DKEventLoop.h"
#include "DKMap.h"
#include "DKArray.h"
#include "DKSpinLock.h"
#include "DKFunction.h"
#include "DKLog.h"
#include "DKCondition.h"

namespace DKFoundation
{
	namespace Private
	{
#if defined(__APPLE__) && defined(__MACH__)
		void PerformOperationInsidePool(DKOperation* op);
#else
		static inline void PerformOperationInsidePool(DKOperation* op) { op->Perform(); }
#endif

		typedef DKMap<DKThread::ThreadId, DKObject<DKEventLoop>, DKDummyLock> EventLoopMap;
		static EventLoopMap& GetEventLoopMap(void)
		{
			static EventLoopMap runloopMap;
			return runloopMap;
		}
		static DKSpinLock& GetEventLoopMapLock(void)
		{
			static DKSpinLock lock;
			return lock;
		}
		static bool RegisterEventLoop(DKThread::ThreadId id, DKEventLoop* eventLoop)
		{
			bool ret = false;
			GetEventLoopMapLock().Lock();
			ret = GetEventLoopMap().Insert(id, eventLoop);
			GetEventLoopMapLock().Unlock();
			return ret;
		}
		static void UnregisterEventLoop(DKThread::ThreadId id)
		{
			GetEventLoopMapLock().Lock();
			GetEventLoopMap().Remove(id);
			GetEventLoopMapLock().Unlock();
		}
		static DKEventLoop* GetEventLoop(DKThread::ThreadId id)
		{
			DKEventLoop* ret = NULL;
			GetEventLoopMapLock().Lock();
			EventLoopMap::Pair* p = GetEventLoopMap().Find(id);
			if (p)
				ret = p->value;
			GetEventLoopMapLock().Unlock();
			return ret;
		}
		static bool IsEventLoopExist(const DKEventLoop* runloop)
		{
			bool found = false;
			GetEventLoopMapLock().Lock();
			GetEventLoopMap().EnumerateForward([&](EventLoopMap::Pair& pair, bool* stop)
			{
				if (pair.value == runloop)
				{
					*stop = true;
					found = true;
				}
			});
			GetEventLoopMapLock().Unlock();
			return found;
		}

		static DKCondition resultCond;
		struct EventLoopResultCallback : public DKEventLoop::OperationResult
		{
			enum State
			{
				StatePending,
				StateProcessing,
				StateProcessed,
				StateRevoked,
			};
			mutable State state;

			EventLoopResultCallback(void) : state(StatePending)
			{
			}
			bool EnterOperation(void) const
			{
				DKCriticalSection<DKCondition> guard(resultCond);
				if (state == StatePending)
				{
					state = StateProcessing;
					return true;
				}
				return false;
			}
			void LeaveOperation(void) const
			{
				DKCriticalSection<DKCondition> guard(resultCond);
				DKASSERT(state == StateProcessing);
				state = StateProcessed;
				resultCond.Broadcast();
			}
			bool Revoke(void) const
			{
				DKCriticalSection<DKCondition> guard(resultCond);
				if (state == StatePending)
				{
					state = StateRevoked;
					resultCond.Broadcast();
				}
				return state == StateRevoked;
			}
			bool Result(void) const
			{
				DKCriticalSection<DKCondition> guard(resultCond);
				while (state != StateProcessed && state != StateRevoked)
					resultCond.Wait();

				return state == StateProcessed;
			}
			bool IsDone(void) const
			{
				DKCriticalSection<DKCondition> guard(resultCond);
				return state == StateProcessed;
			}
			bool IsRevoked(void) const
			{
				DKCriticalSection<DKCondition> guard(resultCond);
				return state == StateRevoked;
			}
			bool IsPending(void) const
			{
				DKCriticalSection<DKCondition> guard(resultCond);
				return state == StatePending;
			}
		};
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

bool DKEventLoop::InternalCommandCompareOrder(const InternalCommandTick& lhs, const InternalCommandTick& rhs)
{
	return lhs.fire < rhs.fire;
}

bool DKEventLoop::InternalCommandCompareOrder(const InternalCommandTime& lhs, const InternalCommandTime& rhs)
{
	return lhs.fire < rhs.fire;
}

DKEventLoop::DKEventLoop(void)
: run(false)
, threadId(DKThread::invalidId)
, commandQueueTick(&DKEventLoop::InternalCommandCompareOrder)
, commandQueueTime(&DKEventLoop::InternalCommandCompareOrder)
{
}

DKEventLoop::~DKEventLoop(void)
{
	if (threadId != DKThread::invalidId)
	{
		DKERROR_THROW_DEBUG("EventLoop must be terminated before destroy!");
	}

	RevokeAll();
}

bool DKEventLoop::Run(void)
{
	if (BindThread())
	{
		bool loop = true;
		bool next = true;
		while (loop)
		{
			next = this->Dispatch();
			loop = this->run;
			if (!next && loop)
				OnIdle();
		}
		UnbindThread();
		return true;
	}
	return false;
}

bool DKEventLoop::IsRunning(void) const
{
	if (threadId != DKThread::invalidId)
	{
		DKASSERT_DEBUG(IsEventLoopExist(this));
		return true;
	}
	return false;
}

void DKEventLoop::InternalPostCommand(const InternalCommandTick& cmd)
{
	DKCriticalSection<DKCondition> guard(commandQueueCond);
	commandQueueTick.Insert(cmd);
	commandQueueCond.Signal();
}

void DKEventLoop::InternalPostCommand(const InternalCommandTime& cmd)
{
	DKCriticalSection<DKCondition> guard(commandQueueCond);
	commandQueueTime.Insert(cmd);
	commandQueueCond.Signal();
}

bool DKEventLoop::BindThread(void)
{
	if (this->threadId == DKThread::invalidId)
	{
		DKASSERT_DEBUG(IsEventLoopExist(this) == false);
		DKThread::ThreadId tid = DKThread::CurrentThreadId();
		if (RegisterEventLoop(tid, this))
		{
			this->threadId = tid;
			run = true;
			return true;
		}
		else
		{

			DKLog("BindThread failed!\n");
		}
	}
	return false;
}

void DKEventLoop::UnbindThread(void)
{
	DKASSERT_DEBUG(IsEventLoopExist(this));
	DKASSERT_DEBUG(threadId != DKThread::invalidId);
	DKASSERT_DEBUG(GetEventLoop(threadId) == this);
	UnregisterEventLoop(this->threadId);
	threadId = DKThread::invalidId;
}

void DKEventLoop::Stop(void)
{
	if (threadId != DKThread::invalidId)
	{
		DKASSERT_DEBUG(IsEventLoopExist(this));

		this->Post(DKFunction([this]() {
			this->run = false;
		})->Invocation());
	}
}

DKObject<DKEventLoop::OperationResult> DKEventLoop::Post(const DKOperation* operation, double delay)
{
	if (operation)
	{
		InternalCommandTick cmd;
		cmd.operation = const_cast<DKOperation*>(operation);
		cmd.result = DKOBJECT_NEW EventLoopResultCallback();
		cmd.fire = DKTimer::SystemTick() + static_cast<DKTimer::Tick>(DKTimer::SystemTickFrequency() * Max(delay, 0.0));
		InternalPostCommand(cmd);

		return cmd.result;
	}
	return NULL;
}

DKObject<DKEventLoop::OperationResult> DKEventLoop::Post(const DKOperation* operation, const DKDateTime& runAfter)
{
	if (operation)
	{
		InternalCommandTime cmd;
		cmd.operation = const_cast<DKOperation*>(operation);
		cmd.result = DKOBJECT_NEW EventLoopResultCallback();
		cmd.fire = runAfter;
		InternalPostCommand(cmd);

		return cmd.result;
	}
	return NULL;
}

bool DKEventLoop::Process(const DKOperation* op)
{
	if (op)
	{
		if (this->IsWrokingThread())
		{
			op->Perform();
			return true;
		}
		else
		{
			auto p = this->Post(op);
			if (p)
				return p->Result();
		}
	}
	return false;
}

size_t DKEventLoop::RevokeAll(void)
{
	DKCriticalSection<DKCondition> guard(this->commandQueueCond);

	//DKTimer::Tick tick = DKTimer::SystemTick();

	size_t numItems = this->commandQueueTick.Count() + this->commandQueueTime.Count();

	auto revoke = [](const InternalCommand& ic)
	{
		const EventLoopResultCallback* callback = ic.result.StaticCast<EventLoopResultCallback>();
		if (callback)
			callback->Revoke();
	};

	for (const InternalCommand& ic : this->commandQueueTick)
		revoke(ic);
	for (const InternalCommand& ic : this->commandQueueTime)
		revoke(ic);

	this->commandQueueTick.Clear();
	this->commandQueueTime.Clear();
	return numItems;
}

bool DKEventLoop::GetNextLoopIntervalNL(double* d) const
{
	DKTimer::Tick currentTick = DKTimer::SystemTick();
	DKDateTime currentDate = DKDateTime::Now();

	size_t numTickCmd = 0;
	size_t numTimeCmd = 0;
	double tickDelay = 0;
	double timeDelay = 0;

	numTickCmd = commandQueueTick.Count();
	numTimeCmd = commandQueueTime.Count();

	double freq = 1.0 / static_cast<double>(DKTimer::SystemTickFrequency());
	if (numTickCmd > 0)
	{
		const InternalCommandTick& cmd = commandQueueTick.Value(0);
		if (cmd.fire > currentTick && freq > 0)
			tickDelay = static_cast<double>(cmd.fire - currentTick) * freq;
	}
	if (numTimeCmd > 0)
	{
		const InternalCommandTime& cmd = commandQueueTime.Value(0);
		if (cmd.fire > currentDate)
			timeDelay = cmd.fire.Interval(currentDate);
	}

	if (numTickCmd > 0 || numTimeCmd > 0)
	{
		double delay = 0;

		if (numTickCmd > 0 && numTimeCmd > 0)
			delay = Min(tickDelay, timeDelay);
		else if (numTickCmd > 0)
			delay = tickDelay;
		else
			delay = timeDelay;

		*d = delay;
		return true;
	}
	return false;
}

void DKEventLoop::WaitNextLoop(void)
{
	DKCriticalSection<DKCondition> guard(this->commandQueueCond);

	double d = 0.0;
	if (GetNextLoopIntervalNL(&d))
	{
		d = Max(d, 0.0);
		if (d > 0.0)
			this->commandQueueCond.WaitTimeout(d);
	}
	else
	{
		this->commandQueueCond.Wait();
	}
}

bool DKEventLoop::WaitNextLoopTimeout(double t)
{
	if (t > 0.0)
	{
		DKCriticalSection<DKCondition> guard(this->commandQueueCond);

		double d = 0.0;
		if (GetNextLoopIntervalNL(&d))
		{
			double delay = Clamp(d, 0.0, t);
			if (delay > 0.0)
			{
				this->commandQueueCond.WaitTimeout(delay);
			}
			return delay < t;
		}
		else
		{
			this->commandQueueCond.WaitTimeout(t);
		}
	}
	return false;
}

void DKEventLoop::PerformOperation(const DKOperation* operation)
{
	operation->Perform();
}

bool DKEventLoop::Dispatch(void)
{
	DKASSERT_DEBUG(this->threadId == DKThread::CurrentThreadId());

	DKObject<DKOperation> operation = NULL;
	DKObject<OperationResult> result = NULL;

	commandQueueCond.Lock();

	DKDateTime currentTime = DKDateTime::Now();
	DKTimer::Tick currentTick = DKTimer::SystemTick();

	if (operation == NULL && this->commandQueueTick.Count() > 0)
	{
		const InternalCommandTick& cmd = this->commandQueueTick.Value(0);
		if (cmd.fire <= currentTick)
		{
			operation = cmd.operation;
			result = cmd.result;
			this->commandQueueTick.Remove(0);
		}
	}
	if (operation == NULL && this->commandQueueTime.Count() > 0)
	{
		const InternalCommandTime& cmd = this->commandQueueTime.Value(0);
		if (cmd.fire <= currentTime)
		{
			operation = cmd.operation;
			result = cmd.result;
			this->commandQueueTime.Remove(0);
		}
	}
	commandQueueCond.Unlock();

	if (operation)
	{
		struct OpWrapper : public DKOperation
		{
			OpWrapper(DKEventLoop* e, DKOperation* o) : el(e), op(o) {}
			DKEventLoop* el;
			DKOperation* op;

			void Perform(void) const override
			{
				el->PerformOperation(op);
			}
		};
		OpWrapper op(this, operation);
		EventLoopResultCallback* resultCallback = result.StaticCast<EventLoopResultCallback>();
		if (resultCallback)
		{
			if (resultCallback->EnterOperation())
			{
				Private::PerformOperationInsidePool(&op);
				resultCallback->LeaveOperation();
			}
		}
		else
		{
			Private::PerformOperationInsidePool(&op);
		}

		return true;
	}
	return false;
}

bool DKEventLoop::IsWrokingThread(void) const
{
	return DKThread::CurrentThreadId() == this->threadId;
}

DKEventLoop* DKEventLoop::CurrentEventLoop(void)
{
	return GetEventLoop(DKThread::CurrentThreadId());
}

DKEventLoop* DKEventLoop::EventLoopForThreadID(DKThread::ThreadId id)
{
	return GetEventLoop(id);
}

bool DKEventLoop::IsRunning(DKEventLoop* runloop)
{
	return IsEventLoopExist(runloop);
}
