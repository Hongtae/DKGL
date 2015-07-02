//
//  File: DKOpenGLImpl.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include "../../../../lib/OpenGL.h"
#include "../../../../lib/OpenGL/wglext.h"
#include "DKOpenGLImpl.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		static struct OpenGLDeviceDescription
		{
			DKString version;
			DKString vendor;
			int major;
			int minor;
			int patch;
		} openGLDeviceDescription;

		////////////////////////////////////////////////////////////////////////////////
		// WGL Extensions
		// WGL_ARB_extensions_string
		PFNWGLGETEXTENSIONSSTRINGARBPROC										wglGetExtensionsStringARB = 0;
		// WGL_ARB_pixel_format
		PFNWGLGETPIXELFORMATATTRIBIVARBPROC										wglGetPixelFormatAttribivARB = 0;
		PFNWGLGETPIXELFORMATATTRIBFVARBPROC										wglGetPixelFormatAttribfvARB = 0;
		PFNWGLCHOOSEPIXELFORMATARBPROC											wglChoosePixelFormatARB = 0;
		// WGL_ARB_make_current_read
		PFNWGLMAKECONTEXTCURRENTARBPROC											wglMakeContextCurrentARB = 0;
		PFNWGLGETCURRENTREADDCARBPROC											wglGetCurrentReadDCARB = 0;
		// WGL_ARB_create_context
		PFNWGLCREATECONTEXTATTRIBSARBPROC										wglCreateContextAttribsARB = 0;
		// WGL_EXT_extensions_string
		PFNWGLGETEXTENSIONSSTRINGEXTPROC										wglGetExtensionsStringEXT = 0;
		// WGL_EXT_make_current_read
		PFNWGLMAKECONTEXTCURRENTEXTPROC											wglMakeContextCurrentEXT = 0;
		PFNWGLGETCURRENTREADDCEXTPROC											wglGetCurrentReadDCEXT = 0;
		// WGL_EXT_pixel_format
		PFNWGLGETPIXELFORMATATTRIBIVEXTPROC										wglGetPixelFormatAttribivEXT = 0;
		PFNWGLGETPIXELFORMATATTRIBFVEXTPROC										wglGetPixelFormatAttribfvEXT = 0;
		PFNWGLCHOOSEPIXELFORMATEXTPROC											wglChoosePixelFormatEXT = 0;
		// WGL_EXT_swap_control
		PFNWGLSWAPINTERVALEXTPROC												wglSwapIntervalEXT = 0;
		PFNWGLGETSWAPINTERVALEXTPROC											wglGetSwapIntervalEXT = 0;
		////////////////////////////////////////////////////////////////////////////////

#define GET_GL_EXT_PROC(func)		*(PROC*)&func = wglGetProcAddress(#func)

		static bool InitWGLExtensions(void)
		{
			DKString className = L"Dummy Window for OpenGL Extensions";

			// create dummy window. (to create old-style context for query OpenGL extensions)
			WNDCLASSW	wc = { CS_OWNDC, (WNDPROC)DefWindowProcW, 0, 0, ::GetModuleHandleW(NULL), 0, 0, 0, 0, (const wchar_t*)className };
			if (!RegisterClassW(&wc))
				DKERROR_THROW("RegisterClass failed");

			HWND hWnd = ::CreateWindowExW(0, (const wchar_t*)className, L"dummy", WS_POPUP, 0, 0, 0, 0, NULL, NULL, ::GetModuleHandleW(NULL), 0);
			DKASSERT_DESC(hWnd != NULL, "CreateWindowExW failed");

			HDC hDC = GetDC(hWnd);
			DKASSERT_DESC(hDC != NULL, "GetDC failed");

			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_SWAP_EXCHANGE | PFD_GENERIC_ACCELERATED;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 24;
			pfd.iLayerType = PFD_MAIN_PLANE;

			int pixelFormat = ChoosePixelFormat(hDC, &pfd);
			DKASSERT_DESC(pixelFormat != 0, "ChoosePixelFormat error!");

			if (!SetPixelFormat(hDC, pixelFormat, &pfd))
				DKERROR_THROW("SetPixelFormat failed.");

			HGLRC hTemp = wglCreateContext(hDC);
			DKASSERT_DESC(hTemp != NULL, "wglCreateContext failed.");

			wglMakeCurrent(hDC, hTemp);

			DKString version = (const char*)glGetString(GL_VERSION);
			if (version.Length())
			{
				openGLDeviceDescription.version = version;
				openGLDeviceDescription.major = version.Left(version.Find(L'.')).ToUnsignedInteger();
				version = version.Right(version.Find(L'.') + 1);
				openGLDeviceDescription.minor = version.Left(version.Find(L'.')).ToUnsignedInteger();
				openGLDeviceDescription.patch = version.Right(version.Find(L'.') + 1).ToUnsignedInteger();
			}
			else
			{
				openGLDeviceDescription.version = L"";
				openGLDeviceDescription.major = 0;
				openGLDeviceDescription.minor = 0;
				openGLDeviceDescription.patch = 0;
				DKLog("CRITICAL ERROR: glGetString(GL_VERSION) returns NULL\n");
			}
			openGLDeviceDescription.vendor = (const char*)glGetString(GL_VENDOR);
			DKLog("OpenGL Hardware Version = %d.%d.%d (%ls: %ls)\n",
				openGLDeviceDescription.major,
				openGLDeviceDescription.minor,
				openGLDeviceDescription.patch,
				openGLDeviceDescription.vendor,
				openGLDeviceDescription.version);
			////////////////////////////////////////////////////////////////////////////////
			// load WGL extensions (new style WGL context)
			//WGL_ARB_extensions_string
			GET_GL_EXT_PROC(wglGetExtensionsStringARB);
			//WGL_ARB_pixel_format
			GET_GL_EXT_PROC(wglGetPixelFormatAttribivARB);
			GET_GL_EXT_PROC(wglGetPixelFormatAttribfvARB);
			GET_GL_EXT_PROC(wglChoosePixelFormatARB);
			//WGL_ARB_make_current_read
			GET_GL_EXT_PROC(wglMakeContextCurrentARB);
			GET_GL_EXT_PROC(wglGetCurrentReadDCARB);
			//WGL_ARB_create_context
			GET_GL_EXT_PROC(wglCreateContextAttribsARB);
			//WGL_EXT_extensions_string
			GET_GL_EXT_PROC(wglGetExtensionsStringEXT);
			//WGL_EXT_make_current_read
			GET_GL_EXT_PROC(wglMakeContextCurrentEXT);
			GET_GL_EXT_PROC(wglGetCurrentReadDCEXT);
			//WGL_EXT_pixel_format
			GET_GL_EXT_PROC(wglGetPixelFormatAttribivEXT);
			GET_GL_EXT_PROC(wglGetPixelFormatAttribfvEXT);
			GET_GL_EXT_PROC(wglChoosePixelFormatEXT);
			//WGL_EXT_swap_control 
			GET_GL_EXT_PROC(wglSwapIntervalEXT);
			GET_GL_EXT_PROC(wglGetSwapIntervalEXT);

			wglMakeCurrent(0, 0);
			wglDeleteContext(hTemp);

			::ReleaseDC(hWnd, hDC);
			::DestroyWindow(hWnd);

			::UnregisterClassW(className, ::GetModuleHandleW(NULL));
			return true;
		}

		bool InitOpenGLExtensions(void);
	}
}

using namespace DKFramework;
using namespace DKFramework::Private;

DKOpenGLInterface* DKOpenGLInterface::CreateInterface(DKOpenGLContext*)
{
	return new DKOpenGLImpl();
}

#define OPENGL_CONTEXT_WINDOW_CLASS		L"Window for OpenGL Shared Context"

DKOpenGLImpl::DKOpenGLImpl(void)
	: mainContext(NULL)
	, window(NULL)
	, windowDC(NULL)
	, attribs(NULL)
{
	// init WGL extensions
	static bool initWGL = InitWGLExtensions();

	// create window to create context (a window for main-context)
	WNDCLASSW	wc = {CS_OWNDC, (WNDPROC)DefWindowProcW, 0, 0, GetModuleHandleW(NULL), 0, 0, 0, 0, OPENGL_CONTEXT_WINDOW_CLASS };
	if (!RegisterClassW(&wc))
		DKERROR_THROW("RegisterClass failed");
	window = CreateWindowExW(0, OPENGL_CONTEXT_WINDOW_CLASS, L"dummy", WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandleW(NULL), 0);
	DKASSERT_DESC(window != NULL, "Failed to create window.");

	windowDC = GetDC(window);
	DKASSERT_DESC(windowDC != NULL, "Failed to get DC.");

	pixelFormat = 0;
	const int iAttributes[] = {
		WGL_DRAW_TO_WINDOW_ARB,			GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,			GL_TRUE,
		WGL_ACCELERATION_ARB,			WGL_FULL_ACCELERATION_ARB,
		WGL_DOUBLE_BUFFER_ARB,			GL_TRUE,
		WGL_SWAP_METHOD_ARB,			WGL_SWAP_EXCHANGE_ARB,
		WGL_PIXEL_TYPE_ARB,				WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,				24,
		WGL_ALPHA_BITS_ARB,				8,
		WGL_DEPTH_BITS_ARB,				24,
		WGL_STENCIL_BITS_ARB,			0,
		0,0
	};
	UINT numOutFormats = 0;
	// setup pixel format
	if (!wglChoosePixelFormatARB(windowDC, iAttributes, NULL, 1, &pixelFormat, &numOutFormats) || numOutFormats < 1 || pixelFormat == 0)
		DKERROR_THROW("wglChoosePixelFormatARB failed.");

	// old-style pixel format
	memset(&pxielFormatDesc, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pxielFormatDesc.nSize		= sizeof(PIXELFORMATDESCRIPTOR);
	pxielFormatDesc.nVersion	= 1;
	pxielFormatDesc.dwFlags		= PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_SWAP_EXCHANGE | PFD_GENERIC_ACCELERATED;
	pxielFormatDesc.iPixelType	= PFD_TYPE_RGBA;
	pxielFormatDesc.cColorBits	= 32;
	pxielFormatDesc.cDepthBits	= 24;
	pxielFormatDesc.iLayerType	= PFD_MAIN_PLANE;

	if (!SetPixelFormat(windowDC , pixelFormat, &pxielFormatDesc))
		DKERROR_THROW("SetPixelFormat failed.");

	////////////////////////////////////////////////////////////////////////////
	// 2011-09-11 : set WGL_CONTEXT_MINOR_VERSION_ARB = 1, (Intel supports 3.1)
	// 2010-08-20 : WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB needed on ATI to use
	//              one-shader code for OpenGL, OpenGL ES together.
	// 2010-05-13 : OpenGL ES supports GL_ALPHA only for 1-channel texture,
	//              but OpenGL 3 supports GL_RED only for 1-channel.
	//              We have to use compatible mode for using both types.
	int wglAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB,		3,
		WGL_CONTEXT_MINOR_VERSION_ARB,		2,
		//WGL_CONTEXT_MINOR_VERSION_ARB,		1,
		//WGL_CONTEXT_FLAGS_ARB,				WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		//WGL_CONTEXT_PROFILE_MASK_ARB,		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB,		WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0	// end of the array
	};

	mainContext = wglCreateContextAttribsARB(windowDC, 0, wglAttribs);
	if (mainContext == NULL)	// retry with 3.1
	{
		DKLog("Warning: Failed to create OpenGL 3.2 context. Retry with OpenGL 3.1.\n");
		wglAttribs[3] = 1;		// retry with change value of WGL_CONTEXT_MINOR_VERSION_ARB (3.2 -> 3.1)
		mainContext = wglCreateContextAttribsARB(windowDC, 0, wglAttribs);
	}
	DKASSERT_DESC(mainContext != NULL, "CreateContext failed.\n");

	// copy attribs for other context.
	this->attribs = (int*)malloc(sizeof(wglAttribs));
	memcpy(this->attribs, wglAttribs, sizeof(wglAttribs));

	HGLRC oldRC = wglGetCurrentContext();
	HDC oldDC = wglGetCurrentDC();

	wglMakeCurrent(windowDC, mainContext);

	static bool initExtensions = InitOpenGLExtensions();

	wglMakeCurrent(oldDC, oldRC);
}

DKOpenGLImpl::~DKOpenGLImpl(void)
{
	wglMakeCurrent(0, 0);

	sharedContexts.EnumerateForward([](SharedContextMap::Pair& pair)
	{
		wglDeleteContext(pair.value.ctxt);
	});
	sharedContexts.Clear();

	// delete main context
	wglDeleteContext(mainContext);
	// destroy window
	ReleaseDC(window, windowDC);
	DestroyWindow(window);
	UnregisterClassW(OPENGL_CONTEXT_WINDOW_CLASS, GetModuleHandleW(NULL));

	free(attribs);
}

bool DKOpenGLImpl::IsBound(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(GetCurrentThreadId());
	if (p)
	{
		DKASSERT_DESC_DEBUG(p->value.ctxt == wglGetCurrentContext(), "Current Context Mismatch!");
		return true;
	}
	return false;
}

void DKOpenGLImpl::Bind(void* target) const
{
	DKASSERT_DEBUG(mainContext != NULL);

	DWORD currentThreadId = GetCurrentThreadId();

	SharedContextMap::Pair* p = sharedContexts.Find(currentThreadId);
	if (p)
	{
		if (target)
		{
			DKASSERT_DESC_DEBUG(IsWindow((HWND)target), "Target is invalid");
			DKASSERT_DESC_DEBUG(target == p->value.targetWindow, "Target mismatch!");
		}
		DKASSERT_DEBUG(p->value.count >= 0);
		p->value.count++;
	}
	else
	{
		// create new context with attribs (used by main context)
		HGLRC hGL = wglCreateContextAttribsARB(windowDC, mainContext, attribs);

		DKASSERT_DESC(hGL != NULL, "wglCreateContextAttribsARB failed!");

		HWND hWnd = NULL;
		if (target)
		{
			if (!IsWindow((HWND)target))
				DKERROR_THROW("Target is invalid");
			hWnd = (HWND)target;
		}
		else
		{
			hWnd = window;
		}
		HDC hDC = GetDC(hWnd);

		DKASSERT_DESC(hDC != NULL, "Failed to get DC");

		// set pixel format with hDC.
		int format = GetPixelFormat(hDC);
		if (format == 0)
		{
			if (!SetPixelFormat(hDC , pixelFormat, &pxielFormatDesc))
				DKERROR_THROW("SetPixelFormat failed.");
		}
		else if (pixelFormat != format)
		{
			DKERROR_THROW("Different pixel format.");
		}

		if (!wglMakeCurrent(hDC, hGL))
		{
			wglDeleteContext(hGL);
			DKERROR_THROW("wglMakeCurrent failed!");
		}

		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(0);

		BoundContext	bc = {hGL, hWnd, hDC, 1};
		if (!sharedContexts.Insert(currentThreadId, bc))
			DKERROR_THROW("SharedContext insert failed!");
	}
}

void DKOpenGLImpl::Unbind(void) const
{
	DWORD currentThreadId = GetCurrentThreadId();

	SharedContextMap::Pair* p = sharedContexts.Find(currentThreadId);
	if (p)
	{
		DKASSERT_DEBUG(p->value.count >= 0);
		p->value.count--;
		if (p->value.count == 0)
		{
			glFinish();

			wglMakeCurrent(0, 0);
			wglDeleteContext(p->value.ctxt);
			ReleaseDC(p->value.targetWindow, p->value.targetDC);
			sharedContexts.Remove(currentThreadId);
		}
	}
	else
	{
		DKERROR_THROW_DEBUG("cannot find context");
	}
}

void DKOpenGLImpl::Flush(void) const
{
#ifdef DKLIB_DEBUG_ENABLED
	SharedContextMap::Pair* p = sharedContexts.Find(GetCurrentThreadId());
	if (p)
		glFlush();
	else
		DKLog("Error: No context for this thread(%x)\n", GetCurrentThreadId());
#else
	glFlush();
#endif
}

void DKOpenGLImpl::Finish(void) const
{
#ifdef DKLIB_DEBUG_ENABLED
	SharedContextMap::Pair* p = sharedContexts.Find(GetCurrentThreadId());
	if (p)
		glFinish();
	else
		DKLog("Error: No context for this thread(%x)\n", GetCurrentThreadId());
#else
	glFinish();
#endif
}

void DKOpenGLImpl::Present(void) const
{
#ifdef DKLIB_DEBUG_ENABLED
	SharedContextMap::Pair* p = sharedContexts.Find(GetCurrentThreadId());
	if (p)
	{
		HDC hDC = wglGetCurrentDC();
		if (hDC != NULL && hDC == p->value.targetDC)
		{
			if (!SwapBuffers(hDC))
				DKLog("SwapBuffers failed.\n");
		}
		else
		{
			DKLog("Error: Bound DC is different or NULL!\n");
		}
	}
	else
	{
		DKLog("Error: No context for this thread(%x)\n", GetCurrentThreadId());
	}
#else
	HDC hDC = wglGetCurrentDC();
	if (hDC)
	{
		if (!SwapBuffers(hDC))
			DKLog("SwapBuffers failed.\n");
	}
#endif
}

void DKOpenGLImpl::Update(void) const
{
}

bool DKOpenGLImpl::GetSwapInterval(void) const
{
	if (wglGetSwapIntervalEXT)
		return wglGetSwapIntervalEXT() != 0;
	return 0;
}

void DKOpenGLImpl::SetSwapInterval(bool interval) const
{
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(interval);
}

#endif	// ifdef _WIN32

