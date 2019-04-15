//
//  File: DKFence.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2019 Hongtae Kim. All rights reserved.
//

#include "DKFence.h"
#include "DKCondition.h"
#include "DKSpinLock.h"
#include "DKMap.h"
#include "DKThread.h"

#define BUCKET_SIZE		17

namespace DKFoundation::Private
{
    struct FData
    {
        DKThread::ThreadId threadId;
        size_t count;
    };
    typedef DKMap<const void*, FData> Bucket;
    static Bucket buckets[BUCKET_SIZE];
    static DKCondition cond;
}
using namespace DKFoundation;
using namespace DKFoundation::Private;

DKFence::DKFence(const void* p, bool exclusive)
: key(p)
{
	DKCriticalSection<DKCondition> guard(cond);
	Bucket& b = buckets[reinterpret_cast<uintptr_t>(key) % BUCKET_SIZE];

	if (exclusive)
	{
		DKThread::ThreadId tid = DKThread::invalidId;

		while (b.Find(key) != NULL)
		{
			cond.Wait();
		}
		FData& fd = b.Value(key);
		fd.threadId = tid;
		fd.count = 1;
	}
	else
	{
		DKThread::ThreadId tid = DKThread::CurrentThreadId();

		while (true)
		{
			if (auto p = b.Find(key); p)
			{
				if (p->value.threadId == tid)
				{
					b.Value(key).count++;
					break;
				}
			}
			else
			{
				FData& fd = b.Value(key);
				fd.threadId = tid;
				fd.count = 1;
				break;
			}
			cond.Wait();
		}
	}
}

DKFence::~DKFence()
{
	DKCriticalSection<DKCondition> guard(cond);
	Bucket& b = buckets[reinterpret_cast<uintptr_t>(key) % BUCKET_SIZE];
	auto p = b.Find(key);

	DKASSERT_DESC(p != NULL, "object did not locked?");
	DKASSERT_DESC(p->value.threadId == DKThread::CurrentThreadId(), "INVALID THREAD ACCESS");

	p->value.count--;
	if (p->value.count == 0)
		b.Remove(key);
	cond.Broadcast();
}
