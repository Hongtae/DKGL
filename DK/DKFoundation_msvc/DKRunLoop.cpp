//
//  File: DKRunLoop.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKObject.h"
#include "DKRunLoop.h"
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

		typedef DKMap<DKThread::ThreadId, DKObject<DKRunLoop>, DKDummyLock> RunLoopMap;
		static RunLoopMap& GetRunLoopMap(void)
		{
			static RunLoopMap runloopMap;
			return runloopMap;
		}
		static DKSpinLock& GetRunLoopMapLock(void)
		{
			static DKSpinLock lock;
			return lock;
		}
		static bool RegisterRunLoop(DKThread::ThreadId id, DKRunLoop* runLoop)
		{
			bool ret = false;
			GetRunLoopMapLock().Lock();
			ret = GetRunLoopMap().Insert(id, runLoop);
			GetRunLoopMapLock().Unlock();
			return ret;
		}
		static void UnregisterRunLoop(DKThread::ThreadId id)
		{
			GetRunLoopMapLock().Lock();
			GetRunLoopMap().Remove(id);
			GetRunLoopMapLock().Unlock();
		}
		static DKRunLoop* GetRunLoop(DKThread::ThreadId id)
		{
			DKRunLoop* ret = NULL;
			GetRunLoopMapLock().Lock();
			RunLoopMap::Pair* p = GetRunLoopMap().Find(id);
			if (p)
				ret = p->value;
			GetRunLoopMapLock().Unlock();
			return ret;
		}
		static bool IsRunLoopExist(DKRunLoop* runloop)
		{
			bool found = false;
			GetRunLoopMapLock().Lock();
			GetRunLoopMap().EnumerateForward([&](RunLoopMap::Pair& pair, bool* stop)
			{
				if (pair.value == runloop)
				{
					*stop = true;
					found = true;
				}
			});
			GetRunLoopMapLock().Unlock();
			return found;
		}

		void WaitTerminateAllRunLoops(void)
		{
			GetRunLoopMapLock().Lock();

			DKArray<DKThread::ThreadId> runloopThreadIds;
			runloopThreadIds.Reserve(GetRunLoopMap().Count());
			// Terminate all RunLoops
			GetRunLoopMap().EnumerateForward([&runloopThreadIds](RunLoopMap::Pair& pair)
			{
				runloopThreadIds.Add(pair.key);
				pair.value->Terminate(0);

			});
			GetRunLoopMapLock().Unlock();

			// Wait until all runloops being terminated.
			for (DKThread::ThreadId tid : runloopThreadIds)
			{
				DKObject<DKThread> thread = DKThread::FindThread(tid);
				if (thread)
					thread->WaitTerminate();
			}
		}

		static DKCondition resultCond;
		struct RunLoopResultCallback : public DKRunLoop::OperationResult
		{
			enum State
			{
				StatePending,
				StateProcessing,
				StateProcessed,
				StateRevoked,
			};
			mutable State state;

			RunLoopResultCallback(void) : state(StatePending)
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

bool DKRunLoop::InternalCommandCompareOrder(const InternalCommandTick& lhs, const InternalCommandTick& rhs)
{
	return lhs.fire < rhs.fire;
}

bool DKRunLoop::InternalCommandCompareOrder(const InternalCommandTime& lhs, const InternalCommandTime& rhs)
{
	return lhs.fire < rhs.fire;
}

DKRunLoop::DKRunLoop(void)
: terminate(true)
, thread(NULL)
, threadId(DKThread::invalidId)
, commandQueueTick(&DKRunLoop::InternalCommandCompareOrder)
, commandQueueTime(&DKRunLoop::InternalCommandCompareOrder)
{
}

DKRunLoop::~DKRunLoop(void)
{
	if (thread && thread->IsAlive())
	{
		DKERROR_THROW_DEBUG("RunLoop must be terminated before destroy!");
	}

	RevokeAllOperations();
}

bool DKRunLoop::Run(void)
{
	if (thread && thread->IsAlive())
		return false;

	DKCriticalSection<DKCondition> guard(this->commandQueueCond);

	// Using DKObject<DKRunLoop> type to prevent loosing ownership on thread.
	// calling Terminate() will destory instance.
	this->thread = DKThread::Create(DKFunction(DKObject<DKRunLoop>(this), &DKRunLoop::RunLoopProc)->Invocation());
	commandQueueCond.Signal();

	threadLock.Lock();
	terminate = false;
	threadLock.Unlock();

	return this->thread != NULL;
}

bool DKRunLoop::IsRunning(void) const
{
	if (thread)
		return thread->IsAlive();
	return false;
}

void DKRunLoop::InternalPostCommand(const InternalCommandTick& cmd)
{
	DKCriticalSection<DKCondition> guard(commandQueueCond);
	commandQueueTick.Insert(cmd);
	commandQueueCond.Signal();
}

void DKRunLoop::InternalPostCommand(const InternalCommandTime& cmd)
{
	DKCriticalSection<DKCondition> guard(commandQueueCond);
	commandQueueTime.Insert(cmd);
	commandQueueCond.Signal();
}

void DKRunLoop::Terminate(bool wait)
{
	if (thread && thread->IsAlive())
	{
		threadLock.Lock();
		terminate = true;
		threadLock.Unlock();

		// On called by worker-thread:
		//    post quit message and return immediately.
		// On called by outside:
		//    Wait until being terminated if wait is true.
		//    otherwise, return immediately.
		if (wait)
		{
			if (DKThread::CurrentThreadId() != thread->Id())
			{
				thread->WaitTerminate();
			}
		}
	}
}

DKObject<DKRunLoop::OperationResult> DKRunLoop::PostOperation(const DKOperation* operation, double delay)
{
	if (operation)
	{
		InternalCommandTick cmd;
		cmd.operation = const_cast<DKOperation*>(operation);
		cmd.result = DKOBJECT_NEW RunLoopResultCallback();
		cmd.fire = DKTimer::SystemTick() + static_cast<DKTimer::Tick>(DKTimer::SystemTickFrequency() * Max(delay, 0.0));
		InternalPostCommand(cmd);

		return cmd.result;
	}
	return NULL;
}

DKObject<DKRunLoop::OperationResult> DKRunLoop::PostOperation(const DKOperation* operation, const DKDateTime& runAfter)
{
	if (operation)
	{
		InternalCommandTime cmd;
		cmd.operation = const_cast<DKOperation*>(operation);
		cmd.result = DKOBJECT_NEW RunLoopResultCallback();
		cmd.fire = runAfter;
		InternalPostCommand(cmd);

		return cmd.result;
	}
	return NULL;
}

bool DKRunLoop::ProcessOperation(const DKOperation* op)
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
			auto p = this->PostOperation(op);
			if (p)
				return p->Result();
		}
	}
	return false;
}

size_t DKRunLoop::RevokeAllOperations(void)
{
	DKCriticalSection<DKCondition> guard(this->commandQueueCond);

	//DKTimer::Tick tick = DKTimer::SystemTick();

	size_t numItems = this->commandQueueTick.Count() + this->commandQueueTime.Count();

	auto revoke = [](const InternalCommand& ic)
	{
		const RunLoopResultCallback* callback = ic.result.StaticCast<RunLoopResultCallback>();
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

bool DKRunLoop::GetNextLoopIntervalNL(double* d) const
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

void DKRunLoop::WaitNextLoop(void)
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

bool DKRunLoop::WaitNextLoopTimeout(double t)
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

void DKRunLoop::OnInitialize(void)
{
}

void DKRunLoop::OnTerminate(void)
{
}

void DKRunLoop::OnIdle(void)
{
	WaitNextLoop();
}

void DKRunLoop::PerformOperation(const DKOperation* operation)
{
	operation->Perform();
}

bool DKRunLoop::ProcessOne(bool processIdle)
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
			OpWrapper(DKRunLoop* r, DKOperation* o) : rl(r), op(o) {}
			DKRunLoop* rl;
			DKOperation* op;

			void Perform(void) const override
			{
				rl->PerformOperation(op);
			}
		};
		OpWrapper op(this, operation);
		RunLoopResultCallback* resultCallback = result.StaticCast<RunLoopResultCallback>();
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
	if (processIdle && !this->terminate)
	{
		struct IdleWrapper : public DKOperation
		{
			IdleWrapper(DKRunLoop* r) : rl(r) {}
			DKRunLoop* rl;

			void Perform(void) const override
			{
				rl->OnIdle();
			}
		};
		IdleWrapper op(this);
		Private::PerformOperationInsidePool(&op);
	}
	return false;
}

size_t DKRunLoop::ProcessInTime(size_t maxCmd, double timeout)
{
	DKASSERT_DEBUG(this->threadId == DKThread::CurrentThreadId());

	size_t processed = 0;
	DKTimer timer;
	timer.Reset();

	while (processed < maxCmd && timer.Elapsed() < timeout)
	{
		if (this->ProcessOne(true))
			processed++;
	}
	return processed;
}

bool DKRunLoop::IsWrokingThread(void) const
{
	return DKThread::CurrentThreadId() == this->threadId;
}

void DKRunLoop::RunLoopProc(void)
{
	this->commandQueueCond.Lock();
	while (this->thread == NULL)
		this->commandQueueCond.Wait();
	this->commandQueueCond.Unlock();

	this->threadId = DKThread::CurrentThreadId();
	DKASSERT_DEBUG(threadId == this->thread->Id());

	DKTimer timer;
	timer.Reset();

	// registering thread for RunLoop.
	RegisterRunLoop(this->threadId, this);

	this->OnInitialize();
	DKLog("DKRunLoop Thread:0x%x initialized.\n", threadId);

	auto TerminateRequest = [this]()->bool
	{
		DKCriticalSection<DKSpinLock> tmp(threadLock);
		return this->terminate;
	};

	while (!TerminateRequest())
	{
		DKASSERT_DEBUG(this->threadId == this->thread->Id());
		this->ProcessOne(true);
	}

	this->OnTerminate();
	UnregisterRunLoop(this->threadId);

	DKLog("DKRunLoop Thread:0x%x terminated. (running %f seconds)\n", threadId, timer.Elapsed());
	this->threadId = DKThread::invalidId;
}

DKRunLoop* DKRunLoop::CurrentRunLoop(void)
{
	return GetRunLoop(DKThread::CurrentThreadId());
}

DKRunLoop* DKRunLoop::RunLoopForThreadID(DKThread::ThreadId id)
{
	return GetRunLoop(id);
}

bool DKRunLoop::IsRunning(DKRunLoop* runloop)
{
	return IsRunLoopExist(runloop);
}
