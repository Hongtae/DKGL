//
//  File: DKOperationQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKThread.h"
#include "DKOperation.h"
#include "DKQueue.h"
#include "DKCondition.h"
#include "DKSpinLock.h"

////////////////////////////////////////////////////////////////////////////////
// DKOperationQueue
// processing operations with multi-threaded.
// this class manages thread pool automatically.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKOperationQueue
	{
	public:
		struct OperationSync
		{
			enum State
			{
                StateUnknown = 0,
				StatePending,
				StateProcessed,
				StateCancelled,
			};

			virtual ~OperationSync(void) {}
			virtual bool Sync(void) = 0;
			virtual bool Cancel(void) = 0;
			virtual State OperationState(void) = 0;
		};

		class ThreadFilter
		{
			friend class DKOperationQueue;
		public:
			virtual ~ThreadFilter(void) {}
		protected:
			virtual void OnThreadInitialized(void) {}
			virtual void OnThreadTerminate(void) {}
			virtual void PerformOperation(DKOperation* op)
			{
				op->Perform();
			}
		};

		DKOperationQueue(ThreadFilter* filter = NULL);
		~DKOperationQueue(void);

		void SetMaxConcurrentOperations(size_t maxConcurrent);
		size_t MaxConcurrentOperations(void) const;

		void Post(DKOperation* operation);
		DKObject<OperationSync> ProcessAsync(DKOperation* operation);
		bool Process(DKOperation* operation);	// wait until done.
		void CancelAllOperations(void);			// cancel all operations.
		void WaitForCompletion(void) const;		// wait until all operations are done.

		size_t QueueLength(void) const;
		size_t RunningOperations(void) const;
		size_t RunningThreads(void) const;

	private:
		struct Operation
		{
			DKObject<DKOperation> operation;
			DKObject<OperationSync> sync;
		};
		typedef DKQueue<Operation, DKDummyLock> OperationQueue;
		OperationQueue operationQueue;
		size_t maxConcurrentOperations;
		size_t threadCount;			// available threads count
		size_t maxThreadCount;		// maximum threads count
		size_t activeThreads;		// working threads count
		DKCondition threadCond;
		DKObject<ThreadFilter> filter;

		void UpdateThreadPool(void);
		void OperationProc(void);

		DKOperationQueue(const DKOperationQueue&);
		DKOperationQueue& operator = (const DKOperationQueue&);
	};
}
