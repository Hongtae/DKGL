//
//  File: DKThread.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKOperation.h"

////////////////////////////////////////////////////////////////////////////////
// DKThread
// creating new thread with DKOperation.
// becouse thread runs parallel, thread ownership will be shared with caller and callee.
// If you abandon ownership of thread, thread will be detached automatically.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKThread
	{
	public:
		typedef unsigned long ThreadId;

		// waiting for join.
		void WaitTerminate(void) const;
		// get thread-id (system thread-id)
		// GetCurrentThreadId() for Win32, pthread_self() for pthread.
		ThreadId Id(void) const;
		// determines thread is running(or sleeping).
		bool IsAlive(void) const;

		// find thread specified by id.
		static DKObject<DKThread> FindThread(ThreadId id);
		// get current thread as DKThread object.
		static DKObject<DKThread> CurrentThread(void);
		// get current thread-id
		static ThreadId CurrentThreadId(void);
		// yeild CPU
		static void Yield(void);
		// sleep current thread.
		static void Sleep(double d);

		// create new thread with DKOperation and run.
		static DKObject<DKThread> Create(const DKOperation* op);

		// a constant of invalid-thread id.
		static const ThreadId invalidId;

	private:
		DKThread(void);
		~DKThread(void);

		DKThread(const DKThread&);
		DKThread& operator = (const DKThread&);

		mutable ThreadId threadId;
		friend class DKObject<DKThread>;
	};
}
