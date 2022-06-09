//
//  File: DKDispatchQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKOperation.h"
#include "DKCallableRef.h"

namespace DKFoundation
{
    class DKGL_API DKDispatchQueue
    {
    public:
        struct ExecutionState
        {
            enum State
            {
                StatePending = 0,
                StateRevoked,
                StateProcessing,
                StateCompleted,
                StateCompletedWithError,
            };

            virtual ~ExecutionState() {}
            virtual enum State State() const = 0;
            virtual bool WaitUntilCompleted() const = 0;
            virtual bool Revoke() const = 0;
            virtual bool IsPending() const = 0;
            virtual bool IsRevoked() const = 0;
            virtual bool IsCompleted() const = 0;
        };

        struct ScheduledTask
        {
            virtual ~ScheduledTask() {}
            virtual size_t Count() const = 0;
            virtual double Interval() const = 0;
            virtual bool IsRunning() const = 0;
            virtual void Invalidate() = 0;
        };

        constexpr static double minimumScheduledInterval = 0.001;


        DKDispatchQueue();
        virtual ~DKDispatchQueue();

        virtual bool DispatchSync(DKOperation* op)
        {
            return Submit(op)->WaitUntilCompleted();
        }
        virtual void DispatchAsync(DKOperation* op)
        {
            Submit(op);
        }

        bool DispatchSync(const DKCallableRef<void ()>& callable)
        {
            return DispatchSync((DKOperation*)callable->Invocation());
        }
        void DispatchAsync(const DKCallableRef<void ()>& callable)
        {
            return DispatchAsync((DKOperation*)callable->Invocation());
        }
        /// Submits a single job to the dispatch queue with a specific delay time.
        virtual DKObject<ExecutionState> Submit(DKOperation*, double delay = 0.0);
        /// Submit repetitive task at specific time intervals
        DKObject<ScheduledTask> SubmitScheduled(DKOperation*, double interval);

        /// execute single operation, should be executed in the dispatch thread.
        virtual bool Execute();
        /// revoke all pending operations
        void RevokeAll();

        bool WaitQueue(double timeout) const;
        void WaitQueue() const;
        static void NotyfyThreads();

        double NextDispatchInterval() const;

        /// If this function returns false, 
        /// the WaitQueue() function will eventually stop and return.
        virtual bool IsRunning() const { return true; }

        /// If this function can correctly identify the dispatch thread, 
        /// the DispatchSync(), DispatchAsync() functions can execute the task directly.
        virtual bool IsDispatchThread() const { return true; }

        virtual bool InvokeOperation(DKOperation*) const;

    private:
        struct Context;
        Context* context;

        DKDispatchQueue(const DKDispatchQueue&) = delete;
        DKDispatchQueue& operator = (const DKDispatchQueue&) = delete;
    };
}
