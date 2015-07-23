//
//  File: DKRenderState.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKRenderState.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		namespace
		{
			inline GLenum GetGLValue(DKRenderState::GLState st)
			{
				switch (st)
				{
				case DKRenderState::GLStateBlend:				return GL_BLEND;
				case DKRenderState::GLStateDepthTest:			return GL_DEPTH_TEST;
				case DKRenderState::GLStateCullFace:			return GL_CULL_FACE;
				case DKRenderState::GLStatePolygonOffsetFill:	return GL_POLYGON_OFFSET_FILL;
				}
				return 0;
			}
		}

		void ClearFramebuffers(void);		// defined in DKRenderTarget.cpp
	}
}

using namespace DKFramework;


DKRenderState::DKRenderState(void)
	: maxCombinedTextureImageUnits(0)	
	, maxVertexAttribs(0)					
	, maxTextureSize(0)						
	, maxColorAttachments(0)
	, defaultVertexArray(0)
{
	memset(glStates, 0, sizeof(glStates));
}

DKRenderState::~DKRenderState(void)
{
	if (defaultVertexArray)
		glDeleteVertexArrays(1, &defaultVertexArray);

	Private::ClearFramebuffers();
}

void DKRenderState::Reset(void)
{
	if (defaultVertexArray)
		glDeleteVertexArrays(1, &defaultVertexArray);
	defaultVertexArray = 0;

	Private::ClearFramebuffers();

	maxCombinedTextureImageUnits = 0;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureImageUnits);
	DKASSERT_DEBUG(maxCombinedTextureImageUnits > 0);

	maxVertexAttribs = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	DKASSERT_DEBUG(maxVertexAttribs > 0);

	maxTextureSize = 0;
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	DKASSERT_DEBUG(maxTextureSize > 0);

	maxColorAttachments = 1;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
	DKASSERT_DEBUG(maxColorAttachments > 0);

	memset(glStates, 0, sizeof(glStates));

	glGenVertexArrays(1, &defaultVertexArray);
	glBindVertexArray(0);

	frameBuffer = 0;
	vertexArray = defaultVertexArray;
	program = 0;
	blendFuncRGB = GL_FUNC_ADD;
	blendFuncAlpha = GL_FUNC_ADD;
	blendSrcRGB = GL_ONE;
	blendSrcAlpha = GL_ONE;
	blendDstRGB = GL_ZERO;
	blendDstAlpha = GL_ZERO;
	depthMask = 0;
	depthFunc = GL_ALWAYS;
	activeTexture = 0;
	frontFace = GLFrontFaceCCW;
	cullFace = GLCullFaceBack;
	clearColor[0] = clearColor[1] = clearColor[2] = clearColor[3] = 0.0f;
	blendColor[0] = blendColor[1] = blendColor[2] = blendColor[3] = 0.0f;
	colorMask[0] = colorMask[1] = colorMask[2] = colorMask[3] = true;
	viewport[0] = viewport[1] = 0;
	viewport[2] = viewport[3] = 1;
	clearDepth = 1.0;
	depthRangeNear = 0.0;
	depthRangeFar = 1.0;
	polygonOffsetFactor = 0.0;
	polygonOffsetUnits = 0.0;
	lineWidth = 1.0;

	TextureUnit tu = {0,0};
	activeTextures = DKArray<TextureUnit>(tu, maxCombinedTextureImageUnits);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(program);
	glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
	glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	glBlendColor(blendColor[0], blendColor[1], blendColor[2], blendColor[3]);
	glBlendFunc(GL_ONE, GL_ZERO);
	glBlendEquation(GL_FUNC_ADD);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glDepthMask(depthMask);
	glDepthFunc(depthFunc);
	
#ifdef DKGL_OPENGL_ES
	glClearDepthf(clearDepth);
	glDepthRangef(depthRangeNear, depthRangeFar);
#else
	glClearDepth(clearDepth);
	glDepthRange(depthRangeNear, depthRangeFar);
#endif

	glPolygonOffset(polygonOffsetFactor, polygonOffsetUnits);

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glLineWidth(lineWidth);

	Disable(GLStateCullFace);
	glDisable(GL_CULL_FACE);
	Disable(GLStateBlend);
	glDisable(GL_BLEND);
	Disable(GLStateDepthTest);
	glDisable(GL_DEPTH_TEST);
	Disable(GLStatePolygonOffsetFill);
	glDisable(GL_POLYGON_OFFSET_FILL);
	
	// 2011-07-05 by Hongtae Kim
	// DO NOT USE  GL_POINT_SMOOTH, GL_LINE_SMOOTH, GL_POLYGON_SMOOTH
	// AIT has bug. (very very slow)
	vertexArrays.Clear();
	VertexArray& va = vertexArrays.Value(vertexArray);
	va.vertexBuffer = 0;
	va.indexBuffer = 0;
	va.attribArrayEnabled = DKArray<bool>(false, maxVertexAttribs);
	for (int i = 0; i < maxVertexAttribs; i++)
		glDisableVertexAttribArray(i);
	
	unsigned int err = GetError();
	if (err != GL_NO_ERROR)
	{
		DKLog("DKRenderState::Reset Error:%x (%s)\n", err, GetErrorString(err));
	}

	glFlush();
}

void DKRenderState::Enable(GLState mode)
{
	unsigned int uval = static_cast<unsigned int>(mode);
	if (uval > GLStateUnused && uval < GLStateMaxValue)
	{
		unsigned int vIdx = uval / 8;
		unsigned char vMask = 0x01U << (uval % 8);

		DKASSERT_DEBUG(vIdx < GLStateLength);

		bool enabled = (glStates[vIdx] & vMask) != 0x00;

		if (!enabled)
		{
			GLenum val = Private::GetGLValue(mode);
			if (val)
				glEnable(val);
			else
				DKLog("DKRenderState::Enable Unsupported target: 0x%x\n", uval);

			glStates[vIdx] |= vMask;
		}
	}
}

void DKRenderState::Disable(GLState mode)
{
	unsigned int uval = static_cast<unsigned int>(mode);
	if (uval > GLStateUnused && uval < GLStateMaxValue)
	{
		unsigned int vIdx = uval / 8;
		unsigned char vMask = 0x01U << (uval % 8);

		DKASSERT_DEBUG(vIdx < GLStateLength);

		bool enabled = (glStates[vIdx] & vMask) != 0x00;

		if (enabled)
		{
			GLenum val = Private::GetGLValue(mode);
			if (val)
				glDisable(val);
			else
				DKLog("DKRenderState::Disable Unsupported target: 0x%x\n", uval);

			glStates[vIdx] &= ~vMask;
		}
	}
}

bool DKRenderState::IsEnabled(GLState mode)
{
	unsigned int uval = static_cast<unsigned int>(mode);
	if (uval > GLStateUnused && uval < GLStateMaxValue)
	{
		unsigned int vIdx = uval / 8;
		unsigned char vMask = 0x01U << (uval % 8);

		DKASSERT_DEBUG(vIdx < GLStateLength);

		bool enabled = (glStates[vIdx] & vMask) != 0x00;

		return enabled;
	}
	return false;
}

void DKRenderState::UseProgram(unsigned int id)
{
	if (id == program)
		return;

	glUseProgram(id);
	program = id;
}

void DKRenderState::BindVertexArray(unsigned int id)
{
	if (id == vertexArray)
		return;

	if (id == 0)
		id = defaultVertexArray;

	DKMap<unsigned int, VertexArray>::Pair* p = vertexArrays.Find(id);
	if (p)
	{
		glBindVertexArray(id);
	}
	else
	{
		DKASSERT_DEBUG(glIsVertexArray(id) != 0);	// target is not VAO!

		VertexArray& a = vertexArrays.Value(id);
		glBindVertexArray(id);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		a.vertexBuffer = 0;
		a.indexBuffer = 0;
		int maxArray = 0;

		a.attribArrayEnabled = DKArray<bool>(false, maxVertexAttribs);
		for (int i = 0; i < maxVertexAttribs; i++)
			glDisableVertexAttribArray(i);
	}
	vertexArray = id;
}

void DKRenderState::ClearVertexArray(unsigned int id)
{
	if (vertexArray == id)
		BindVertexArray(0);

	vertexArrays.Remove(id);
}

void DKRenderState::BindFrameBuffer(unsigned int id)
{
	if (id == frameBuffer)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	frameBuffer = id;
}

void DKRenderState::BindVertexBuffer(unsigned int id)
{
	VertexArray& va = vertexArrays.Value(vertexArray);
	if (va.vertexBuffer == id)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, id);
	va.vertexBuffer = id;
}

void DKRenderState::BindIndexBuffer(unsigned int id)
{
	VertexArray& va = vertexArrays.Value(vertexArray);
	if (va.indexBuffer == id)
		return;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	va.indexBuffer = id;
}

void DKRenderState::ActiveTexture(unsigned int index)
{
	if (index >= activeTextures.Count())
		index = 0;
	if (activeTexture == index)
		return;

	glActiveTexture(GL_TEXTURE0 + index);
	activeTexture = index;
}

void DKRenderState::BindTexture(unsigned int target, unsigned int id)
{
	if (activeTextures.Value(activeTexture).target == target && 
		activeTextures.Value(activeTexture).id == id)
		return;

	glBindTexture(target, id);
	activeTextures.Value(activeTexture).target = target;
	activeTextures.Value(activeTexture).id = id;
}

void DKRenderState::EnableVertexAttribArray(unsigned int index)
{
	VertexArray& va = vertexArrays.Value(vertexArray);
	if (index >= va.attribArrayEnabled.Count() || va.attribArrayEnabled.Value(index))
		return;

	glEnableVertexAttribArray(index);
	va.attribArrayEnabled.Value(index) = true;
}

void DKRenderState::DisableVertexAttribArray(unsigned int index)
{
	VertexArray& va = vertexArrays.Value(vertexArray);
	if (index >= va.attribArrayEnabled.Count() || !va.attribArrayEnabled.Value(index))
		return;

	glDisableVertexAttribArray(index);
	va.attribArrayEnabled.Value(index) = false;
}

void DKRenderState::ClearColor(float r, float g, float b, float a)
{
	r = Clamp(r, 0.0, 1.0);
	g = Clamp(g, 0.0, 1.0);
	b = Clamp(b, 0.0, 1.0);
	a = Clamp(a, 0.0, 1.0);

	if (clearColor[0] == r && clearColor[1] == g && clearColor[2] == b && clearColor[3] == a)
		return;

	glClearColor(r,g,b,a);
	clearColor[0] = r;
	clearColor[1] = g;
	clearColor[2] = b;
	clearColor[3] = a;
}

void DKRenderState::ClearDepth(float d)
{
	d = Clamp(d, 0.0, 1.0);

	if (clearDepth == d)
		return;

#ifdef DKGL_OPENGL_ES
	glClearDepthf(d);
#else
	glClearDepth(d);
#endif

	clearDepth = d;
}

void DKRenderState::BlendEquation(unsigned int funcRGB, unsigned int funcAlpha)
{
	if (funcRGB == blendFuncRGB && funcAlpha == blendFuncAlpha)
		return;

	if (funcRGB == funcAlpha)
		glBlendEquation(funcRGB);
	else
		glBlendEquationSeparate(funcRGB, funcAlpha);

	blendFuncRGB = funcRGB;
	blendFuncAlpha = funcAlpha;
}

void DKRenderState::BlendFunc(unsigned int srcRGB, unsigned int srcAlpha, unsigned int dstRGB, unsigned int dstAlpha)
{
	if (srcRGB == blendSrcRGB && srcAlpha == blendSrcAlpha &&
		dstRGB == blendDstRGB && dstAlpha == blendDstAlpha)
		return;

	if (srcRGB == srcAlpha && dstRGB == dstAlpha)
		glBlendFunc(srcRGB, dstRGB);
	else
		glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);

	blendSrcRGB = srcRGB;
	blendSrcAlpha = srcAlpha;
	blendDstRGB = dstRGB;
	blendDstAlpha = dstAlpha;
}

void DKRenderState::BlendColor(float r, float g, float b, float a)
{
	r = Clamp(r, 0.0, 1.0);
	g = Clamp(g, 0.0, 1.0);
	b = Clamp(b, 0.0, 1.0);
	a = Clamp(a, 0.0, 1.0);

	if (blendColor[0] == r && blendColor[1] == g && blendColor[2] == b && blendColor[3] == a)
		return;

	glBlendColor(r,g,b,a);
	blendColor[0] = r;
	blendColor[1] = g;
	blendColor[2] = b;
	blendColor[3] = a;
}

void DKRenderState::DepthFunc(unsigned int func)
{
	if (depthFunc == func)
		return;

	glDepthFunc(func);
	depthFunc = func;
}

void DKRenderState::DepthMask(bool mask)
{
	if (depthMask == mask)
		return;

	glDepthMask(mask);
	depthMask = mask;
}

void DKRenderState::ColorMask(bool r, bool g, bool b, bool a)
{
	if (colorMask[0] == r && colorMask[1] == g && colorMask[2] == b && colorMask[3] == a)
		return;

	glColorMask(r,g,b,a);
	colorMask[0] = r;
	colorMask[1] = g;
	colorMask[2] = b;
	colorMask[3] = a;
}

void DKRenderState::CullFace(GLCullFace face)
{
	if (cullFace == face)
		return;

	switch (face)
	{
	case GLCullFaceFront:	glCullFace(GL_FRONT);			cullFace = face;	break;
	case GLCullFaceBack:	glCullFace(GL_BACK);			cullFace = face;	break;
	case GLCullFaceBoth:	glCullFace(GL_FRONT_AND_BACK);	cullFace = face;	break;
	default:
		DKERROR_THROW_DEBUG("DKRenderState::CullFace unknown value");
	}	
}

DKRenderState::GLCullFace DKRenderState::CullFace(void) const
{
	return cullFace;
}

void DKRenderState::FrontFace(GLFrontFace face)
{
	if (frontFace == face)
		return;

	switch (face)
	{
	case GLFrontFaceCCW:	glFrontFace(GL_CCW);	frontFace = face;	break;
	case GLFrontFaceCW:		glFrontFace(GL_CW);		frontFace = face;	break;
	default:
		DKERROR_THROW_DEBUG("DKRenderState::FrontFace unknown value");
	}
}

DKRenderState::GLFrontFace DKRenderState::FrontFace(void) const
{
	return frontFace;
}

void DKRenderState::Viewport(int x, int y, int w, int h)
{
	if (viewport[0] == x && viewport[1] == y && viewport[2] == w && viewport[3] == h)
		return;

	glViewport(x,y,w,h);
	viewport[0] = x;
	viewport[1] = y;
	viewport[2] = w;
	viewport[3] = h;
}

void DKRenderState::DepthRange(float n, float f)
{
	n = Clamp(n, 0.0, 1.0);
	f = Clamp(f, 0.0, 1.0);

	if (depthRangeNear == n && depthRangeFar == f)
		return;

#ifdef DKGL_OPENGL_ES
	glDepthRangef(n, f);
#else
	glDepthRange(n, f);
#endif

	depthRangeNear = n;
	depthRangeFar = f;
}

void DKRenderState::PolygonOffset(float factor, float units)
{
	if (polygonOffsetFactor == factor && polygonOffsetUnits == units)
		return;

	glPolygonOffset(factor, units);
	polygonOffsetFactor = factor;
	polygonOffsetUnits = units;
}

void DKRenderState::LineWidth(float w)
{
	if (lineWidth == w)
		return;

	glLineWidth(w);
	lineWidth = w;
}

void DKRenderState::Flush(void)
{
	glFlush();
}

void DKRenderState::Finish(void)
{
	glFinish();
}

unsigned int DKRenderState::GetError(void)
{
	return glGetError();
}

const char* DKRenderState::GetErrorString(unsigned int err)
{
	switch (err)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
		break;
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
		break;
#ifdef GL_STACK_OVERFLOW
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
		break;
#endif
#ifdef GL_STACK_UNDERFLOW
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
		break;
#endif
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
		break;
	}
	return "Unknown";
}
