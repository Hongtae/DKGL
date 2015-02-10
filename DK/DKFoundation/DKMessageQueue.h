//
//  File: DKMessageQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKQueue.h"
#include "DKCondition.h"
#include "DKTimer.h"
#include "DKSpinLock.h"
#include "DKObject.h"
#include "DKTimer.h"
#include "DKThread.h"

////////////////////////////////////////////////////////////////////////////////
// DKMessageQueue
// message queue template class. You have to specify return type to 'Result',
// content type of queue to 'MessageContent'
//
// multiple-threads can share one queue, and communicate each others.
//
// to use DKMessageQueue, define your 'Result', 'Content' type.
// you can use your own class or any builtin-types as Result.
// if you provide your own class as 'Result', you should implement
// copy-constructor, assign operator(=), default constructor.
// you can use void type as 'Result'.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename Result, typename MessageContent>
	class DKMessageQueue
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// type stored inside queue.
		// use QReceiver's mutex for thread-synchronize.
		template <typename R, typename U> class QReceiver
		{
		public:
			QReceiver(void) : cond(NULL) {}
			QReceiver(const QReceiver& r) : object(r.object), cond(r.cond), result(r.result) {}
			QReceiver(const U& obj, DKCondition* c) : object(obj), cond(c) {}
			~QReceiver(void) {}
			void SetResult(const R& r)
			{
				if (cond)
				{
					cond->Lock();
					result = r;
					cond->Broadcast();
					cond->Unlock();
					cond = NULL;
				}
				else
				{
					result = r;
				}
			}
			U& Content(void)							{return object;}
			const U& Content(void) const				{return object;}
			QReceiver& operator = (const QReceiver& r)
			{
				object = r.object;
				cond = r.cond;
				result = r.result;
				return *this;
			}
			R GetResult(void) const	{return result;}
		private:
			U object;
			R result;
			DKCondition* cond;
		};
		// specialized template version for void as 'Result'.
		template <typename U> class QReceiver<void, U>
		{
		public:
			QReceiver(void) : cond(NULL) {}
			QReceiver(const QReceiver& r) : object(r.object), cond(r.cond) {}
			QReceiver(const U& obj, DKCondition* c) : object(obj), cond(c) {}
			~QReceiver(void) {}
			void SetResult(void)
			{
				if (cond)
				{
					cond->Lock();
					cond->Broadcast();
					cond->Unlock();
					cond = NULL;
				}
			}
			U& Content(void)							{return object;}
			const U& Content(void) const				{return object;}
			QReceiver& operator = (const QReceiver& r)
			{
				object = r.object;
				cond = r.cond;
				return *this;
			}
			void GetResult(void) const {}
		private:
			U object;
			DKCondition* cond;
		};
		typedef QReceiver<Result, MessageContent> Receiver;

		DKMessageQueue(void)
		{
		}
		~DKMessageQueue(void)
		{
		}
		// post message (with context) and return immediately.
		void PostMessage(const MessageContent& mesg)
		{
			DKObject<Receiver> rp = DKOBJECT_NEW Receiver(mesg, NULL);
			queueCond.Lock();
			messageQueue.PushFront(rp);
			queueCond.Broadcast();
			queueCond.Unlock();
		}
		// post message and wait until done.
		Result ProcessMessage(const MessageContent& mesg)
		{
			DKCondition cond;
			DKObject<Receiver> r = DKOBJECT_NEW Receiver(mesg, &cond);

			queueCond.Lock();
			messageQueue.PushFront(r);
			queueCond.Broadcast();
			queueCond.Unlock();

			cond.Lock();
			cond.Wait(); // wait until done. (working thread should call 'SetResult')
			cond.Unlock();
			return r->GetResult();
		}
		// get message from queue in specified time period.
		// returns NULL if no message received. (timed out)
		DKObject<Receiver> GetMessageTimeOut(double timeout)
		{
			DKObject<Receiver> ret = NULL;
			DKTimer timer;
			timer.Reset();

			queueCond.Lock();
			while (messageQueue.PopBack(ret) == false)
			{
				double dt = timeout - timer.Elapsed();
				if (dt > 0)
				{
					if (queueCond.WaitTimeout(dt))
						continue;
					else
						break;
				}
				else
				{
					break;
				}
			}
			if (messageQueue.Count() > 0)
				queueCond.Signal(); // wake other thread.
			queueCond.Unlock();
			return ret;
		}
		// get message from queue.
		// if wait is true, this function will not return until message received.
		// a retrived message will be removed from queue.
		DKObject<Receiver> GetMessage(bool wait)
		{
			DKObject<Receiver> ret = NULL;

			queueCond.Lock();
			while (messageQueue.PopBack(ret) == false && wait)
			{
				queueCond.Wait();
			}
			if (messageQueue.Count() > 0)
			{
				DKASSERT_DEBUG(ret != NULL);
				queueCond.Signal(); // wake other thread.
			}
			queueCond.Unlock();
			return ret;
		}
		// get a copy of message from queue if queue has messages.
		// original message still remains within queue.
		bool PeekMessage(MessageContent& mesg) const
		{
			DKObject<Receiver> rp = NULL;
			bool ret = false;
			queueCond.Lock();
			if (messageQueue.Back(rp))
			{
				mesg = rp->Content();
				ret = true;
			}
			queueCond.Unlock();
			return ret;
		}
		size_t PendingMessageCount(void) const
		{
			return messageQueue.Count();
		}
		
	private:
		DKCondition						queueCond;
		DKQueue<DKObject<Receiver>>		messageQueue;
	};
}
