//
//  File: DKCallback.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKFunction.h"
#include "DKRunLoop.h"
#include "DKArray.h"
#include "DKSet.h"
#include "DKMap.h"
#include "DKDummyLock.h"
#include "DKCriticalSection.h"

////////////////////////////////////////////////////////////////////////////////
// DKCallback
// enabling Group-Invocations by using DKFunctionSignature, DKRunLoop objects.
//
// Note:
//  If you don't specify runloop (DKRunLoop), then invocation will be called directly.
//  DKCallback's template argument 'Function' must be a function type.
//   (same as DKFunctionSignature's argument types)
//   ex:  DKCallback<int (int), void*> (function-type, context-type)
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename Function, typename Context, typename Lock = DKDummyLock> class DKCallback;
	template <typename R, typename... Ps, typename Context, typename Lock>
	class DKCallback<R (Ps...), Context, Lock>
	{
	public:
		//using Function = R (Ps...);
		typedef R Function(Ps...);
		using FunctionSignature = DKFunctionSignature<Function>;
		using ParameterTuple = typename FunctionSignature::ParameterTuple;

		struct Callback
		{
			DKObject<FunctionSignature>	function;
			DKRunLoop*					runLoop;
		};

		void PostInvocation(Ps... vs) const
		{
			if (contextCallbackMap.Count() > 0)
			{
				ParameterTuple tuple;
				tuple.template SetValue<0>(vs...);

				CallbackList targets;
				CopyCallbackList(targets);
				InvokeTargetsWithTuple(targets, tuple, true);
			}
		}
		void ProcessInvocation(Ps... vs) const
		{
			if (contextCallbackMap.Count() > 0)
			{
				ParameterTuple tuple;
				tuple.template SetValue<0>(vs...);

				CallbackList targets;
				CopyCallbackList(targets);
				InvokeTargetsWithTuple(targets, tuple, false);
			}
		}
		void SetCallback(FunctionSignature* function, DKRunLoop* runLoop, Context context)
		{
			Callback callback;
			callback.function = function;
			callback.runLoop = runLoop;
			SetCallback(&callback, context);
		}
		void SetCallback(Callback* c, Context context)
		{
			if (c && c->function)
				contextCallbackMap.Value(context) = *c;
			else
				contextCallbackMap.Remove(context);
		}
		void Remove(Context context)
		{
			SetCallback(NULL, NULL, context);
		}
		bool IsEmpty(void) const
		{
			return contextCallbackMap.IsEmpty();
		}
		size_t Count(void) const
		{
			return contextCallbackMap.Count();
		}
		FunctionSignature* FunctionForContext(Context context)
		{
			typename ContextCallbackMap::Pair* p = contextCallbackMap.Find(context);
			if (p)
				return p->value.function;
			return NULL;
		}
		const FunctionSignature* FunctionForContext(Context context) const
		{
			const typename ContextCallbackMap::Pair* p = contextCallbackMap.Find(context);
			if (p)
				return p->value.function;
			return NULL;
		}
		DKRunLoop* RunLoopForContext(Context context)
		{
			typename ContextCallbackMap::Pair* p = contextCallbackMap.Find(context);
			if (p)
				return p->value.runLoop;
			return NULL;
		}
		const DKRunLoop* RunLoopForContext(Context context) const
		{
			const typename ContextCallbackMap::Pair* p = contextCallbackMap.Find(context);
			if (p)
				return p->value.runLoop;
			return NULL;
		}
	private:
		using ContextCallbackMap = DKMap<Context, Callback, Lock>;
		struct CallbackWithResult
		{
			Callback callback;
			DKObject<DKRunLoop::OperationResult> result;
		};
		using CallbackList = DKArray<CallbackWithResult>;

		void InvokeTargetsWithTuple(CallbackList& targets, ParameterTuple& tuple, bool async) const
		{
			for (CallbackWithResult& c : targets)
			{
				if (c.callback.runLoop)
				{
					DKObject<DKOperation> op = c.callback.function->InvocationWithTuple(tuple).template SafeCast<DKOperation>();
					DKObject<DKRunLoop::OperationResult> r = c.callback.runLoop->PostOperation(op);
					if (!async)
						c.result = r;
				}
				else
					c.callback.function->InvokeWithTuple(tuple);
			}
			for (CallbackWithResult& c : targets)
			{
				if (c.result)
					c.result->Result();
				c.result = NULL;
			}
		}
		void CopyCallbackList(CallbackList& targets) const
		{
			targets.Reserve(targets.Count() + contextCallbackMap.Count());
			auto fn = [&targets](const typename ContextCallbackMap::Pair& pair)
			{
				targets.Add({pair.value, NULL});
			};
			contextCallbackMap.EnumerateForward(fn);
		}
		ContextCallbackMap contextCallbackMap;
	};
}
