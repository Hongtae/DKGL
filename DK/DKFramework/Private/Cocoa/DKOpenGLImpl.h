//
//  File: DKOpenGLImpl.h
//  Platform: Mac OS X
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if !TARGET_OS_IPHONE

#ifdef __OBJC__
#import <AppKit/AppKit.h> 
#else
class NSView;
class NSOpenGLPixelFormat;
class NSOpenGLContext;
#endif

#include "../../Interface/DKOpenGLInterface.h"

namespace DKFramework
{
	namespace Private
	{
		class DKOpenGLImpl : public DKOpenGLInterface
		{
		public:
			DKOpenGLImpl(void);
			~DKOpenGLImpl(void);

			void Bind(void* target) const;		// bind context to thread
			void Unbind(void) const;
			bool IsBound(void) const;

			void Flush(void) const;				// glFlush
			void Finish(void) const;			// glFinis
			void Present(void) const;			// swap buffers
			
			void Update(void) const;			// update view

			bool GetSwapInterval(void) const;
			void SetSwapInterval(bool interval) const;

			unsigned int FramebufferId(void) const		{return 0;}
		private:
			NSOpenGLPixelFormat*	pixelFormat;
			NSOpenGLContext*		mainContext;

			struct BoundContext
			{
				NSOpenGLContext*	ctxt;
				NSView*				target;
				int					count;
			};

			// context pool
			typedef DKFoundation::DKMap<pthread_t, BoundContext, DKFoundation::DKSpinLock> SharedContextMap;
			mutable SharedContextMap		sharedContexts;
		};
	}
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
