//
//  File: DKOpenGLImpl.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
#warning Compiling DKOpenGLImpl for iOS

#include <pthread.h>

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

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
: mainContext(NULL)
{
	mainContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
	DKASSERT_DESC(mainContext != nil, "Failed to create context.\n");
	
	EAGLContext* currentContext = [EAGLContext currentContext];
	[EAGLContext setCurrentContext:mainContext];

	DKString::StringArray exts = DKString((const DKUniChar8*)glGetString(GL_EXTENSIONS)).Split(L" ");
	for (size_t i = 0; i < exts.Count(); i++)
	{
		//DKLog("OpenGL Extension: %ls\n", (const wchar_t*)exts.Value(i));
		extensions.Insert(exts.Value(i));
	}
	[EAGLContext setCurrentContext:currentContext];
}

DKOpenGLImpl::~DKOpenGLImpl(void)
{
	if (IsBound())
		Unbind();
	
	[EAGLContext setCurrentContext: mainContext];

	sharedContexts.EnumerateForward([](SharedContextMap::Pair& pair)
									{
										BoundContext& bc = pair.value;

										EAGLContext* context = bc.ctxt;
										if (bc.frameBufferId)
											glDeleteFramebuffers(1, &bc.frameBufferId);
										if (bc.colorBufferId)
											glDeleteRenderbuffers(1, &bc.colorBufferId);
										if (bc.depthBufferId)
											glDeleteRenderbuffers(1, &bc.depthBufferId);
										[context release];
										bc.ctxt = nil;
									});

	sharedContexts.Clear();

	[EAGLContext setCurrentContext: nil];
	[mainContext release];
}

bool DKOpenGLImpl::IsBound(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		DKASSERT_DESC_DEBUG(p->value.ctxt == [EAGLContext currentContext], "Current Context Mismatch!");
		
		return true;
	}
	return false;
}

void DKOpenGLImpl::Bind(void* target) const
{
	pthread_t currentThreadId = pthread_self();

	if (target && [(id)target isKindOfClass:[UIView class]] == NO)
		DKERROR_THROW("Invalid Target");
		
	SharedContextMap::Pair* p = sharedContexts.Find(currentThreadId);
	if (p)
	{
		if (target)
		{
			DKASSERT_DESC_DEBUG(p->value.target == target, "Target mismatch.");
		}
		DKASSERT_DEBUG(p->value.count >= 0);
		p->value.count++;
	}
	else
	{
		// create new context
		EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:[mainContext sharegroup] ];
		DKASSERT_DESC(context != nil, "NSOpenGLContext creation failed!");

		[EAGLContext setCurrentContext:context];

		unsigned int frameBufferId = 0;
		unsigned int colorBufferId = 0;
		unsigned int depthBufferId = 0;
		
		if (target)
		{
			CAEAGLLayer* eaglLayer = (CAEAGLLayer*)((UIView*)target).layer;
			eaglLayer.opaque = YES;
			eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
											[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
											kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
											nil];
			
			glGenFramebuffers(1, &frameBufferId);
			glGenRenderbuffers(1, &colorBufferId);
			glGenRenderbuffers(1, &depthBufferId);
		}
		
		BoundContext	bc = {context, (UIView*)target, frameBufferId, colorBufferId, depthBufferId, 1};
		if (!sharedContexts.Insert(currentThreadId, bc))
			DKERROR_THROW("SharedContext insert failed!");
		Update();
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
			
			if (p->value.frameBufferId)
				glDeleteFramebuffers(1, &p->value.frameBufferId);
			if (p->value.colorBufferId)
				glDeleteRenderbuffers(1, &p->value.colorBufferId);
			if (p->value.depthBufferId)
				glDeleteRenderbuffers(1, &p->value.depthBufferId);
			
			[EAGLContext setCurrentContext: nil];
			EAGLContext* context = p->value.ctxt;
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
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		EAGLContext* context = p->value.ctxt;
		UIView* target = p->value.target;
		
		if (context != nil && context == [EAGLContext currentContext])
		{
			if (target)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, p->value.frameBufferId);
				const GLenum discards[] = {GL_DEPTH_ATTACHMENT};
				glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);
				glBindRenderbuffer(GL_RENDERBUFFER, p->value.colorBufferId);
				BOOL result = [context presentRenderbuffer:GL_RENDERBUFFER];
				if (!result)
					DKLog("Error: Failed to present buffer.\n");				
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
}

void DKOpenGLImpl::Update(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		EAGLContext* context = p->value.ctxt;
		UIView* target = p->value.target;
		if (target)
		{
			glFlush();

			[EAGLContext setCurrentContext: nil];
			[EAGLContext setCurrentContext:context];	
			
			CAEAGLLayer* eaglLayer = (CAEAGLLayer*)target.layer;

			// framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, p->value.frameBufferId);
			
			// color buffer
			glBindRenderbuffer(GL_RENDERBUFFER, p->value.colorBufferId);
			[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, p->value.colorBufferId);
			
			// get vewport resolution
			GLint backingWidth = 0;
			GLint backingHeight = 0;			
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
			
			// depth buffer
			glBindRenderbuffer(GL_RENDERBUFFER, p->value.depthBufferId);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, backingWidth, backingHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, p->value.depthBufferId);
			
			// check status
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				NSLog(@"Error: Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
				DKERROR_THROW("Failed to attach render buffer.");
			}

			GLint depthBufferBits = 0;
			glGetIntegerv(GL_DEPTH_BITS, &depthBufferBits);
			DKLog("[%s] Rendering device updated. (%d x %d x %d)\n", DKGL_FUNCTION_NAME, backingWidth, backingHeight, depthBufferBits);
		}
		else
		{
			glFlush();
		}
	}
}

bool DKOpenGLImpl::GetSwapInterval(void) const
{
	DKLog("Warning: GetSwapInterval is not supported in this implementation.\n");
	return false;
}

void DKOpenGLImpl::SetSwapInterval(bool interval) const
{
	DKLog("Warning: SetSwapInterval is not supported in this implementation.\n");
}

unsigned int DKOpenGLImpl::FramebufferId(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		return p->value.frameBufferId;
	}
	return 0;
}

#endif //if TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)

