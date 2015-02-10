//
//  File: DKOpenGLContext.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKWindow.h"
#include "DKRenderState.h"

////////////////////////////////////////////////////////////////////////////////
// DKOpenGLContext
// OpenGL context (OpenGL, OpenGL ES)
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKOpenGLInterface;
	template <typename T> class DKContextScopeBinder
	{
	public:
		DKContextScopeBinder(T* ctxt) : context(ctxt)	{ if (context) context->Bind(); }
		~DKContextScopeBinder(void)						{ if (context) context->Unbind();}
		T* operator -> (void)							{return context;}
		const T* operator -> (void) const				{return context;}
	private:
		DKFoundation::DKObject<T> context;
	};
	template <typename T, typename R> class DKContextScopeBinderWithParam
	{
	public:
		DKContextScopeBinderWithParam(T* ctxt, R p) : context(ctxt)		{ if (context) context->Bind(p); }
		~DKContextScopeBinderWithParam(void)							{ if (context) context->Unbind();}
		T* operator -> (void)											{return context;}
		const T* operator -> (void) const								{return context;}
	private:
		DKFoundation::DKObject<T> context;
	};

	class DKLIB_API DKOpenGLContext : public DKFoundation::DKSharedInstance<DKOpenGLContext>
	{
	public:
		~DKOpenGLContext(void);

		bool IsBound(void) const;
		void Bind(const DKWindow* window = NULL) const;	// bind current thread to this context.
		void Unbind(void) const;						// unbind

		void Flush(void) const;		// glFlush, error check on debug mode.
		void Finish(void) const;	// glFinish, error check on debug mode.
		void Present(void) const;	// swap buffer.
		void Update(void) const;	// update view. (of window)

		bool GetSwapInterval(void) const;
		void SetSwapInterval(bool interval) const;

		static DKRenderState& RenderState(void);

		// framebuffer(FBO) id for current bound window.
		unsigned int FramebufferId(void) const;
		
	private:
		friend class DKFoundation::DKObject<DKOpenGLContext>;
		friend class DKFoundation::DKSharedInstance<DKOpenGLContext>;
		DKOpenGLContext(void);
		DKOpenGLContext(const DKOpenGLContext&);
		DKOpenGLContext& operator = (const DKOpenGLContext&);
		
		typedef DKFoundation::DKMap<DKFoundation::DKThread::ThreadId, DKFoundation::DKObject<DKRenderState>> RenderStateMap;
		static RenderStateMap stateMap;
		static DKFoundation::DKSpinLock stateLock;
		DKOpenGLInterface* impl; // core-interface
	};
}
