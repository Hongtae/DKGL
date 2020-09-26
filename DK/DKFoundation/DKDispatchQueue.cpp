//
//  File: DKDispatchQueue.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#include "DKObject.h"
#include "DKDispatchQueue.h"
#include "DKTimer.h"
#include "DKCondition.h"
#include "DKArray.h"

namespace DKFoundation::Private
{
#if defined(__APPLE__) && defined(__MACH__)
    void PerformOperationInsidePool(DKOperation* op);
#else
    FORCEINLINE void PerformOperationInsidePool(DKOperation* op) { op->Perform(); }
#endif

    struct DispatchQueueItemState : public DKDispatchQueue::ExecutionState
    {
        DispatchQueueItemState(DKCondition& c) : cond(c), state(StatePending) {}
        DKCondition& cond;
        mutable enum State state;

        enum State State() const override
        {
            DKCriticalSection guard(cond);
            return state;
        }
        bool WaitUntilCompleted() const override
        {
            DKCriticalSection guard(cond);
            while (state == StatePending || state == StateProcessing)
                cond.Wait();

            return
                state == StateCompleted || state == StateCompletedWithError;
        }
        bool Revoke() const override
        {
            DKCriticalSection guard(cond);
            if (state == StatePending)
            {
                state = StateRevoked;
                return true;
            }
            return state == StateRevoked;
        }
        bool IsPending() const override
        {
            DKCriticalSection guard(cond);
            return state == StatePending;
        }
        bool IsRevoked() const override
        {
            DKCriticalSection guard(cond);
            return state == StateRevoked;
        }
        bool IsCompleted() const override
        {
            DKCriticalSection guard(cond);
            return state == StateCompleted || state == StateCompletedWithError;
        }
    };
    struct DispatchQueueItem
    {
        DKTimer::Tick tick;
        DKObject<DKOperation> op;
        DKObject<DispatchQueueItemState> state;
        bool IsReady() const { return DKTimer::Tick() >= tick; }
    };
    DKCondition& DispatchQueueCondition()
    {
        static DKCondition cond;
        return cond;
    }
}
using namespace DKFoundation;
using namespace DKFoundation::Private;

struct DKDispatchQueue::Context
{
    DKCondition& cond;
    DKArray<DispatchQueueItem> queue;
};

DKDispatchQueue::DKDispatchQueue()
    : context(new Context{ DispatchQueueCondition() })
{
}

DKDispatchQueue::~DKDispatchQueue()
{
    RevokeAll();
    delete context;
}

DKObject<DKDispatchQueue::ExecutionState> DKDispatchQueue::Submit(DKOperation* op, double delay)
{
    delay = Max(delay, 0.0);
    DKTimer::Tick fire = DKTimer::SystemTick() + static_cast<DKTimer::Tick>(DKTimer::SystemTickFrequency() * delay);

    DispatchQueueItem item = { fire };
    item.op = op;
    item.state = DKOBJECT_NEW DispatchQueueItemState(context->cond);

    DKCriticalSection guard(context->cond);
    auto pos = context->queue.UpperBound(fire, [](DKTimer::Tick tick, const DispatchQueueItem& rhs)
    {
        return tick < rhs.tick;
    });
    context->queue.Insert(item, pos);
    context->cond.Broadcast();
    return item.state.SafeCast<ExecutionState>();
}

bool DKDispatchQueue::Execute()
{
    DispatchQueueItem item = {};
    if (true)
    {
        DKCriticalSection guard(context->cond);
        if (context->queue.Count() > 0 && context->queue.Value(0).IsReady())
        {
            if (context->queue.Value(0).state->state == DispatchQueueItemState::StatePending)
            {
                item = context->queue.Value(0);
                item.state->state = DispatchQueueItemState::StateProcessing;
            }
            context->queue.Remove(0);
        }
    }
    if (item.state)
    {
        struct CompleteState
        {
            DKObject<DispatchQueueItemState> state;
            bool completedWithoutError;
            ~CompleteState()
            {
                DKCriticalSection guard(state->cond);
                DKASSERT_DEBUG(state->state == DispatchQueueItemState::StateProcessing);
                if (completedWithoutError)
                    state->state = DispatchQueueItemState::StateCompleted;
                else
                    state->state = DispatchQueueItemState::StateCompletedWithError;
                state->cond.Broadcast();
            }
        } s = { item.state, false };

        InvokeOperation(item.op);
        s.completedWithoutError = true;
        return true;
    }
    return false;
}

void DKDispatchQueue::RevokeAll()
{
    DKCriticalSection guard(context->cond);
    for (DispatchQueueItem& item : context->queue)
    {
        if (item.state->state == DispatchQueueItemState::StatePending)
            item.state->state = DispatchQueueItemState::StateRevoked;
    }
    context->queue.Clear();
    context->cond.Broadcast();
}

bool DKDispatchQueue::WaitQueue(double timeout) const
{
    timeout = Max(timeout, 0.0);
    const DKTimer::Tick timeoutTick = DKTimer::SystemTick() +
        static_cast<DKTimer::Tick>(DKTimer::SystemTickFrequency() * timeout);

    const double tickToTime = 1.0 / DKTimer::SystemTickFrequency();

    while (true)
    {
        bool running = IsRunning();
        DKCriticalSection guard(context->cond);
        DKTimer::Tick waitUntil = timeoutTick;
        if (context->queue.Count() > 0)
        {
            const DispatchQueueItem& item = context->queue.Value(0);
            if (item.IsReady())
                return true;
            waitUntil = Min(item.tick, waitUntil);
        }

        const DKTimer::Tick tick = DKTimer::SystemTick();
        if (tick >= waitUntil)
            break;

        if (!running)
            break;

        const double t = static_cast<double>(waitUntil - tick) * tickToTime;
        context->cond.WaitTimeout(t);
    }
    return false;
}

void DKDispatchQueue::WaitQueue() const
{
    const double tickToTime = 1.0 / DKTimer::SystemTickFrequency();

    while (true)
    {
        bool running = IsRunning();
        DKCriticalSection guard(context->cond);
        if (context->queue.Count() > 0)
        {
            const DKTimer::Tick tick = DKTimer::SystemTick();
            const DispatchQueueItem& item = context->queue.Value(0);
            if (item.IsReady())
                break;

            if (!running)
                break;

            const double t = static_cast<double>(item.tick - tick) * tickToTime;
            context->cond.WaitTimeout(t);
        }
        else
        {
            if (!running)
                break;
            context->cond.Wait();
        }
    }
}

void DKDispatchQueue::NotyfyThreads()
{
    DispatchQueueCondition().Broadcast();
}

bool DKDispatchQueue::InvokeOperation(DKOperation* op) const
{
    if (op)
        PerformOperationInsidePool(op);
    return true;
}
