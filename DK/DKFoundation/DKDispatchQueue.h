//
//  File: DKDispatchQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKOperation.h"

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

        DKDispatchQueue();
        virtual ~DKDispatchQueue();

        virtual bool DispatchSync(DKOperation* op)
        {
            if (IsDispatchThread())
                return InvokeOperation(op);
            return Submit(op)->WaitUntilCompleted();
        }
        virtual void DispatchAsync(DKOperation* op)
        {
            if (IsDispatchThread())
                InvokeOperation(op);
            Submit(op);
        }
        virtual DKObject<ExecutionState> Submit(DKOperation*, double delay = 0.0);

        /// execute single operation, should be executed in the dispatch thread.
        virtual bool Execute();
        /// revoke all pending operations
        void RevokeAll();

        bool WaitQueue(double timeout) const;
        void WaitQueue() const;
        static void NotyfyThreads();

        /// If this function returns false, 
        /// the WaitQueue() function will eventually stop and return.
        virtual bool IsRunning() const { return true; }

        /// If this function can correctly identify the dispatch thread, 
        /// the DispatchSync(), DispatchAsync() functions can execute the task directly.
        virtual bool IsDispatchThread() const { return false; }

        virtual bool InvokeOperation(DKOperation*) const;

    private:
        struct Context;
        Context* context;
    };
}
