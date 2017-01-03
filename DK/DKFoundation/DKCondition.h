//
//  File: DKCondition.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/**
	 @brief Condition object (pthread_cond)
	 This class can be used in multi-threaded,
	 threads can communicate with this object by generating signal.

	 You should call Lock() before using, Unlock() when done.
	 Wait(), WaitTimeout() invocation will unlock temporary, re-lock when returns.

	 @code
	  // thread #1
	  cond.Lock();    // lock before access value directly.
	  value = false;  // modify value which is shared between multiple-threads.
	  cond.Signal();  // nodify value has been changed to other thread.
	  cond.Unlock();  // done
	 @endcode
	 @code
	  // thread#2 using Wait() function.
	  cond.Lock();
	  while ( value )
		  cond.Wait();  // wait until value become to 'true'.
	  // do-something.. value is true.
	  cond.Unlock();  // done!
	 @endcode
	 @code
	  // thread#3 using WaitTimeOut() function.
	  cond.Lock();
	  if (cond.WaitTimeout(3.0))  // wait for 3 secs
	  {
		  // signal-state, check value
	  }
	  else
	  {
		  // time-out! do something.. thread still locked.
	  }
	  cond.Unlock();  // done!
	 @endcode
	 */
	class DKGL_API DKCondition
	{
	public:
		DKCondition(void);
		~DKCondition(void);

		/// wait until signal
		void Wait(void) const;

		/// wait until signal or timed-out
		/// return true if signal state else return false (timed-out)
		bool WaitTimeout(double t) const;

		/// Signals the condition, waking up one thread waiting on it.
		void Signal(void) const;
		/// Signals the condition, waking up all threads waiting on it.
		void Broadcast(void) const;

		/// lock context
		void Lock(void) const;
		/// try lock context, return true if lock succeeded.
		bool TryLock(void) const;
		void Unlock(void) const;

	private:
		DKCondition(const DKCondition&) = delete;
		DKCondition& operator = (const DKCondition&) = delete;
		void* impl;
	};
}
