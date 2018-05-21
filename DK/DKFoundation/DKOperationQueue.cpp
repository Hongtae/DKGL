//
//  File: DKOperationQueue.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKObject.h"
#include "DKOperationQueue.h"
#include "DKFunction.h"
#include "DKLog.h"
#include "DKTimer.h"
#include "DKCondition.h"
#include "DKUtils.h"

namespace DKFoundation
{
	namespace Private
	{
#if defined(__APPLE__) && defined(__MACH__)
		void PerformOperationInsidePool(DKOperation* op);
#else
		FORCEINLINE void PerformOperationInsidePool(DKOperation* op) { op->Perform(); }
#endif

		static DKCondition operationStateCond;
		struct OperationSyncState : public DKOperationQueue::OperationSync
		{
			State state;

			OperationSyncState(void) : state(StateUnknown)
			{
			}
			bool Sync(void)
			{
				DKCriticalSection<DKCondition> guard(operationStateCond);
				while (state == State::StatePending)
					operationStateCond.Wait();

				return state == State::StateProcessed;
			}
			bool Cancel(void)
			{
				DKCriticalSection<DKCondition> guard(operationStateCond);
				if (state == State::StatePending)
				{
					state = State::StateCancelled;
					return true;
				}
				return false;
			}
			State OperationState(void)
			{
				return state;
			}
		};
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKOperationQueue::DKOperationQueue(ThreadFilter* f)
	: maxConcurrentOperations(16)
	, threadCount(0)
	, maxThreadCount(0)
	, activeThreads(0)
	, filter(f)
{
	maxConcurrentOperations = Max(2, static_cast<int>(DKNumberOfProcessors()) - 1);
}

DKOperationQueue::~DKOperationQueue(void)
{
	threadCond.Lock();
	maxThreadCount = 0;
	threadCond.Broadcast();
	while (threadCount > 0)
		threadCond.Wait();

	DKASSERT_DEBUG(activeThreads == 0);

	operationStateCond.Lock();
	auto cancelOps = [](Operation& op)
	{
		OperationSyncState* st = op.sync.StaticCast<OperationSyncState>();
		if (st && st->state == OperationSync::StatePending)
			st->state = OperationSync::StateCancelled;
	};
	operationQueue.EnumerateForward(cancelOps);
	operationQueue.Clear();

	operationStateCond.Broadcast();
	operationStateCond.Unlock();
	threadCond.Unlock();
}

void DKOperationQueue::SetMaxConcurrentOperations(size_t maxConcurrent)
{
	threadCond.Lock();
	maxConcurrentOperations = Max(maxConcurrent, 1);
	threadCond.Unlock();

	UpdateThreadPool();
}

size_t DKOperationQueue::MaxConcurrentOperations(void) const
{
	DKCriticalSection<DKCondition> guard(threadCond);
	return maxConcurrentOperations;
}

void DKOperationQueue::Post(DKOperation* operation)
{
	if (operation)
	{
		Operation op = {operation, NULL};
		threadCond.Lock();
		operationQueue.PushBack(op);
		threadCond.Broadcast();
		threadCond.Unlock();
		UpdateThreadPool();
	}
}

DKObject<DKOperationQueue::OperationSync> DKOperationQueue::ProcessAsync(DKOperation* operation)
{
	if (operation)
	{
		DKObject<OperationSyncState> sync = DKOBJECT_NEW OperationSyncState();
		sync->state = OperationSync::StatePending;
		Operation op = {operation, sync.StaticCast<OperationSync>()};
		threadCond.Lock();
		operationQueue.PushBack(op);
		threadCond.Broadcast();
		threadCond.Unlock();
		UpdateThreadPool();

		return sync.StaticCast<OperationSync>();
	}
	return NULL;
}

bool DKOperationQueue::Process(DKOperation* operation)
{
	DKObject<OperationSync> s = ProcessAsync(operation);
	if (s)
		return s->Sync();
	return false;
}

void DKOperationQueue::UpdateThreadPool(void)
{
	threadCond.Lock();
	maxThreadCount = maxConcurrentOperations;
	while (threadCount < maxThreadCount)
	{
		if (operationQueue.Count() > threadCount)
		{
			DKObject<DKThread> thread = DKThread::Create(DKFunction(this, &DKOperationQueue::OperationProc)->Invocation());
			if (thread)
			{
				threadCount++;
			}
		}
		else
		{
			break;
		}
	}
	//size_t numThreads = threadCount;
	threadCond.Unlock();

	//DKLog("OperationQueue running %u threads (maxConcurrent:%d)\n", (unsigned int)numThreads, maxConcurrentOperations);
}

void DKOperationQueue::CancelAllOperations(void)
{
	threadCond.Lock();

	operationStateCond.Lock();
	auto cancelOps = [](Operation& op)
	{
		OperationSyncState* st = op.sync.StaticCast<OperationSyncState>();
		if (st && st->state == OperationSync::StatePending)
			st->state = OperationSync::StateCancelled;
	};
	operationQueue.EnumerateForward(cancelOps);
	operationQueue.Clear();

	operationStateCond.Broadcast();
	operationStateCond.Unlock();

	threadCond.Broadcast();
	threadCond.Unlock();
}

void DKOperationQueue::WaitForCompletion(void) const
{
	DKCriticalSection<DKCondition> guard(threadCond);
	while (operationQueue.Count() > 0 || activeThreads > 0)
		threadCond.Wait();
}

bool DKOperationQueue::WaitForAnyOperation(double timeout) const
{
	timeout = Max(timeout, 0.0);
	DKCriticalSection<DKCondition> guard(threadCond);
	return threadCond.WaitTimeout(timeout);

}

size_t DKOperationQueue::QueueLength(void) const
{
	DKCriticalSection<DKCondition> guard(threadCond);
	return operationQueue.Count();
}

size_t DKOperationQueue::RunningOperations(void) const
{
	DKCriticalSection<DKCondition> guard(threadCond);
	return activeThreads;
}

size_t DKOperationQueue::RunningThreads(void) const
{
	DKCriticalSection<DKCondition> guard(threadCond);
	return threadCount;
}

void DKOperationQueue::OperationProc(void)
{
	DKThread::ThreadId threadId = DKThread::CurrentThreadId();
	DKTimer timer;
	timer.Reset();
	size_t numOps = 0;

	threadCond.Lock();

	auto PerformOperation = [this](DKOperation* op)
	{
		struct Wrapper : public DKOperation
		{
			void Perform(void) const override
			{
				if (filter)
					filter->PerformOperation(op);
				else
					op->Perform();
			}
			Wrapper(ThreadFilter* f, DKOperation* o) : filter(f), op(o) {}
			ThreadFilter* filter;
			DKOperation* op;
		};
		Wrapper wr(filter, op);
		PerformOperationInsidePool(&wr);
	};

	if (filter)
		filter->OnThreadInitialized();

	DKLog("DKOperationQueue_Thread:0x%x started.\n", threadId);

	while (true)
	{
		DKASSERT_DEBUG(threadCount > 0);
		if (threadCount > maxThreadCount)
		{
			break; // terminate.
		}

		Operation op = {NULL, NULL};
		if (operationQueue.PopFront(op))
		{
			activeThreads++;
			threadCond.Unlock();

			OperationSyncState* st = op.sync.StaticCast<OperationSyncState>();
			if (st)
			{
				operationStateCond.Lock();
				if (st->state == OperationSync::StatePending)
				{
					if (op.operation)
					{
						st->state = OperationSync::StateExecuting;
						operationStateCond.Unlock();
						PerformOperation(op.operation);
						numOps++;
						operationStateCond.Lock();
						st->state = OperationSync::StateProcessed;
					}
					else
					{
						st->state = OperationSync::StateCancelled;
					}
					operationStateCond.Broadcast();
				}
				operationStateCond.Unlock();
			}
			else if (op.operation)
			{
				PerformOperation(op.operation);
				numOps++;
			}

			op.operation = NULL;
			op.sync = NULL;

			threadCond.Lock();
			activeThreads--;
			threadCond.Broadcast();
		}
		else
		{
			threadCond.Wait();
		}
	}

	if (filter)
		filter->OnThreadTerminate();

	DKLog("DKOperationQueue_Thread:0x%x terminated. (running %f seconds, %lu processed)\n", threadId, timer.Elapsed(), numOps);

	threadCount--;
	threadCond.Broadcast();
	threadCond.Unlock();
}
