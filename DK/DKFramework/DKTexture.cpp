//
//  File: DKTexture.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"

#include "DKTexture.h"
#include "DKOpenGLContext.h"

namespace DKFramework
{
	namespace Private
	{
		GLint GetMaxTextureSize(void)
		{
			static GLint maxTextureSize = 0;
			if (maxTextureSize == 0)
				glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

			DKASSERT_DEBUG(maxTextureSize > 0);
			return maxTextureSize;
		}

		GLenum GetTextureTargetGLValue(DKTexture::Target t)
		{
			switch (t)
			{
				case DKTexture::Target2D:			return GL_TEXTURE_2D;
#ifdef GL_TEXTURE_3D
				case DKTexture::Target3D:			return GL_TEXTURE_3D;
#endif
				case DKTexture::TargetCube:			return GL_TEXTURE_CUBE_MAP;
			}
			DKFoundation::DKLog("Warning: DKTexture target (%x) invalid or not supported.\n", t);
			return 0;
		}
		GLenum GetTextureFormatGLValue(DKTexture::Format f)
		{
			switch (f)
			{
				case DKTexture::FormatAlpha:		return GL_ALPHA;
				case DKTexture::FormatRGB:			return GL_RGB;
				case DKTexture::FormatRGBA:			return GL_RGBA;
				case DKTexture::FormatDepth16:
#ifdef	GL_DEPTH_COMPONENT24
				case DKTexture::FormatDepth24:
#endif
#ifdef	GL_DEPTH_COMPONENT32
				case DKTexture::FormatDepth32:
#endif
					return GL_DEPTH_COMPONENT;
			}
			DKFoundation::DKLog("Warning: DKTexture format (%x) invalid or not supported.\n", f);
			return 0;
		}
		GLenum GetTextureInternalFormatGLValue(DKTexture::Format f)
		{
			switch (f)
			{
				case DKTexture::FormatAlpha:		return GL_ALPHA;
				case DKTexture::FormatRGB:			return GL_RGB;
				case DKTexture::FormatRGBA:			return GL_RGBA;
				case DKTexture::FormatDepth16:		return GL_DEPTH_COMPONENT16;
#ifdef	GL_DEPTH_COMPONENT24
				case DKTexture::FormatDepth24:		return GL_DEPTH_COMPONENT24;
#endif
#ifdef	GL_DEPTH_COMPONENT32
				case DKTexture::FormatDepth32:		return GL_DEPTH_COMPONENT32;
#endif
			}
			DKFoundation::DKLog("Warning: DKTexture format (%x) invalid or not supported.\n", f);
			return 0;
		}
		GLenum GetTextureTypeGLValue(DKTexture::Type t)
		{
			switch (t)
			{
				case DKTexture::TypeSignedByte:		return GL_BYTE;
				case DKTexture::TypeUnsignedByte:	return GL_UNSIGNED_BYTE;
				case DKTexture::TypeSignedShort:	return GL_SHORT;
				case DKTexture::TypeUnsignedShort:	return GL_UNSIGNED_SHORT;
				case DKTexture::TypeSignedInt:		return GL_INT;
				case DKTexture::TypeUnsignedInt:	return GL_UNSIGNED_INT;
				case DKTexture::TypeFloat:			return GL_FLOAT;
#ifdef GL_DOUBLE
				case DKTexture::TypeDouble:			return GL_DOUBLE;
#endif
			}
			DKFoundation::DKLog("Warning: DKTexture type (%x) invalid or not supported.\n", t);
			return 0;
		}
		DKTexture::Format GetTextureFormat(GLenum f)
		{
			switch (f)
			{
				case GL_ALPHA:						return DKTexture::FormatAlpha;
				case GL_RGB:						return DKTexture::FormatRGB;
				case GL_RGBA:						return DKTexture::FormatRGBA;
				case GL_DEPTH_COMPONENT16:			return DKTexture::FormatDepth16;
#ifdef GL_DEPTH_COMPONENT24
				case GL_DEPTH_COMPONENT24:			return DKTexture::FormatDepth24;
#endif
#ifdef GL_DEPTH_COMPONENT32
				case GL_DEPTH_COMPONENT32:			return DKTexture::FormatDepth32;
#endif
			}
			DKFoundation::DKLog("Warning: Cannot convert texture format (%x) invalid or not supported.\n", f);
			return DKTexture::FormatUnknown;
		}
	}
}


using namespace DKFoundation;
using namespace DKFramework;


DKTexture::DKTexture(void)
	: resourceId(0)
	, target(TargetUnknown)
	, width(0), height(0), depth(0)
	, components(0)
{
}

DKTexture::DKTexture(Target t)
	: resourceId(0)
	, target(t)
	, width(0), height(0), depth(0)
	, components(0)
{
}

DKTexture::~DKTexture(void)
{
	if (resourceId)
		glDeleteTextures(1, &resourceId);
}

void DKTexture::Bind(void) const
{
	if (target != TargetUnknown)
		DKOpenGLContext::RenderState().BindTexture(Private::GetTextureTargetGLValue(target), resourceId);
}

bool DKTexture::IsValid(void) const
{
	if (target != TargetUnknown &&
		format != FormatUnknown &&
		width > 0 && height > 0 && depth > 0 &&
		resourceId != 0)
	{
		return true;
	}
	return false;
}

DKSize DKTexture::Resolution(void) const
{
	return DKSize(width, height);
}

DKVector3 DKTexture::Dimensions(void) const
{
	return DKVector3(width, height, depth);
}

size_t DKTexture::BytesPerPixel(void) const
{
	switch (type)
	{
	case TypeSignedByte:
		return sizeof(signed char) * components;
		break;
	case TypeUnsignedByte:
		return sizeof(unsigned char) * components;
		break;
	case TypeSignedShort:
		return sizeof(signed short) * components;
		break;
	case TypeUnsignedShort:
		return sizeof(unsigned short) * components;
		break;
	case TypeSignedInt:
		return sizeof(signed int) * components;
		break;
	case TypeUnsignedInt:
		return sizeof(unsigned int) * components;
		break;
	case TypeFloat:
		return sizeof(float) * components;
		break;
	case TypeDouble:
		return sizeof(double) * components;
		break;
	}
	return 0;
}

bool DKTexture::IsColorTexture(void) const
{
	if (format == FormatRGB ||
		format == FormatRGBA)
		return true;
	return false;
}

bool DKTexture::IsDepthTexture(void) const
{
	if (format == FormatDepth16 ||
		format == FormatDepth24 ||
		format == FormatDepth32)
		return true;
	return false;
}

DKObject<DKSerializer> DKTexture::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKTexture* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKTexture");
			this->Bind(L"super", target->DKResource::Serializer(), NULL);

			return this;
		}
	private:
		DKObject<DKTexture> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}

int DKTexture::MaxTextureSize(void)
{
	return Private::GetMaxTextureSize();
}
