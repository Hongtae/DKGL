//
//  File: DKOpenGLContext.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKOpenGLContext.h"
#include "Interface/DKOpenGLInterface.h"

using namespace DKFoundation;
using namespace DKFramework;

DKOpenGLContext::RenderStateMap DKOpenGLContext::stateMap;
DKSpinLock DKOpenGLContext::stateLock;

DKOpenGLContext::DKOpenGLContext(void)
{
	impl = DKOpenGLInterface::CreateInterface(this);
}

DKOpenGLContext::~DKOpenGLContext(void)
{
	delete impl;
}

bool DKOpenGLContext::IsBound(void) const
{
	return impl->IsBound();
}

void DKOpenGLContext::Bind(const DKWindow* window) const
{
	void* target = NULL;
	if (window)
		target = window->PlatformHandle();
	impl->Bind(target);

	DKCriticalSection<DKSpinLock> guard(stateLock);
	DKThread::ThreadId currentThreadId = DKThread::CurrentThreadId();
	RenderStateMap::Pair* p = stateMap.Find(currentThreadId);
	if (p == NULL)
	{
		DKObject<DKRenderState> renderState = DKOBJECT_NEW DKRenderState();
		renderState->Reset();
		stateMap.Insert(currentThreadId, renderState);
	}
}

void DKOpenGLContext::Unbind(void) const
{
	impl->Unbind();
	if (impl->IsBound() == false)
	{
		DKCriticalSection<DKSpinLock> guard(stateLock);
		stateMap.Remove(DKThread::CurrentThreadId());
	}
}

void DKOpenGLContext::Flush(void) const
{
	impl->Flush();
}

void DKOpenGLContext::Finish(void) const
{
	impl->Finish();
}

void DKOpenGLContext::Present(void) const
{
	impl->Present();
}

void DKOpenGLContext::Update(void) const
{
	impl->Update();
	RenderState().Reset();
}

bool DKOpenGLContext::GetSwapInterval(void) const
{
	return impl->GetSwapInterval();
}

void DKOpenGLContext::SetSwapInterval(bool interval) const
{
	return impl->SetSwapInterval(interval);
}

DKRenderState& DKOpenGLContext::RenderState(void)
{
	DKCriticalSection<DKSpinLock> guard(stateLock);
	RenderStateMap::Pair* p = stateMap.Find(DKThread::CurrentThreadId());

	DKASSERT_DESC_DEBUG(p != NULL, "OpenGL Context did not bound to current thread!");

	return *(p->value);
}

unsigned int DKOpenGLContext::FramebufferId(void) const
{
	return impl->FramebufferId();
}
