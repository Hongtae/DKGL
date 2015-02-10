//
//  File: DKOpenGLInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2013-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKFoundation.h"

////////////////////////////////////////////////////////////////////////////////
// DKOpenGLInterface
// An abstract class, interface for OpenGL or OpenGL ES for platform provides.
// You may need to subclass for your platform, If you have plan to use
// DKOpenGLContext.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKOpenGLContext;
	class DKOpenGLInterface
	{
	public:
		virtual ~DKOpenGLInterface(void) {}

		virtual void Bind(void* target) const = 0;		// bind to thread
		virtual void Unbind(void) const = 0;			// unbind
		virtual bool IsBound(void) const = 0;

		virtual void Flush(void) const = 0;				// glFlush
		virtual void Finish(void) const = 0;			// glFinish
		virtual void Present(void) const = 0;			// swap buffers

		virtual void Update(void) const = 0;			// update window, view

		virtual bool GetSwapInterval(void) const = 0;
		virtual void SetSwapInterval(bool) const = 0;

		virtual unsigned int FramebufferId(void) const		{return 0;}

		static DKOpenGLInterface* CreateInterface(DKOpenGLContext*);
	};
}
