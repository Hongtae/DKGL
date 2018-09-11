//
//  File: DKSpinLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKSpinLock.h"
#include "DKThread.h"

namespace DKFoundation
{
	namespace Private
	{
		enum SpinLockState
		{
			SpinLockStateFree = 0,
			SpinLockStateLocked = 1,
		};
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKSpinLock::DKSpinLock()
	: state(SpinLockStateFree)
{
}

DKSpinLock::~DKSpinLock()
{
}

void DKSpinLock::Lock() const
{
	while (!TryLock())
		DKThread::Yield();
}

bool DKSpinLock::TryLock() const
{
	return state.CompareAndSet(SpinLockStateFree, SpinLockStateLocked);
}

void DKSpinLock::Unlock() const
{
	state = SpinLockStateFree;
}
