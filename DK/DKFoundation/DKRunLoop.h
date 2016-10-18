//
//  File: DKRunLoop.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
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
// Installs Run-Loop system into current thread which called DKRunLoop::Run(),
// and provides control interfaces.
// The Run-Loop is running loops and invoke operations.
// Operations can be scheduled by specifed date or delayed by specified delay value.
//
// If you call 'Run()', the RunLoop enter infinite loop and dispatch messages.
// You can post termination message with DKRunLoop::Terminate() to terminate loop.
//
// On main thread, application should waits RunLoop's beging terminated by
// calling 'Terminate(true)' on application exits.
//
// You can control individual operation would be process or not by overrides in
// subclass. You can call 'DKRunLoop::Process()' for process one operation,
//
// PostOperation has two version with overloaded.
//   tick-based: system-tick based, calling operation with delayed time.
//   time-based: system time based, calling operation at specified system time.
//               if system time has changed, calling operations will adjusted.
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKRunLoop
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

        void WaitNextLoop(void);
		bool WaitNextLoopTimeout(double t);

		// PostOperation: insert operation and return immediately.
		virtual DKObject<OperationResult> PostOperation(const DKOperation* operation, double delay = 0);			// tick base
		virtual DKObject<OperationResult> PostOperation(const DKOperation* operation, const DKDateTime& runAfter);	// time base

		// ProcessOperation: insert operation and wait until done.
		virtual bool ProcessOperation(const DKOperation* operation);

		// returns RunLoop object which runs on current thread as worker-thread.
		static DKRunLoop* CurrentRunLoop(void);
		// returns RunLoop for specified thread-id.
		static DKRunLoop* RunLoopForThreadID(DKThread::ThreadId id);
		static bool IsRunning(DKRunLoop* runloop);

	protected:
		virtual void PerformOperation(const DKOperation* operation);
		virtual void OnStart(void) {}
		virtual void OnStop(void) {}
		virtual void OnIdle(void) { WaitNextLoop(); }

		bool Process(void); // return true if a message has been dispatched.

        // If you override 'Run()' for customize behaviors, you should bind-thread
        // by calling 'BindThread()', and unbind by calling 'UnbindThread()',
        // within your customized 'Run()'. Do not call DKRunLoop::Run() in your
        // overridden version of 'Run()'.
        bool BindThread(void);
        void UnbindThread(void);
        bool ShouldTerminate(void) const;
		size_t RevokeAllOperations(void);

	private:
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

		DKCondition			terminateCond; // wait for shut-down
		DKThread::ThreadId	threadId;
		bool				terminate;

		static bool InternalCommandCompareOrder(const InternalCommandTick&, const InternalCommandTick&);
		static bool InternalCommandCompareOrder(const InternalCommandTime&, const InternalCommandTime&);
	};
	typedef DKRunLoop::OperationResult DKRunLoopOperationResult;
}
