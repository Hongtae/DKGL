//
//  File: DKOpenGLImpl.h
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2009-2014 Hongtae Kim. All rights reserved.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE

#ifdef __OBJC__
#import <UIKit/UIKit.h>
#else	// ifdef __OBJC__
class UIView;
class EAGLContext;
#endif	// ifdef __OBJC__

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

			void Bind(void* target) const;		// bind context to thread.
			void Unbind(void) const;			// unbind
			bool IsBound(void) const;

			void Flush(void) const;				// glFlush
			void Finish(void) const;			// glFinish
			void Present(void) const;			// swap buffers

			void Update(void) const;			// update view

			bool GetSwapInterval(void) const;
			void SetSwapInterval(bool interval) const;

			unsigned int FramebufferId(void) const;
		private:
			EAGLContext*	mainContext;		// main context
		
			struct BoundContext
			{
				EAGLContext*		ctxt;
				UIView*				target;
				unsigned int		frameBufferId;
				unsigned int		colorBufferId;
				unsigned int		depthBufferId;
				int					count;
			};

			// OpenGL ES extensions
			DKFoundation::DKSet<DKFoundation::DKString> extensions;
			
			// context pool
			typedef DKFoundation::DKMap<pthread_t, BoundContext, DKFoundation::DKSpinLock> SharedContextMap;
			mutable SharedContextMap		sharedContexts;
		};
	}
}

#endif //if TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
