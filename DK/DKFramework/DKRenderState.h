//
//  File: DKRenderState.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

////////////////////////////////////////////////////////////////////////////////
// DKRenderState
// Render state control, only for current bound OpenGL thread.
// Render state not shared by threads. only for current thread.
// Every OpenGL bound thread has one DKRenderState object at least.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKRenderState
	{
	public:
		enum GLState
		{
			GLStateUnused = 0,
			GLStateBlend,
			GLStateDepthTest,
			GLStateCullFace,
			GLStatePolygonOffsetFill,
			GLStateMaxValue,
		};
		enum GLFrontFace
		{
			GLFrontFaceCCW = 0,
			GLFrontFaceCW,
		};
		enum GLCullFace
		{
			GLCullFaceFront = 0,
			GLCullFaceBack,
			GLCullFaceBoth,
		};

		~DKRenderState(void);

		void Reset(void);
		void Enable(GLState mode);
		void Disable(GLState mode);
		bool IsEnabled(GLState mode);
		void UseProgram(unsigned int id);
		void BindVertexArray(unsigned int id);
		void ClearVertexArray(unsigned int id);
		void BindFrameBuffer(unsigned int id);
		void BindVertexBuffer(unsigned int id);
		void BindIndexBuffer(unsigned int id);
		void ActiveTexture(unsigned int index);
		void BindTexture(unsigned int target, unsigned int id);
		void EnableVertexAttribArray(unsigned int index);
		void DisableVertexAttribArray(unsigned int index);
		void BlendEquation(unsigned int funcRGB, unsigned int funcAlpha);
		void BlendFunc(unsigned int srcRGB, unsigned int srcAlpha, unsigned int dstRGB, unsigned int dstAlpha);
		void BlendColor(float r, float g, float b, float a);
		void DepthFunc(unsigned int func);
		void DepthMask(bool mask);
		void ColorMask(bool r, bool g, bool b, bool a);
		void CullFace(GLCullFace face);
		void FrontFace(GLFrontFace face);
		void Viewport(int x, int y, int w, int h);
		void DepthRange(float n, float f);
		void PolygonOffset(float factor, float units);
		void ClearColor(float r, float g, float b, float a);
		void ClearDepth(float d);
		void LineWidth(float w);		// w <= 1.0

		GLCullFace CullFace(void) const;
		GLFrontFace FrontFace(void) const;

		void Flush(void);
		void Finish(void);

		unsigned int GetError(void);
		static const char* GetErrorString(unsigned int err);

		int maxCombinedTextureImageUnits;		// GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
		int maxVertexAttribs;					// GL_MAX_VERTEX_ATTRIBS
		int maxTextureSize;						// GL_MAX_TEXTURE_SIZE
		int maxDrawBuffers;						// GL_MAX_DRAW_BUFFERS
		int maxColorAttachments;				// GL_MAX_COLOR_ATTACHMENTS

	private:
		DKRenderState(void);
		DKRenderState(const DKRenderState&);
		DKRenderState& operator = (const DKRenderState&);

		struct VertexArray
		{
			unsigned int	vertexBuffer;
			unsigned int	indexBuffer;
			DKFoundation::DKArray<bool>	attribArrayEnabled;
		};
		struct TextureUnit
		{
			unsigned int	target;
			unsigned int	id;
		};
		unsigned int	frameBuffer;
		unsigned int	vertexArray;
		unsigned int	program;
		bool			colorMask[4];
		float			clearColor[4];
		float			clearDepth;
		float			blendColor[4];
		unsigned int	blendFuncRGB;
		unsigned int	blendFuncAlpha;
		unsigned int	blendSrcRGB;
		unsigned int	blendSrcAlpha;
		unsigned int	blendDstRGB;
		unsigned int	blendDstAlpha;
		bool			depthMask;
		unsigned int	depthFunc;
		unsigned int	activeTexture;
		GLFrontFace		frontFace;
		GLCullFace		cullFace;
		int				viewport[4];
		float			depthRangeNear;
		float			depthRangeFar;
		float			polygonOffsetFactor;
		float			polygonOffsetUnits;
		float			lineWidth;

		DKFoundation::DKMap<unsigned int, VertexArray>	vertexArrays;
		DKFoundation::DKArray<TextureUnit>				activeTextures;

		enum {GLStateLength = (GLStateMaxValue / 8) + (GLStateMaxValue % 8 ? 1 : 0)};
		unsigned char glStates[GLStateLength];

		friend class DKOpenGLContext;
	};
}
