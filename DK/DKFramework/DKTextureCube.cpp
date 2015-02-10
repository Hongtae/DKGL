//
//  File: DKTextureCube.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKTextureCube.h"
#include "DKRenderTarget.h"
#include "DKOpenGLContext.h"

namespace DKFramework
{
	namespace Private
	{
		GLenum GetTextureTargetGLValue(DKTexture::Target t);
		GLenum GetTextureFormatGLValue(DKTexture::Format f);
		GLenum GetTextureInternalFormatGLValue(DKTexture::Format f);
		GLenum GetTextureTypeGLValue(DKTexture::Type t);
		DKTexture::Format GetTextureFormat(GLenum f);
	}
}

using namespace DKFoundation;
using namespace DKFramework;


DKTextureCube::DKTextureCube(void)
	: DKTexture(TargetCube)
{
}

DKTextureCube::~DKTextureCube(void)
{
}

DKObject<DKTextureCube> DKTextureCube::Create(int width, int height, Format imageFormat, Type dataType)
{
	if (width <= 0 || height <= 0)
		return NULL;
	if (Private::GetTextureFormatGLValue(imageFormat) == 0 ||
		Private::GetTextureInternalFormatGLValue(imageFormat) == 0 ||
		Private::GetTextureTypeGLValue(dataType) == 0)
		return NULL;
	
	unsigned int texId = 0;
	glGenTextures(1, &texId); 
	glBindTexture(GL_TEXTURE_CUBE_MAP, texId); 
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
//	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
//	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
//	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int face = 0; face < 6; face++)
	{ 
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
					 Private::GetTextureInternalFormatGLValue(imageFormat),
					 width, height, 0,
					 Private::GetTextureFormatGLValue(imageFormat),
					 Private::GetTextureTypeGLValue(dataType), NULL);
	} 
	// get info.
	GLint textureWidth = width;
	GLint textureHeight = height;
	GLint textureComponents = 0;
//	glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &textureWidth);
//	glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_HEIGHT, &textureHeight);
//	glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_COMPONENTS, &textureComponents);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	DKObject<DKTextureCube> ret = DKObject<DKTextureCube>::New();
	ret->resourceId = texId;
	ret->format = imageFormat;
	ret->type = dataType;
	ret->width = textureWidth;
	ret->height = textureHeight;
	ret->depth = 1;
	ret->components = textureComponents;

	return ret;
}

DKRenderTarget* DKTextureCube::GetRenderTarget(void)
{
	if (IsValid() && renderTarget == NULL)
	{
		renderTarget = DKObject<DKRenderTarget>::New();
		renderTarget->SetDepthBuffer(this->width, this->height, DKRenderTarget::DepthFormat32);
		renderTarget->Bind();

		// bind 6 cube textures into FBO.
		for (int face = 0; face < 6; face++)
		{ 
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + face,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, resourceId, 0);
		}
		DKOpenGLContext::RenderState().BindFrameBuffer(0);
	}
	return renderTarget;
}

DKObject<DKSerializer> DKTextureCube::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKTextureCube* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKTextureCube");
			this->Bind(L"super", target->DKTexture::Serializer(), NULL);

			return this;
		}
	private:
		DKObject<DKTextureCube> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
