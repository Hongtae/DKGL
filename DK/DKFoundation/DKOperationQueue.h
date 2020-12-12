//
//  File: DKOperationQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKThread.h"
#include "DKOperation.h"
#include "DKQueue.h"
#include "DKCondition.h"
#include "DKSpinLock.h"

namespace DKFoundation
{
	/// Processing operations with multi-threaded.
	/// This class manages thread pool automatically.
	class DKGL_API DKOperationQueue
	{
	public:
		/// retrieve operation state which is enqueued by DKOperationQueue::Post
		struct OperationSync
		{
			enum State
			{
                StateUnknown = 0,
				StatePending,	///< operation is pending and not processed yet.
				StateExecuting, ///< operation is currently executing and cannot be cancelled.
				StateProcessed,	///< operation has been processed.
				StateCancelled,	///< operation was cancelled by system or user
			};

			virtual ~OperationSync() {}
			virtual bool Sync() = 0;	///< Wait until operation finished
			virtual bool Cancel() = 0;	///< Cancellation request
			virtual State OperationState() = 0; ///< Query state
		};

		/// threading filter.
		/// Use this class to override behavior of background thread.
		class ThreadFilter
		{
			friend class DKOperationQueue;
		public:
			virtual ~ThreadFilter() {}
		protected:
			virtual void OnThreadInitialized() {}	///< invoked on initialize thread
			virtual void OnThreadTerminate() {}		///< invoked on finialize thread
			virtual void PerformOperation(DKOperation* op) ///< invoked on processing operation
			{
				op->Perform();
			}
		};

		DKOperationQueue(ThreadFilter* filter = NULL);
		~DKOperationQueue();

		void SetMaxConcurrentOperations(size_t maxConcurrent);
		size_t MaxConcurrentOperations() const;

		void Post(DKOperation* operation);
		DKObject<OperationSync> ProcessAsync(DKOperation* operation);
		bool Process(DKOperation* operation);	///< wait until done.
		void CancelAllOperations();			///< cancel all operations.
		void WaitForCompletion() const;		///< wait until all operations are done.
		bool WaitForAnyOperation(double timeout) const; ///< wait for any single operation has done or time-out.

		size_t QueueLength() const;			///< Number of operations in queue.
		size_t RunningOperations() const;	///< Number of operations currently in process.
		size_t RunningThreads() const;		///< Number of active threads.

	private:
		struct Operation
		{
			DKObject<DKOperation> operation;
			DKObject<OperationSync> sync;
		};
		typedef DKQueue<Operation> OperationQueue;
		OperationQueue operationQueue;
		size_t maxConcurrentOperations;
		size_t threadCount;			// available threads count
		size_t maxThreadCount;		// maximum threads count
		size_t activeThreads;		// working threads count
		DKCondition threadCond;
		DKObject<ThreadFilter> filter;

		void UpdateThreadPool();
		void OperationProc();

		DKOperationQueue(const DKOperationQueue&);
		DKOperationQueue& operator = (const DKOperationQueue&) = delete;
	};
}
