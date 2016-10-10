//
//  File: DKOpenGLImpl.mm
//  Platform: Mac OS X
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if !TARGET_OS_IPHONE
#warning Compiling DKOpenGLImpl for Mac OS X

#include <pthread.h>
#import <AppKit/AppKit.h> 

#include "../../../DKInclude.h"
#include "../../../../lib/OpenGL.h"
#include "DKOpenGLImpl.h"


using namespace DKGL;
using namespace DKGL;
using namespace DKGL::Private;

DKOpenGLInterface* DKOpenGLInterface::CreateInterface(DKOpenGLContext*)
{
	return new DKOpenGLImpl();
}

DKOpenGLImpl::DKOpenGLImpl(void)
: pixelFormat(NULL)
, mainContext(NULL)
{
	// create pixel format
	NSOpenGLPixelFormatAttribute attrs[] = {
		NSOpenGLPFAScreenMask,		CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
		NSOpenGLPFAColorSize,		32,
		NSOpenGLPFADepthSize,		24,
		NSOpenGLPFAOpenGLProfile,	NSOpenGLProfileVersion3_2Core,
		NSOpenGLPFADoubleBuffer,	1,
		NSOpenGLPFAAccelerated,		1,
		0
	};
	
	pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];
	
	mainContext = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext:NULL];
	DKASSERT_DESC(mainContext != nil, "Failed to create context.");
}

DKOpenGLImpl::~DKOpenGLImpl(void)
{
	if (IsBound())
		Unbind();

	sharedContexts.EnumerateForward([](SharedContextMap::Pair& pair)
									{
										BoundContext& bc = pair.value;
										NSOpenGLContext* context = bc.ctxt;
										[context clearDrawable];
										[context release];
										bc.ctxt = nil;
									});

	sharedContexts.Clear();

	[mainContext release];
	[pixelFormat release];
}

bool DKOpenGLImpl::IsBound(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		DKASSERT_DESC_DEBUG(p->value.ctxt == [NSOpenGLContext currentContext], "Current Context Mismatch!");
		return true;
	}
	return false;
}

void DKOpenGLImpl::Bind(void* target) const
{
	pthread_t currentThreadId = pthread_self();

	if (target && [(id)target isKindOfClass:[NSView class]] == NO)
		DKERROR_THROW("Invalid Target");
	
	SharedContextMap::Pair* p = sharedContexts.Find(currentThreadId);
	if (p)
	{
		if (target)
		{
			DKASSERT_DESC_DEBUG([p->value.ctxt view] == target, "Target mismatch.");
		}
		DKASSERT_DEBUG(p->value.count >= 0);
		p->value.count++;
	}
	else
	{
		// create new context
		NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: mainContext];
		DKASSERT_DESC(context != nil, "NSOpenGLContext creation failed!");

		// if window is not initialized, view will not be set properly.
		// in this case, update context by calling Update() after window
		// initialize succeeded.
		[context setView:(NSView*)target];
		[context makeCurrentContext];
		BoundContext	bc = {context, (NSView*)target, 1};
		if (!sharedContexts.Insert(currentThreadId, bc))
			DKERROR_THROW("SharedContext insert failed!");
	}
}

void DKOpenGLImpl::Unbind(void) const
{
	pthread_t currentThreadId = pthread_self();

	SharedContextMap::Pair* p = sharedContexts.Find(currentThreadId);
	if (p)
	{
		DKASSERT_DEBUG(p->value.count >= 0);
		p->value.count--;
		if (p->value.count == 0)
		{
			glFinish();

			[NSOpenGLContext clearCurrentContext];
			NSOpenGLContext* context = p->value.ctxt;
			[context clearDrawable];
			[context release];
			sharedContexts.Remove(currentThreadId);
		}
	}
	else
	{
		DKERROR_THROW("cannot find context");
	}
}

void DKOpenGLImpl::Flush(void) const
{
#ifdef DKGL_DEBUG_ENABLED
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
		glFlush();
	else
		DKLog("Error: No context for this thread(%x)\n", pthread_self());
#else
	glFlush();
#endif
}

void DKOpenGLImpl::Finish(void) const
{
#ifdef DKGL_DEBUG_ENABLED
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
		glFinish();
	else
		DKLog("Error: No context for this thread(%x)\n", pthread_self());
#else
	glFinish();
#endif
}

void DKOpenGLImpl::Present(void) const
{
#ifdef DKGL_DEBUG_ENABLED
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		NSOpenGLContext* context = p->value.ctxt;
		NSView* target = p->value.target;
		if (context != nil && context == [NSOpenGLContext currentContext])
		{
			if (target)
			{
				[context flushBuffer];
			}
			else
			{
				DKLog("Error: Failed to present buffer! context has no drawable target!\n");
			}
		}
		else
		{
			DKLog("Error: Bound context is different or nil!\n");
		}
	}
	else
	{
		DKLog("Error: No context for this thread(%x)\n", pthread_self());
	}
#else
	NSOpenGLContext* context = [NSOpenGLContext currentContext];
	if (context)
		[context flushBuffer];
#endif
}

void DKOpenGLImpl::Update(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		if ([p->value.ctxt view] != p->value.target)
			[p->value.ctxt setView:p->value.target];
		else
			[p->value.ctxt update];
	}
}

bool DKOpenGLImpl::GetSwapInterval(void) const
{
	NSOpenGLContext* context = [NSOpenGLContext currentContext];
	if (context)
	{
		GLint val;
		[context getValues:&val forParameter: NSOpenGLCPSwapInterval];
		return (bool)val;
	}
	return false;
}

void DKOpenGLImpl::SetSwapInterval(bool interval) const
{
	NSOpenGLContext* context = [NSOpenGLContext currentContext];
	if (context)
	{
		GLint val = interval;
		[context setValues:&val forParameter: NSOpenGLCPSwapInterval];
	}
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
