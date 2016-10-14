//
//  File: DKSpinLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKSpinLock.h"
#include "DKThread.h"

namespace DKGL
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

using namespace DKGL;
using namespace DKGL::Private;

DKSpinLock::DKSpinLock(void)
	: state(SpinLockStateFree)
{
}

DKSpinLock::~DKSpinLock(void)
{
}

void DKSpinLock::Lock(void) const
{
	while (!TryLock())
		DKThread::Yield();
}

bool DKSpinLock::TryLock(void) const
{
	return state.CompareAndSet(SpinLockStateFree, SpinLockStateLocked);
}

void DKSpinLock::Unlock(void) const
{
	state = SpinLockStateFree;
}
