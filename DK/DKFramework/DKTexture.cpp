//
//  File: DKTexture.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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
				case DKTexture::Target3D:			return GL_TEXTURE_3D;
				case DKTexture::TargetCube:			return GL_TEXTURE_CUBE_MAP;
			}
			DKFoundation::DKLog("Warning: DKTexture target (%x) invalid or not supported.\n", t);
			return 0;
		}
		int GetTextureFormatComponents(DKTexture::Format f)
		{
			switch (f)
			{
				case DKTexture::FormatR8:
				case DKTexture::FormatR16F:
				case DKTexture::FormatR32F:
					return 1;
				case DKTexture::FormatRG8:
				case DKTexture::FormatRG16F:
				case DKTexture::FormatRG32F:
					return 2;
				case DKTexture::FormatRGB8:
				case DKTexture::FormatRGB16F:
				case DKTexture::FormatRGB32F:
					return 3;
				case DKTexture::FormatRGBA8:
				case DKTexture::FormatRGBA16F:
				case DKTexture::FormatRGBA32F:
					return 4;
				case DKTexture::FormatR8I:
				case DKTexture::FormatR8UI:
				case DKTexture::FormatR16I:
				case DKTexture::FormatR16UI:
				case DKTexture::FormatR32I:
				case DKTexture::FormatR32UI:
					return 1;
				case DKTexture::FormatRG8I:
				case DKTexture::FormatRG8UI:
				case DKTexture::FormatRG16I:
				case DKTexture::FormatRG16UI:
				case DKTexture::FormatRG32I:
				case DKTexture::FormatRG32UI:
					return 2;
				case DKTexture::FormatRGB8I:
				case DKTexture::FormatRGB8UI:
				case DKTexture::FormatRGB16I:
				case DKTexture::FormatRGB16UI:
				case DKTexture::FormatRGB32I:
				case DKTexture::FormatRGB32UI:
					return 3;
				case DKTexture::FormatRGBA8I:
				case DKTexture::FormatRGBA8UI:
				case DKTexture::FormatRGBA16I:
				case DKTexture::FormatRGBA16UI:
				case DKTexture::FormatRGBA32I:
				case DKTexture::FormatRGBA32UI:
					return 4;
				case DKTexture::FormatDepth16:
				case DKTexture::FormatDepth24:
				case DKTexture::FormatDepth32F:
					return 1;
			}
			DKFoundation::DKLog("Warning: DKTexture format (%x) invalid or not supported.\n", f);
			return 0;
		}
		GLenum GetTextureFormatGLValue(DKTexture::Format f)
		{
			switch (f)
			{
				case DKTexture::FormatR8:
				case DKTexture::FormatR16F:
				case DKTexture::FormatR32F:
					return GL_RED;
				case DKTexture::FormatRG8:
				case DKTexture::FormatRG16F:
				case DKTexture::FormatRG32F:
					return GL_RG;
				case DKTexture::FormatRGB8:
				case DKTexture::FormatRGB16F:
				case DKTexture::FormatRGB32F:
					return GL_RGB;
				case DKTexture::FormatRGBA8:
				case DKTexture::FormatRGBA16F:
				case DKTexture::FormatRGBA32F:
					return GL_RGBA;
				case DKTexture::FormatR8I:
				case DKTexture::FormatR8UI:
				case DKTexture::FormatR16I:
				case DKTexture::FormatR16UI:
				case DKTexture::FormatR32I:
				case DKTexture::FormatR32UI:
					return GL_RED_INTEGER;
				case DKTexture::FormatRG8I:
				case DKTexture::FormatRG8UI:
				case DKTexture::FormatRG16I:
				case DKTexture::FormatRG16UI:
				case DKTexture::FormatRG32I:
				case DKTexture::FormatRG32UI:
					return GL_RG_INTEGER;
				case DKTexture::FormatRGB8I:
				case DKTexture::FormatRGB8UI:
				case DKTexture::FormatRGB16I:
				case DKTexture::FormatRGB16UI:
				case DKTexture::FormatRGB32I:
				case DKTexture::FormatRGB32UI:
					return GL_RGB_INTEGER;
				case DKTexture::FormatRGBA8I:
				case DKTexture::FormatRGBA8UI:
				case DKTexture::FormatRGBA16I:
				case DKTexture::FormatRGBA16UI:
				case DKTexture::FormatRGBA32I:
				case DKTexture::FormatRGBA32UI:
					return GL_RGBA_INTEGER;
				case DKTexture::FormatDepth16:
				case DKTexture::FormatDepth24:
				case DKTexture::FormatDepth32F:
					return GL_DEPTH_COMPONENT;
			}
			DKFoundation::DKLog("Warning: DKTexture format (%x) invalid or not supported.\n", f);
			return 0;
		}
		GLenum GetTextureInternalFormatGLValue(DKTexture::Format f)
		{
			switch (f)
			{
				case DKTexture::FormatR8:		return GL_R8;
				case DKTexture::FormatR16F:		return GL_R16F;
				case DKTexture::FormatR32F:		return GL_R32F;
				case DKTexture::FormatRG8:		return GL_RG8;
				case DKTexture::FormatRG16F:	return GL_RG16F;
				case DKTexture::FormatRG32F:	return GL_RG32F;
				case DKTexture::FormatRGB8:		return GL_RGB8;
				case DKTexture::FormatRGB16F:	return GL_RGB16F;
				case DKTexture::FormatRGB32F:	return GL_RGB32F;
				case DKTexture::FormatRGBA8:	return GL_RGBA8;
				case DKTexture::FormatRGBA16F:	return GL_RGBA16F;
				case DKTexture::FormatRGBA32F:	return GL_RGBA32F;

				case DKTexture::FormatR8I:		return GL_R8I;
				case DKTexture::FormatR8UI:		return GL_R8UI;
				case DKTexture::FormatR16I:		return GL_R16I;
				case DKTexture::FormatR16UI:	return GL_R16UI;
				case DKTexture::FormatR32I:		return GL_R32I;
				case DKTexture::FormatR32UI:	return GL_R32UI;
				case DKTexture::FormatRG8I:		return GL_RG8I;
				case DKTexture::FormatRG8UI:	return GL_RG8I;
				case DKTexture::FormatRG16I:	return GL_RG16I;
				case DKTexture::FormatRG16UI:	return GL_RG16UI;
				case DKTexture::FormatRG32I:	return GL_RG32I;
				case DKTexture::FormatRG32UI:	return GL_RG32UI;
				case DKTexture::FormatRGB8I:	return GL_RGB8I;
				case DKTexture::FormatRGB8UI:	return GL_RGB8UI;
				case DKTexture::FormatRGB16I:	return GL_RGB16I;
				case DKTexture::FormatRGB16UI:	return GL_RGB16UI;
				case DKTexture::FormatRGB32I:	return GL_RGB32I;
				case DKTexture::FormatRGB32UI:	return GL_RGB32UI;
				case DKTexture::FormatRGBA8I:	return GL_RGBA8I;
				case DKTexture::FormatRGBA8UI:	return GL_RGBA8UI;
				case DKTexture::FormatRGBA16I:	return GL_RGBA16I;
				case DKTexture::FormatRGBA16UI:	return GL_RGBA16UI;
				case DKTexture::FormatRGBA32I:	return GL_RGBA32I;
				case DKTexture::FormatRGBA32UI:	return GL_RGBA32UI;

				case DKTexture::FormatDepth16:	return GL_DEPTH_COMPONENT16;
				case DKTexture::FormatDepth24:	return GL_DEPTH_COMPONENT24;
				case DKTexture::FormatDepth32F:	return GL_DEPTH_COMPONENT32F;
			}
			DKFoundation::DKLog("Warning: DKTexture format (%x) invalid or not supported.\n", f);
			return 0;
		}
		GLenum GetTextureTypeGLValue(DKTexture::Type t)
		{
			switch (t)
			{
				case DKTexture::TypeByte:			return GL_BYTE;
				case DKTexture::TypeUnsignedByte:	return GL_UNSIGNED_BYTE;
				case DKTexture::TypeShort:			return GL_SHORT;
				case DKTexture::TypeUnsignedShort:	return GL_UNSIGNED_SHORT;
				case DKTexture::TypeInt:			return GL_INT;
				case DKTexture::TypeUnsignedInt:	return GL_UNSIGNED_INT;
				case DKTexture::TypeFloat16:		return GL_HALF_FLOAT;
				case DKTexture::TypeFloat32:		return GL_FLOAT;
#ifdef GL_DOUBLE
				case DKTexture::TypeDouble:			return GL_DOUBLE;
#endif
			}
			DKFoundation::DKLog("Warning: DKTexture type (%x) invalid or not supported.\n", t);
			return 0;
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
	{
		DKOpenGLContext::RenderState().BindTexture(Private::GetTextureTargetGLValue(target), 0);
		glDeleteTextures(1, &resourceId);
	}
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
	case TypeByte:
		return sizeof(signed char) * components;
		break;
	case TypeUnsignedByte:
		return sizeof(unsigned char) * components;
		break;
	case TypeShort:
		return sizeof(signed short) * components;
		break;
	case TypeUnsignedShort:
		return sizeof(unsigned short) * components;
		break;
	case TypeInt:
		return sizeof(signed int) * components;
		break;
	case TypeUnsignedInt:
		return sizeof(unsigned int) * components;
		break;
	case TypeFloat16:
		return sizeof(GLhalf) * components;
		break;
	case TypeFloat32:
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
	switch (format)
	{
		// R
		case FormatR8:
		case FormatR16F:
		case FormatR32F:
		// RG
		case FormatRG8:
		case FormatRG16F:
		case FormatRG32F:
		// RGB
		case FormatRGB8:
		case FormatRGB16F:
		case FormatRGB32F:
		// RGBA
		case FormatRGBA8:
		case FormatRGBA16F:
		case FormatRGBA32F:
		// R (not normalized)
		case FormatR8I:
		case FormatR8UI:
		case FormatR16I:
		case FormatR16UI:
		case FormatR32I:
		case FormatR32UI:
		// RG (not normalized)
		case FormatRG8I:
		case FormatRG8UI:
		case FormatRG16I:
		case FormatRG16UI:
		case FormatRG32I:
		case FormatRG32UI:
		// RGB (not normalized)
		case FormatRGB8I:
		case FormatRGB8UI:
		case FormatRGB16I:
		case FormatRGB16UI:
		case FormatRGB32I:
		case FormatRGB32UI:
		// RGBA (not normalized)
		case FormatRGBA8I:
		case FormatRGBA8UI:
		case FormatRGBA16I:
		case FormatRGBA16UI:
		case FormatRGBA32I:
		case FormatRGBA32UI:
			return true;
	}
	return false;
}

bool DKTexture::IsDepthTexture(void) const
{
	switch (format)
	{
		case FormatDepth16:
		case FormatDepth24:
		case FormatDepth32F:
			return true;
	}
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
