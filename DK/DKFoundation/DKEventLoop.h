//
//  File: DKEventLoop.h
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
#include "DKSpinLock.h"
#include "DKOrderedArray.h"
#include "DKTimer.h"
#include "DKCondition.h"

////////////////////////////////////////////////////////////////////////////////
// DKEventLoop
// Installs Event-Loop system into current thread which called DKEventLoop::Run(),
// and provides control interfaces.
// The Event-Loop is running loops and invoke operations.
// Operations can be scheduled by specifed date or delayed by specified delay value.
//
// If you call 'Run()', the EventLoop enter infinite loop and dispatch messages.
// You can post termination message with DKEventLoop::Stop() to terminate loop.
//
// On main thread, application should waits EventLoop's beging terminated by
// calling 'Terminate()' on application exits.
//
// You can control individual operation would be process or not by overrides in
// subclass. You can call 'DKEventLoop::Process()' for process one operation,
//
// Post() has two version with overloaded.
//   tick-based: system-tick based, calling operation with delayed time.
//   time-based: system time based, calling operation at specified system time.
//               if system time has changed, calling operations will adjusted.
//
// Note:
//  To make Event-Loop working on a new thread, create a DKThread object and call
//  'DKEventLoop::Run()' inside new working thread.
//
// Ex:
//     DKEventLoop* myLoop;
//     // detach new thread with EventLoop.
//     DKObject<DKThread> workerThread = DKThread::Create([myLoop]() {
//                                     // your initialize code here.
//                                     myLoop->Run();  // run dispatch
//                                     // your finalize code here.
//                                     // thread is about to be terminated.
//                                     })->Invocation());
//      // do something with Event-Loop from outside of worker thread.
//      myLoop->PostOperation(...);
//
//      // terminate Event-Loop
//      myLoop->Stop();
//      workerThread->WaitTerminate();  // wait for termination.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKGL_API DKEventLoop
	{
	public:
		struct PendingState
		{
			virtual ~PendingState(void) {}
			virtual bool Result(void) const = 0;	// wait until done
			virtual bool Revoke(void) const = 0;
			virtual bool IsDone(void) const = 0;
			virtual bool IsRevoked(void) const = 0;
			virtual bool IsPending(void) const = 0;
		};

		DKEventLoop(void);
		virtual ~DKEventLoop(void);

		bool IsRunning(void) const;
		bool IsWrokingThread(void) const;
		DKThread::ThreadId RunningThreadId(void) const;

		virtual bool Run(void);
		virtual void Stop(void);

        void WaitNextLoop(void);
		bool WaitNextLoopTimeout(double t);
		double PendingEventInterval(void) const; // negative value means no pending events in queue.

		// Post: insert operation and return immediately.
		virtual DKObject<PendingState> Post(const DKOperation* operation, double delay = 0);			// tick base
		virtual DKObject<PendingState> Post(const DKOperation* operation, const DKDateTime& runAfter);	// time base

		// Process: insert operation and wait until done.
		bool Process(const DKOperation* operation);

		// returns EventLoop object which runs on current thread as worker-thread.
		static DKEventLoop* CurrentEventLoop(void);
		// returns EventLoop for specified thread-id.
		static DKEventLoop* EventLoopForThreadId(DKThread::ThreadId id);
		static bool IsRunning(DKEventLoop* eventLoop);

	protected:
		virtual void PerformOperation(const DKOperation* operation);
		virtual void OnIdle(void) { WaitNextLoop(); }

		bool Dispatch(void); // return true if a message has been dispatched.

        // If you override 'Run()' for customize behaviors, you should bind-thread
        // by calling 'BindThread()', and unbind by calling 'UnbindThread()',
        // within your customized 'Run()'. Do not call DKEventLoop::Run() in your
        // overridden version of 'Run()'.
        bool BindThread(void);
        void UnbindThread(void);
		size_t RevokeAll(void);

	private:
		bool GetNextLoopIntervalNL(double*) const;

		struct InternalCommand
		{
			DKObject<DKOperation>	operation;
			DKObject<PendingState>	state;

			virtual ~InternalCommand(void) {}
		};
		struct InternalCommandTick : public InternalCommand { DKTimer::Tick fire; };
		struct InternalCommandTime : public InternalCommand { DKDateTime fire; };
		void InternalPostCommand(const InternalCommandTick& cmd);
		void InternalPostCommand(const InternalCommandTime& cmd);

		DKCondition								commandQueueCond;
		DKOrderedArray<InternalCommandTick>		commandQueueTick;
		DKOrderedArray<InternalCommandTime>		commandQueueTime;

		DKThread::ThreadId	threadId;
		bool				running;

		static bool InternalCommandCompareOrder(const InternalCommandTick&, const InternalCommandTick&);
		static bool InternalCommandCompareOrder(const InternalCommandTime&, const InternalCommandTime&);
	};
}
