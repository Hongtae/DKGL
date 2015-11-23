//
//  File: DKOpenGLImpl.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once

#ifdef _WIN32
#include "../../../../lib/OpenGL.h"
#include "../../../DKFoundation.h"
#include "../../Interface/DKOpenGLInterface.h"
#include "../../DKWindow.h"

namespace DKFramework
{
	namespace Private
	{
		class DKOpenGLImpl : public DKOpenGLInterface
		{
		public:
			DKOpenGLImpl(void);
			~DKOpenGLImpl(void);

			void Bind(void* target) const;		// bind context to current thread
			void Unbind(void) const;			// unbind
			bool IsBound(void) const;

			void Flush(void) const;				// glFlush
			void Finish(void) const;			// glFinish
			void Present(void) const;			// swap buffers

			void Update(void) const;			// update window

			bool GetSwapInterval(void) const;
			void SetSwapInterval(bool interval) const;

			unsigned int FramebufferId(void) const		{return 0;}
		private:
			HWND					window;
			HDC						windowDC;
			HGLRC					mainContext;
			PIXELFORMATDESCRIPTOR	pxielFormatDesc;
			int						pixelFormat;
			int*					attribs;
			struct BoundContext
			{
				HGLRC		ctxt;
				HWND		targetWindow;
				HDC			targetDC;
				int			count;
			};

			// context pool
			typedef DKFoundation::DKMap<DWORD, BoundContext, DKFoundation::DKSpinLock> SharedContextMap;
			mutable SharedContextMap		sharedContexts;
		};
	}
}

#endif // ifdef _WIN32
