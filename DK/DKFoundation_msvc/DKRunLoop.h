//
//  File: DKRunLoop.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKThread.h"
#include "DKOperation.h"
#include "DKDateTime.h"
#include "DKQueue.h"
#include "DKSpinLock.h"
#include "DKArray.h"
#include "DKOrderedArray.h"
#include "DKTimer.h"
#include "DKCondition.h"

////////////////////////////////////////////////////////////////////////////////
// DKRunLoop
// Installs Run-Loop system into internal thread, and provides control interfaces.
// The Run-Loop is running loops and invoke operations.
// Operations can be scheduled by specifed date or delayed by specified delay value.
//
// If you call 'Run()', the RunLoop creates internal worker-thread and running.
// It runs with thread which RunLoop has ownership.
// You need to call 'Terminate()' to terminate RunLoop loops by internal thread.
//
// On main thread, application should waits RunLoop's beging terminated by
// calling 'Terminate(true)' on application exits.
//
// You can control individual operation would be process or not by overrides in
// subclass. You can call 'ProcessOne', 'ProcessInTime' for process one operation,
// process multiple operations in time, they should be called in worker-thread.
// (a worker-thread which is IsWorkingThread() returns true.)
//
// PostOperation has two version with overloaded.
//   tick-based: system-tick based, calling operation with delayed time.
//   time-based: system time based, calling operation at specified system time.
//               if system time has changed, calling operation wills adjusted.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKRunLoop
	{
	public:
		struct OperationResult
		{
			virtual ~OperationResult(void) {}
			virtual bool Result(void) const = 0;	// wait until done
			virtual bool Revoke(void) const = 0;
			virtual bool IsDone(void) const = 0;
			virtual bool IsRevoked(void) const = 0;
			virtual bool IsPending(void) const = 0;
		};

		DKRunLoop(void);
		virtual ~DKRunLoop(void);

		bool IsRunning(void) const;
		bool IsWrokingThread(void) const;
		virtual bool Run(void);
		virtual void Terminate(bool wait);

		bool ProcessOne(bool processIdle);
		size_t ProcessInTime(size_t maxCmd, double timeout);
		void WaitNextLoop(void);
		bool WaitNextLoopTimeout(double t);

		// PostOperation: insert operation and return immediately.
		DKObject<OperationResult> PostOperation(const DKOperation* operation, double delay = 0);			// tick base
		DKObject<OperationResult> PostOperation(const DKOperation* operation, const DKDateTime& runAfter);	// time base

		// ProcessOperation: insert operation and wait until done.
		bool ProcessOperation(const DKOperation* operation);

		// returns RunLoop object which runs on current thread as worker-thread.
		static DKRunLoop* CurrentRunLoop(void);
		// returns RunLoop for specified thread-id.
		static DKRunLoop* RunLoopForThreadID(DKThread::ThreadId id);
		static bool IsRunning(DKRunLoop* runloop);

	protected:
		virtual void OnInitialize(void);
		virtual void OnTerminate(void);
		virtual void OnIdle(void);
		virtual void PerformOperation(const DKOperation* operation);

	private:
		size_t RevokeAllOperations(void);
		void RunLoopProc(void);
		bool GetNextLoopIntervalNL(double*) const;

		struct InternalCommand
		{
			DKObject<DKOperation>		operation;
			DKObject<OperationResult>	result;

			virtual ~InternalCommand(void) {}
		};
		struct InternalCommandTick : public InternalCommand { DKTimer::Tick fire; };
		struct InternalCommandTime : public InternalCommand { DKDateTime fire; };
		void InternalPostCommand(const InternalCommandTick& cmd);
		void InternalPostCommand(const InternalCommandTime& cmd);

		DKCondition								commandQueueCond;
		DKOrderedArray<InternalCommandTick>		commandQueueTick;
		DKOrderedArray<InternalCommandTime>		commandQueueTime;

		DKObject<DKThread>	thread;
		DKThread::ThreadId	threadId;
		DKSpinLock			threadLock;
		bool				terminate;

		static bool InternalCommandCompareOrder(const InternalCommandTick&, const InternalCommandTick&);
		static bool InternalCommandCompareOrder(const InternalCommandTime&, const InternalCommandTime&);
	};
	typedef DKRunLoop::OperationResult DKRunLoopOperationResult;
}
