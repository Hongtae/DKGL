//
//  File: DKEventLoop.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
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

namespace DKFoundation
{
	/**
	 @brief
	 Installs Event-Loop system into current thread which called DKEventLoop::Run(),
	 and provides control interfaces.
	 The Event-Loop is running loops and invoke operations.
	 Operations can be scheduled by specifed date or delayed by specified delay value.

	 If you call 'Run()', the EventLoop enter infinite loop and dispatch messages.
	 You can post termination message with DKEventLoop::Stop() to terminate loop.

	 On main thread, application should waits EventLoop's beging terminated by
	 calling 'Terminate()' on application exits.

	 You can control individual operation would be process or not by overrides in
	 subclass. You can call 'DKEventLoop::Process()' for process one operation,

	 Post() has two version with overloaded.
	   tick-based: system-tick based, calling operation with delayed time.
	   time-based: system time based, calling operation at specified system time.
				   if system time has changed, calling operations will adjusted.

	 @note
	  To make Event-Loop working on a new thread, create a DKThread object and call
	  'DKEventLoop::Run()' inside new working thread.

	 @code
		 DKEventLoop* myLoop;
		 // detach new thread with EventLoop.
		 DKObject<DKThread> workerThread = DKThread::Create([myLoop]() {
										 // your initialize code here.
										 myLoop->Run();  // run dispatch
										 // your finalize code here.
										 // thread is about to be terminated.
										 })->Invocation());
		  // do something with Event-Loop from outside of worker thread.
		  myLoop->PostOperation(...);

		  // terminate Event-Loop
		  myLoop->Stop();
		  workerThread->WaitTerminate();  // wait for termination.
	 @endcode
	 */
	class DKGL_API DKEventLoop
	{
	public:
		/// interface to query pending event states
		struct PendingState
		{
			virtual ~PendingState(void) {}
			virtual bool Result(void) const = 0;	///< wait until done
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

		/// Install event queue and run loops on current thread.
		/// If you override 'Run()' for customize behaviors, you should bind-thread
		/// by calling 'BindThread()', and unbind by calling 'UnbindThread()',
		/// within your customized 'Run()'. Do not call DKEventLoop::Run() in your
		/// overridden version of 'Run()'.
		virtual bool Run(void);
		/// Post stop command.
		/// The command will be executed on worker thread.
		virtual void Stop(void);
		
		/// Blocks the current thread until an active event arrives.
		/// However, the arrival of an active event does not guarantee immediate
		/// processing of the event, and the Event-Loop must go to the next loop.
        void WaitNextLoop(void);
		/// Blocks the current thread until an active event arrives or times out.
		/// However, the arrival of an active event does not guarantee immediate
		/// processing of the event, and the Event-Loop must go to the next loop.
		bool WaitNextLoopTimeout(double t);
		/// Retrieves the delay before the next event begins.
		/// @return A positive value indicates a delay before the next event.
		/// @return A negative value means there are no active events in the queue.
		double PendingEventInterval(void) const;

		/// Enqueue operation and return immediately. 
		/// The operation will be performed after delay. (system-tick-based)
		/// @param operation an operation object.
		/// @param delay delay to execute operation (0 = execute as fast as possible)
		virtual DKObject<PendingState> Post(const DKOperation* operation, double delay = 0);			
		/// Enqueue operation and return immediately. 
		/// The operation will be performed at given date. (system-date-based)
		/// @param operation an operation object.
		/// @param runAfter specific date/time to execute operation
		virtual DKObject<PendingState> Post(const DKOperation* operation, const DKDateTime& runAfter);

		/// Enqueue operation and wait until done.
		/// if the function called on working-thread, the operation will be executed immediately.
		/// @param operation an operation object.
		bool Process(const DKOperation* operation);

		/// returns EventLoop object which runs on current thread as worker-thread.
		static DKEventLoop* CurrentEventLoop(void);
		/// returns EventLoop for specified thread-id.
		static DKEventLoop* EventLoopForThreadId(DKThread::ThreadId id);
		/// Check whether the given EventLoop is running or not.
		/// @param eventLoop EventLoop to test, should not be null.
		static bool IsRunning(DKEventLoop* eventLoop);

	protected:
		virtual void PerformOperation(const DKOperation* operation);
		virtual void OnIdle(void) { WaitNextLoop(); }

		/// Process one event(operation)
		/// @return true if a event has been dispatched.
		/// @return false if there are no active events in the queue.
		bool Dispatch(void);

		/// Bind current event-loop object to current thread.
		/// If you override DKEventLoop::Run, You should call BindThread
		/// before process any event.
        bool BindThread(void);
		/// Unbind current event-loop object to current thread.
		/// If you have overridden DKEventLoop::Run, you have to call UnbindThread 
		/// at terminating worker-thread.
		/// counterpart of BindThread
        void UnbindThread(void);

		/// Remove all operations in queue.
		/// Unprocessed events will be in the 'canceled' state.
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
