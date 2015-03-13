//
//  File: DKRenderTarget.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKMath.h"
#include "DKRenderTarget.h"
#include "DKOpenGLContext.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		GLenum GetTextureFormatGLValue(DKFramework::DKTexture::Format f);
		GLenum GetTextureInternalFormatGLValue(DKFramework::DKTexture::Format f);
		DKFramework::DKTexture::Format GetTextureFormat(GLenum f);

		GLint GetMaxTextureSize(void);		// defined in DKTexture.cpp

		GLint GetMaxDrawBuffers(void)
		{
#ifdef GL_MAX_DRAW_BUFFERS	
			static GLint maxDrawBuffers = 0;
			if (maxDrawBuffers == 0)	
				glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

			DKASSERT_DEBUG(maxDrawBuffers > 0);
			return maxDrawBuffers;
#endif
			return 1;
		}

		GLint GetMaxColorAttachments(void)
		{
#ifdef GL_MAX_COLOR_ATTACHMENTS
			static GLint maxColorAttachments = 0;
			if (maxColorAttachments == 0)
				glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
			DKASSERT_DEBUG(maxColorAttachments > 0);
			return maxColorAttachments;
#endif
			return 1;
		}

		namespace
		{
			typedef const DKRenderTarget* FBOSearchKey;
			typedef DKMap<FBOSearchKey, GLuint> FBOMap;
			typedef DKArray<GLuint> FBOArray;
			struct ThreadFBOs
			{
				FBOMap			renderTargets;
				FBOArray		removedFBOs;
			};

			typedef DKMap<DKThread::ThreadId, ThreadFBOs> ThreadFBOMap;
			ThreadFBOMap threadFBOMap;
			DKSpinLock fboMapLock;
		}

		// 2012-08-10 by Hongtae Kim.
		// Cannot share FOB between threads, a thread should have its own FBO.
		// Common workflow as following descriptions.
		//  - call FindFramebuffer(), if 0 returned, setup new FBO.
		//  - registering new FBO with SetFramebuffer()
		//  - if FBO does not needed anymore, call UnsetFramebuffer() to remove
		// FBO will not destroyed immediately in spite of DKRenderTarget object
		//  has ben destoryed.

		// FindFramebuffer
		// find framebuffer for current thread. (created by current thread)
		GLuint FindFramebuffer(FBOSearchKey ctxt)
		{
			DKCriticalSection<DKSpinLock> guard(fboMapLock);
			ThreadFBOMap::Pair* p = threadFBOMap.Find(DKThread::CurrentThreadId());
			if (p)
			{
				if (p->value.removedFBOs.Count() > 0)
				{
					glDeleteFramebuffers((GLsizei)p->value.removedFBOs.Count(), (GLuint*)p->value.removedFBOs);
					p->value.removedFBOs.Clear();
				}

				FBOMap::Pair* p2 = p->value.renderTargets.Find(ctxt);
				if (p2)
					return p2->value;
			}
			return 0;
		}

		// SetFramebuffer
		// set new framebuffer object to current thread. (created by current thread)
		void SetFramebuffer(FBOSearchKey ctxt, GLuint fbo)
		{
			DKCriticalSection<DKSpinLock> guard(fboMapLock);
			ThreadFBOs& tf = threadFBOMap.Value(DKThread::CurrentThreadId());
			DKASSERT_DEBUG( tf.renderTargets.Find(ctxt) == NULL );

			tf.renderTargets.Update(ctxt, fbo);
			if (tf.removedFBOs.Count() > 0)
			{
				glDeleteFramebuffers((GLsizei)tf.removedFBOs.Count(), (GLuint*)tf.removedFBOs);
				tf.removedFBOs.Clear();
			}
		}

		// UnsetFramebuffer
		// remove all FBO objects from all threads.
		// FBO object will be removed by owner thread later.
		void UnsetFramebuffer(FBOSearchKey ctxt)
		{
			DKCriticalSection<DKSpinLock> guard(fboMapLock);

			auto remover = [ctxt](ThreadFBOMap::Pair& pair)
			{
				FBOMap::Pair* p = pair.value.renderTargets.Find(ctxt);
				if (p)
				{
					pair.value.removedFBOs.Add(p->value);
					pair.value.renderTargets.Remove(ctxt);
				}
			};

			threadFBOMap.EnumerateForward(remover);
		}

		// remove FBOs which is no longer needed for current thread
		// (used by DKRenderTarget, which has been destroyed)
		void ClearUnusedFramebuffers(void)
		{
			DKCriticalSection<DKSpinLock> guard(fboMapLock);
			ThreadFBOMap::Pair* p = threadFBOMap.Find(DKThread::CurrentThreadId());
			if (p)
			{
				if (p->value.removedFBOs.Count() > 0)
				{
					glDeleteFramebuffers((GLsizei)p->value.removedFBOs.Count(), (GLuint*)p->value.removedFBOs);
					p->value.removedFBOs.Clear();
				}

				if (p->value.renderTargets.Count() == 0)
				{
					threadFBOMap.Remove(p->key);
				}
			}
		}

		// remove all FBOs from current thread.
		void ClearFramebuffers(void)
		{
			DKCriticalSection<DKSpinLock> guard(fboMapLock);
			ThreadFBOMap::Pair* p = threadFBOMap.Find(DKThread::CurrentThreadId());
			if (p)
			{
				p->value.removedFBOs.Reserve( p->value.removedFBOs.Count() + p->value.renderTargets.Count() );
				p->value.renderTargets.EnumerateForward([p](FBOMap::Pair& pair) {p->value.removedFBOs.Add(pair.value);});

				if (p->value.removedFBOs.Count() > 0)
				{
					glDeleteFramebuffers((GLsizei)p->value.removedFBOs.Count(), (GLuint*)p->value.removedFBOs);
					p->value.removedFBOs.Clear();
				}

				threadFBOMap.Remove(p->key);
			}
		}
	}
}

using namespace DKFramework;

DKRenderTarget::DKRenderTarget(void)
: proxyQuery(NULL)
, depthFormat(DepthFormatNone)
, depthBuffer(0)
, depthResolution(1, 1)
{
}

DKObject<DKRenderTarget> DKRenderTarget::Create(int width, int height, DepthFormat depth)
{
	int maxTexSize = Private::GetMaxTextureSize();
	if (width < 0 || width > maxTexSize || height < 0 || height > maxTexSize)
		return NULL;

	DKObject<DKTexture2D> tex = DKTexture2D::Create(width, height, DKTexture::FormatRGBA, DKTexture::TypeUnsignedByte, 0);
	if (tex == NULL)
		return NULL;

	DKTexture2D* textures[] = { (DKTexture2D*)tex };

	DKObject<DKRenderTarget> target = DKObject<DKRenderTarget>::New();
	target->SetColorTextures(textures, 1);
	if (target->SetDepthBuffer(width, height, depth))
		return target;

	return NULL;
}

DKObject<DKRenderTarget> DKRenderTarget::Create(ProxyQuery* proxy)
{
	if (proxy)
	{
		DKObject<DKRenderTarget> target = DKObject<DKRenderTarget>::New();
		target->proxyQuery = proxy;
		return target;
	}
	return NULL;
}

DKRenderTarget::~DKRenderTarget(void)
{
	Private::UnsetFramebuffer(this);
	Private::ClearUnusedFramebuffers();

	if (depthBuffer)
		glDeleteRenderbuffers(1, &depthBuffer);
}

DKSize DKRenderTarget::Resolution(void) const
{
	if (this->proxyQuery)
	{
		TargetQuery q = this->proxyQuery->Invoke();
		int w = Max(q.width, 1);
		int h = Max(q.height, 1);
		return DKSize(w, h);
	}
	if (this->colorTextures.Count() > 0)
	{
		int w = Max<int>(this->colorTextures.Value(0)->Width(), 1);
		int h = Max<int>(this->colorTextures.Value(0)->Height(), 1);
		return DKSize(w, h);
	}
	return depthResolution;
}

bool DKRenderTarget::SetColorTextures(DKTexture2D** tex, size_t num)
{
	if (this->proxyQuery)
		return false;

	if (this->colorTextures.IsEmpty() && num == 0)
		return true;

	for (size_t i = 0; i < num; ++i)
	{
		if (tex[i] == NULL)
			return false;

		switch (tex[i]->TextureFormat())
		{
		case DKTexture::FormatAlpha:
		case DKTexture::FormatRGB:
		case DKTexture::FormatRGBA:
			break;

		default:
			// invalid format!
			return false;
		}
	}

	this->colorTextures.Clear();
	this->colorTextures.Reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		DKTexture2D* t = tex[i];
		DKASSERT_DEBUG(t);
		this->colorTextures.Add(t);
	}

	DKRenderState& state = DKOpenGLContext::RenderState();
	state.BindFrameBuffer(0);

	Private::UnsetFramebuffer(this);
	Private::ClearUnusedFramebuffers();

	return true;
}

DKTexture2D* DKRenderTarget::ColorTexture(int index)
{
	if (index >= 0 && index < colorTextures.Count())
		return colorTextures.Value(index);
	return NULL;
}

const DKTexture2D* DKRenderTarget::ColorTexture(int index) const
{
	if (index >= 0 && index < colorTextures.Count())
		return colorTextures.Value(index);
	return NULL;
}

size_t DKRenderTarget::NumberOfColorTextures(void) const
{
	return colorTextures.Count();
}

size_t DKRenderTarget::MaxColorTextures(void)
{
	size_t maxTextures = Private::GetMaxColorAttachments();
	size_t maxDrawBuff = Private::GetMaxDrawBuffers();
	return Min(maxTextures, maxDrawBuff);
}

bool DKRenderTarget::SetDepthTexture(DKTexture2D* tex)
{
	if (this->proxyQuery)
		return false;

	if (tex)
	{
		if (tex != this->depthTexture)
		{
			switch (tex->TextureFormat())
			{
			case DKTexture::FormatDepth16:
				this->depthFormat = DepthFormat16;
				break;
			case DKTexture::FormatDepth24:
				this->depthFormat = DepthFormat24;
				break;
			case DKTexture::FormatDepth32:
				this->depthFormat = DepthFormat32;
				break;
			default:
				//DKASSERT_DEBUG(0, "DKRenderTarget::ResetDepthTexture Error: Invalid texture!\n");
				//DKLog("DKRenderTarget::ResetDepthTexture Error: Invalid texture!\n");
				return false;
			}

			DKRenderState& state = DKOpenGLContext::RenderState();
			state.BindFrameBuffer(0);

			if (this->depthBuffer)
				glDeleteRenderbuffers(1, &this->depthBuffer);
			this->depthBuffer = 0;

			Private::UnsetFramebuffer(this);
			Private::ClearUnusedFramebuffers();
		}
	}
	else
	{
		if (this->depthTexture || this->depthBuffer)
		{
			this->depthFormat = DepthFormatNone;

			DKRenderState& state = DKOpenGLContext::RenderState();
			state.BindFrameBuffer(0);

			if (this->depthBuffer)
				glDeleteRenderbuffers(1, &this->depthBuffer);
			this->depthBuffer = 0;

			Private::UnsetFramebuffer(this);
			Private::ClearUnusedFramebuffers();
		}
	}
	this->depthTexture = tex;
	if (this->depthTexture)
		depthResolution = this->depthTexture->Resolution();
	else
		depthResolution = DKSize(1,1);

	return true;
}

DKTexture2D* DKRenderTarget::DepthTexture(void)
{
	return depthTexture;
}

const DKTexture2D* DKRenderTarget::DepthTexture(void) const
{
	return depthTexture;
}

bool DKRenderTarget::SetDepthBuffer(int width, int height, DepthFormat df)
{
	if (this->proxyQuery)
		return false;

	switch (df)
	{
	case DepthFormatNone:
	case DepthFormat16:
	case DepthFormat24:
	case DepthFormat32:
		break;
	default:
		//DKASSERT_DEBUG(0, "Invalid format!");
		return false;
	}

	if (df != DepthFormatNone && (width < 1 || height < 1))
		return false;

	if (this->depthFormat != df || this->depthTexture)
	{
		DKRenderState& state = DKOpenGLContext::RenderState();
		state.BindFrameBuffer(0);

		this->depthFormat = df;

		GLenum format = 0;
		switch (this->depthFormat)
		{
		case DepthFormat16:
			format = Private::GetTextureInternalFormatGLValue(DKTexture::FormatDepth16);
			break;
		case DepthFormat24:
			format = Private::GetTextureInternalFormatGLValue(DKTexture::FormatDepth24);
			break;
		case DepthFormat32:
			format = Private::GetTextureInternalFormatGLValue(DKTexture::FormatDepth32);
			break;
		}

		this->depthTexture = NULL;

		if (format)
		{
			if (this->depthBuffer == 0)
				glGenRenderbuffers(1, &this->depthBuffer);

			DKASSERT_DEBUG(width > 0 && height > 0);

			glBindRenderbuffer(GL_RENDERBUFFER, this->depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			this->depthResolution = DKSize(width, height);
		}
		else
		{
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			if (this->depthBuffer)
				glDeleteRenderbuffers(1, &this->depthBuffer);
			this->depthBuffer = 0;
			this->depthResolution = DKSize(1,1);
		}

		Private::UnsetFramebuffer(this);
		Private::ClearUnusedFramebuffers();
	}
	return true;
}

DKRenderTarget::DepthFormat DKRenderTarget::DepthBufferFormat(void) const
{
	return this->depthFormat;
}

bool DKRenderTarget::IsValid(void) const
{
	if (this->proxyQuery)
		return true;

	GLuint framebuffer = Private::FindFramebuffer(this);
	if (framebuffer)
		return true;

	return const_cast<DKRenderTarget*>(this)->Validate();
}

bool DKRenderTarget::IsProxy(void) const
{
	return this->proxyQuery != NULL;
}

bool DKRenderTarget::Validate(void)
{
	if (this->proxyQuery)
	{
		DKASSERT_DEBUG(this->colorTextures.Count() == 0);
		DKASSERT_DEBUG(this->depthBuffer == 0);
		DKASSERT_DEBUG(this->depthTexture == NULL);
		return true;
	}
	
	GLuint framebuffer = Private::FindFramebuffer(this);
	if (framebuffer == 0)
	{
		DKRenderState& state = DKOpenGLContext::RenderState();

		state.BindFrameBuffer(0);
		Private::ClearUnusedFramebuffers();
		
		glGenFramebuffers(1, &framebuffer);
		state.BindFrameBuffer(framebuffer);
		
		GLenum depthFmt = 0;
		switch (this->depthFormat)
		{
		case DepthFormat16:
			depthFmt = Private::GetTextureInternalFormatGLValue(DKTexture::FormatDepth16);
			break;
		case DepthFormat24:
			depthFmt = Private::GetTextureInternalFormatGLValue(DKTexture::FormatDepth24);
			break;
		case DepthFormat32:
			depthFmt = Private::GetTextureInternalFormatGLValue(DKTexture::FormatDepth32);
			break;
		}
		
		if (this->depthFormat != DepthFormatNone)
		{
			if (this->depthTexture)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthTexture->resourceId, 0);
			}
			else if (this->depthBuffer)
			{
				// attach render buffer to frame buffer
				glBindRenderbuffer(GL_RENDERBUFFER, this->depthBuffer);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->depthBuffer);
			}
		}
		
#ifdef GL_MAX_DRAW_BUFFERS
		DKArray<GLenum> drawTargets;
		drawTargets.Reserve(this->colorTextures.Count());
#endif
		
		for (size_t i = 0; i < this->colorTextures.Count(); ++i)
		{
			const DKTexture2D* tex = this->colorTextures.Value(i);
			// attach color-buffer to framebuffer.
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex->resourceId, 0);
			//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex->resourceId, 0);
			
#ifdef GL_MAX_DRAW_BUFFERS
			drawTargets.Add(GL_COLOR_ATTACHMENT0 + i);
#endif
		}

#ifdef GL_MAX_DRAW_BUFFERS
		if (drawTargets.IsEmpty())
		{
			GLenum bufs[] = { GL_NONE };
			glDrawBuffers(1, bufs);
			glReadBuffer(GL_NONE);
		}
		else
		{
			glDrawBuffers(drawTargets.Count(), (const GLenum*)drawTargets);
			glReadBuffer(drawTargets.Value(0));
		}
#else
#ifndef DKLIB_OPENGL_ES		// OpenGL ES does not have glDrawBuffer, glReadBuffer.
		if (this->colorTextures.IsEmpty())
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		else
		{
			if (this->colorTextures.Count() > 1)
				DKLog("Warning: glDrawBuffers not supported! (MRTs not supported.)\n");
			glDrawBuffer(drawTargets.Value(0));
			glReadBuffer(drawTargets.Value(0));
		}
#endif
#endif
		
		// check FBO status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status == GL_FRAMEBUFFER_COMPLETE)
		{
			Private::SetFramebuffer(this, framebuffer);
			
			//state.ColorMask(1,1,1,1);
			//state.DepthMask(true);
			//state.Viewport(0, 0, floor(resolution.width + 0.5), floor(resolution.height + 0.5));
		}
		else
		{
			DKLog("glCheckFramebufferStatus returns: %x\n", status);
			DKLog("DKRenderTarget::Validate failed.\n");
			return false;
		}
	}
	return true;
}

bool DKRenderTarget::Bind(void) const
{
	if ( const_cast<DKRenderTarget*>(this)->Validate())
	{
		DKRenderState& state = DKOpenGLContext::RenderState();
		
		if (this->proxyQuery)
		{
			TargetQuery tq = this->proxyQuery->Invoke();
			state.BindFrameBuffer(tq.fbo);
		}
		else
		{
			GLuint fbo = Private::FindFramebuffer(this);
			DKASSERT_DEBUG(fbo != 0);
			state.BindFrameBuffer(fbo);
		}
		return true;
	}
	return false;
}

bool DKRenderTarget::CopyColorTexture(int target, DKTexture2D* tex) const
{
	if (this->proxyQuery)
		return false;

	if (tex == NULL || !tex->IsValid() || !this->IsValid())
		return false;

	if (target >= 0 && colorTextures.Count() > target)
	{
		const DKTexture* src = colorTextures.Value(target);
		if (src->Resolution() == tex->Resolution())
		{
			this->Bind();

			tex->Bind();

			glCopyTexSubImage2D(tex->target, 0, 0, 0, 0, 0, src->Width(), src->Height());

			return true;
		}
	}
	return false;
}

bool DKRenderTarget::CopyDepthTexture(DKTexture2D* tex) const
{
	if (this->proxyQuery)
		return false;

	DKERROR_THROW("Not Implemented Yet");
	return false;
}
