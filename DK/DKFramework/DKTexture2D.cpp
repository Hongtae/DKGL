//
//  File: DKTexture2D.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#define DKLIB_EXTDEPS_CXIMAGE
#include "../lib/ExtDeps.h"
#include "../lib/OpenGL.h"
#include "DKTexture2D.h"
#include "DKOpenGLContext.h"
#include "DKResourceLoader.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		GLenum GetTextureTargetGLValue(DKTexture::Target t);
		GLenum GetTextureFormatGLValue(DKTexture::Format f);
		GLenum GetTextureInternalFormatGLValue(DKTexture::Format f);
		GLenum GetTextureTypeGLValue(DKTexture::Type t);
		DKTexture::Format GetTextureFormat(GLenum f);

		GLint GetMaxTextureSize(void);		// DKTexture.cpp

		namespace
		{
			struct TextureInfo
			{
				GLuint				resourceId;
				int					width;
				int					height;
				int					imageWidth;
				int					imageHeight;
				size_t				components;
				DKTexture::Format	format;
				DKTexture::Type		type;			
			};

			bool CreateTexture(int width, int height, DKTexture::Format imageFormat, DKTexture::Type dataType, const void* data, TextureInfo& ti)
			{
				if (width <= 0 || height <= 0)
					return false;
				if (Private::GetTextureFormatGLValue(imageFormat) == 0 ||
					Private::GetTextureInternalFormatGLValue(imageFormat) == 0 ||
					Private::GetTextureTypeGLValue(dataType) == 0)
					return false;

				if (width > GetMaxTextureSize() || height > GetMaxTextureSize())
					return NULL;

				GLuint texId = 0;
				glGenTextures(1, &texId);
				glBindTexture(GL_TEXTURE_2D, texId);

				GLenum wrapMode = GL_CLAMP_TO_EDGE;
				switch (imageFormat)
				{
				case DKTexture::FormatRGB:
				case DKTexture::FormatRGBA:
					wrapMode = (IsPowerOfTwo(width) && IsPowerOfTwo(height)) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
					break;
				}
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);				
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				//glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
				//glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
				//glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				//glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);

#ifdef DKLIB_OPENGL_ES_2
				// Note:
				//   On OpenGL ES 2,
				//   GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24 cannot be used
				//   for internal format. Internal format should be
				//   GL_DEPTH_COMPONENT, and we can specify type with
				//   GL_UNSIGNED_SHORT for 16bit depth,
				//   GL_UNSIGNED_INT for 32bit depth. (24bit on some platform)
				GLenum textureInternalFormat = Private::GetTextureFormatGLValue(imageFormat);
#else
				GLenum textureInternalFormat = Private::GetTextureInternalFormatGLValue(imageFormat);
#endif
				GLenum textureFormat = Private::GetTextureFormatGLValue(imageFormat);
				GLenum textureType = Private::GetTextureTypeGLValue(dataType);

				// copy data
				glTexImage2D(GL_TEXTURE_2D, 0,
							 textureInternalFormat,
							 width, height, 0,
							 textureFormat,
							 textureType,
							 data);

				GLenum err = glGetError();
				if (err != GL_NO_ERROR)
				{
					DKRenderState& rs = DKOpenGLContext::RenderState();
					DKLog("CreateTexture Failed: %s\n", rs.GetErrorString(err));

					glBindTexture(GL_TEXTURE_2D, 0);
					rs.BindTexture(GL_TEXTURE_2D, 0);
					glDeleteTextures(1, &texId);
					return false;
				}

				// get properties
				GLint textureWidth = width;
				GLint textureHeight = height;
				//GLint textureFormatInternal = 0;
				GLint textureComponents = 0;
				switch (imageFormat)
				{
				case DKTexture::FormatAlpha:
					textureComponents = 1;	break;
				case DKTexture::FormatRGB:
					textureComponents = 3;	break;
				case DKTexture::FormatRGBA:
					textureComponents = 4;	break;
				default:
					textureComponents = 1;	break;
				}
				//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
				//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
				// internal-format may differ by platform.
				//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &textureFormatInternal);
				//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &textureComponents);
				glBindTexture(GL_TEXTURE_2D, 0);
				DKOpenGLContext::RenderState().BindTexture(GL_TEXTURE_2D, 0);

				ti.resourceId = texId;
				ti.format = imageFormat;
				ti.type = dataType;
				ti.width = textureWidth;
				ti.height = textureHeight;
				ti.components = textureComponents;
				return true;
			}

			bool CreateTexture(CxImage* image, int width, int height, TextureInfo& ti)
			{
				if (width < 1 || height < 1)
					return false;
				if (width > GetMaxTextureSize() || height > GetMaxTextureSize())
					return false;

				if (image->Resample2(width, height, CxImage::IM_BICUBIC2, CxImage::OM_REPEAT))
				{
					int w = image->GetWidth();
					int h = image->GetHeight();

					void* pixelData = NULL;

					DKTexture::Format format = DKTexture::FormatUnknown;
					if (image->AlphaIsValid())
					{
						size_t bytes = w * h * 4;
						pixelData = malloc( bytes );
						CxMemFile cf((uint8_t*)pixelData, bytes);
						if (image->Encode2RGBA(&cf, false))
						{
							DKASSERT_DEBUG(cf.GetBuffer() == pixelData);
							format = DKTexture::FormatRGBA;
						}
					}
					else
					{
						size_t bytes = w * h * 3;
						pixelData = malloc( bytes );
						CxMemFile cf((uint8_t*)pixelData, bytes);
						if (image->Encode2RGB(&cf, false))
						{
							DKASSERT_DEBUG(cf.GetBuffer() == pixelData);
							format = DKTexture::FormatRGB;
						}
					}

					bool result = false;
					if (format != DKTexture::FormatUnknown)
						result = CreateTexture(w, h, format, DKTexture::TypeUnsignedByte, pixelData, ti);
					DKASSERT_DEBUG(pixelData != NULL);
					free(pixelData);
					return result;
				}
				return false;
			}

			bool CreateTexture(const DKString& file, TextureInfo& ti)
			{
				if (file.Length() == 0)
					return false;
				
				CxImage image;
#ifdef _WIN32
				bool loaded = image.Load((const wchar_t*)file, CXIMAGE_FORMAT_UNKNOWN);
#else
				bool loaded = image.Load((const char*)DKStringU8(file), CXIMAGE_FORMAT_UNKNOWN);
#endif
				if (loaded)
				{
					ti.imageWidth = image.GetWidth();
					ti.imageHeight = image.GetHeight();

					int width = Min(ti.imageWidth, GetMaxTextureSize());
					int height = Min(ti.imageHeight, GetMaxTextureSize());
					return CreateTexture(&image, width, height, ti);
				}
				return false;
			}
			
			bool CreateTexture(const void* data, size_t size, TextureInfo& ti)
			{
				if (data == NULL || size == 0)
					return false;

				CxImage image;
				if (image.Decode((uint8_t*)data, size, CXIMAGE_FORMAT_UNKNOWN))
				{
					ti.imageWidth = image.GetWidth();
					ti.imageHeight = image.GetHeight();

					int width = Min(ti.imageWidth, GetMaxTextureSize());
					int height = Min(ti.imageHeight, GetMaxTextureSize());
					return CreateTexture(&image, width, height, ti);
				}
				return false;
			}

			bool CreateResampledTexture(const DKString& file, int width, int height, TextureInfo& ti)
			{
				if (file.Length() == 0 || width < 1 || height < 1)
					return false;
				if (width > GetMaxTextureSize() || height > GetMaxTextureSize())
					return false;

				CxImage image;
#ifdef _WIN32
				bool loaded = image.Load((const wchar_t*)file, CXIMAGE_FORMAT_UNKNOWN);
#else
				bool loaded = image.Load((const char*)DKStringU8(file), CXIMAGE_FORMAT_UNKNOWN);
#endif
				if (loaded)
				{
					ti.imageWidth = image.GetWidth();
					ti.imageHeight = image.GetHeight();
					return CreateTexture(&image, width, height, ti);
				}
				return false;
			}

			bool CreateResampledTexture(const void* data, size_t size, int width, int height, TextureInfo& ti)
			{
				if (data == NULL || size == 0)
					return false;

				if (width > GetMaxTextureSize() || height > GetMaxTextureSize())
					return false;

				CxImage image;
				if (image.Decode((uint8_t*)data, size, CXIMAGE_FORMAT_UNKNOWN))
				{
					ti.imageWidth = image.GetWidth();
					ti.imageHeight = image.GetHeight();
					return CreateTexture(&image, width, height, ti);
				}
				return false;
			}

			// register image extensions to resource-loader.
			int RegisterImageFileExts(void)
			{
				const wchar_t* exts[] = {
					L"bmp",
					L"jpg", L"jpeg", L"jpe",
					L"gif",
					L"png",
					L"ico", L"cur",
					L"tif", L"tiff",
					L"tga",
					L"pcx",
					L"wbm",
					L"wmf", L"emf",
					NULL };

				auto loaderProc = [](DKStream* stream, DKAllocator&)->DKObject<DKResource>
				{
					return DKTexture2D::Create(stream).SafeCast<DKResource>();
				};
				DKObject<DKResourceLoader::ResourceLoader> loader = DKFunction(loaderProc);
				
				int num = 0;
				while (exts[num])
				{
					DKResourceLoader::SetResourceFileExtension(exts[num], loader);
					num++;
				}
				return num;
			}
			// make sure to allocator exists before register ext-types.
			static DKAllocator::StaticInitializer init;
			int numRegisteredImageExts = RegisterImageFileExts();
		}
	}
}

using namespace DKFramework;


DKTexture2D::DKTexture2D(void)
	: DKTexture(Target2D)
{
}

DKTexture2D::~DKTexture2D(void)
{
}

DKObject<DKTexture2D> DKTexture2D::Create(int width, int height, Format imageFormat, Type dataType, const void* data)
{
	Private::TextureInfo ti;
	if (Private::CreateTexture(width, height, imageFormat, dataType, data, ti))
	{
		DKObject<DKTexture2D> ret = DKObject<DKTexture2D>::New();
		ret->resourceId = ti.resourceId;
		ret->format = ti.format;
		ret->type = ti.type;
		ret->width = ti.width;
		ret->height = ti.height;
		ret->depth = 1;
		ret->components = ti.components;
		return ret;
	}
	return NULL;
}

DKObject<DKTexture2D> DKTexture2D::Create(const DKString& file, int width, int height)
{
	if (file.Left(7).CompareNoCase(L"http://") == 0 || file.Left(6).CompareNoCase(L"ftp://") == 0 || file.Left(7).CompareNoCase(L"file://") == 0)
	{
		return Create(DKBuffer::Create(file), width, height);
	}

	Private::TextureInfo ti;
	if (Private::CreateResampledTexture(file, width, height, ti))
	{
		DKObject<DKTexture2D> ret = DKObject<DKTexture2D>::New();
		ret->resourceId = ti.resourceId;
		ret->format = ti.format;
		ret->type = ti.type;
		ret->width = ti.width;
		ret->height = ti.height;
		ret->depth = 1;
		ret->components = ti.components;
		return ret;
	}
	return NULL;
}

DKObject<DKTexture2D> DKTexture2D::Create(const DKString& file)
{
	if (file.Left(7).CompareNoCase(L"http://") == 0 || file.Left(6).CompareNoCase(L"ftp://") == 0 || file.Left(7).CompareNoCase(L"file://") == 0)
	{
		return Create(DKBuffer::Create(file));
	}

	Private::TextureInfo ti;
	if (Private::CreateTexture(file, ti))
	{
		DKObject<DKTexture2D> ret = DKObject<DKTexture2D>::New();
		ret->resourceId = ti.resourceId;
		ret->format = ti.format;
		ret->type = ti.type;
		ret->width = ti.width;
		ret->height = ti.height;
		ret->depth = 1;
		ret->components = ti.components;
		return ret;
	}
	return NULL;
}

DKObject<DKTexture2D> DKTexture2D::Create(const DKData* data, int width, int height)
{
	Private::TextureInfo ti;

	bool ret = Private::CreateResampledTexture(data->LockShared(), data->Length(), width, height, ti);
	data->UnlockShared();

	if (ret)
	{
		DKObject<DKTexture2D> ret = DKObject<DKTexture2D>::New();
		ret->resourceId = ti.resourceId;
		ret->format = ti.format;
		ret->type = ti.type;
		ret->width = ti.width;
		ret->height = ti.height;
		ret->depth = 1;
		ret->components = ti.components;
		return ret;
	}
	return NULL;
}

DKObject<DKTexture2D> DKTexture2D::Create(const DKData* data)
{
	Private::TextureInfo ti;

	bool ret = Private::CreateTexture(data->LockShared(), data->Length(), ti);
	data->UnlockShared();

	if (ret)
	{
		DKObject<DKTexture2D> ret = DKObject<DKTexture2D>::New();
		ret->resourceId = ti.resourceId;
		ret->format = ti.format;
		ret->type = ti.type;
		ret->width = ti.width;
		ret->height = ti.height;
		ret->depth = 1;
		ret->components = ti.components;
		return ret;
	}
	return NULL;
}

DKObject<DKTexture2D> DKTexture2D::Create(DKStream* stream, int width, int height)
{
	DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
	if (ds)
		return Create(ds->DataSource(), width, height);

	return Create(DKBuffer::Create(stream), width, height);
}

DKObject<DKTexture2D> DKTexture2D::Create(DKStream* stream)
{
	DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
	if (ds)
		return Create(ds->DataSource());

	return Create(DKBuffer::Create(stream));
}

void DKTexture2D::SetPixelData(const DKRect& rc, const void* data)
{
	Bind();
	GLint x = floor(rc.origin.x + 0.5f);
	GLint y = floor(rc.origin.y + 0.5f);
	GLsizei width = floor(rc.size.width + 0.5f);
	GLsizei height = floor(rc.size.height + 0.5f);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height,
					Private::GetTextureFormatGLValue(format),
					Private::GetTextureTypeGLValue(type),
					data);
}

DKObject<DKData> DKTexture2D::CopyPixelData(const DKRect& rc, Format format, Type type) const
{
	if (this->IsValid() && this->ObjectTarget() == Target2D &&
		this->width > 0 && this->height > 0 && this->depth > 0)
	{
		if (format == FormatUnknown)
			format = this->format;

		size_t bpc = 0;		// bytes per component
		switch (type)
		{
		case TypeSignedByte:		bpc = sizeof(signed char);		break;
		case TypeUnsignedByte:		bpc = sizeof(unsigned char);	break;
		case TypeSignedShort:		bpc = sizeof(signed short);		break;
		case TypeUnsignedShort:		bpc = sizeof(unsigned short);	break;
		case TypeSignedInt:			bpc = sizeof(signed int);		break;
		case TypeUnsignedInt:		bpc = sizeof(unsigned int);		break;
		case TypeFloat:				bpc = sizeof(float);			break;
		case TypeDouble:			bpc = sizeof(double);			break;
		}
		size_t numComponents = 0;
		switch (format)
		{
		case FormatAlpha:	numComponents = 1;	break;
		case FormatRGB:		numComponents = 3;	break;
		case FormatRGBA:	numComponents = 4;	break;
		}

		GLint x = floor(rc.origin.x + 0.5f);
		GLint y = floor(rc.origin.y + 0.5f);
		GLsizei width = floor(rc.size.width + 0.5f);
		GLsizei height = floor(rc.size.height + 0.5f);

		size_t bpp = bpc * numComponents;		// bytes per pixel

		if (bpp == 0)
		{
			DKLog("Invalid format.\n");
			return NULL;
		}

		size_t bufferSize = width * height * bpp;
		if (bufferSize == 0)
		{
			DKLog("Invalid region.\n");
			return NULL;
		}

		// create dummy FBO, and copy.
		glGetError();

		glBindTexture(GL_TEXTURE_2D, 0);
		GLuint fbo = 0;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->resourceId, 0);

		GLenum fmt = Private::GetTextureFormatGLValue(format);
		GLenum t = Private::GetTextureTypeGLValue(type);

		DKObject<DKBuffer> data = DKBuffer::Create(NULL, bufferSize);
		void* ptr = data->LockExclusive();

		glReadPixels(x, y, width, height, fmt, t, ptr);

		data->UnlockExclusive();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);

		DKRenderState& state = DKOpenGLContext::RenderState();
		state.BindFrameBuffer(0);
		state.BindTexture(GL_TEXTURE_2D, 0);

		GLenum err = glGetError();

		if (err == GL_NO_ERROR)
			return data.SafeCast<DKData>();
		else
			DKLog("Error: %s\n", DKRenderState::GetErrorString(err));
	}
	return NULL;
}

DKObject<DKTexture2D> DKTexture2D::Duplicate(void) const
{
	if (this->IsValid() && this->ObjectTarget() == Target2D &&
		this->width > 0 && this->height > 0 && this->depth > 0)
	{
		glGetError();
		// create dummy FBO and copy. (use glCopyTexSubImage2D)
		DKRenderState& state = DKOpenGLContext::RenderState();
		state.BindFrameBuffer(0);

		DKObject<DKTexture2D> target = DKTexture2D::Create(this->width,
														   this->height,
														   this->format,
														   this->type,
														   NULL);
		target->Bind();

		GLuint fbo = 0;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->resourceId, 0);

		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, this->width, this->height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);

		state.BindTexture(GL_TEXTURE_2D, 0);

		GLenum err = glGetError();
		if (err == GL_NO_ERROR)
			return target;
		else
			DKLog("Error: %s\n", DKRenderState::GetErrorString(err));
	}
	return NULL;
}

DKObject<DKData> DKTexture2D::CreateImageFileData(const DKString& format) const
{
	if (this->IsValid() && this->ObjectTarget() == Target2D &&
		this->width > 0 && this->height > 0 && this->depth > 0 && this->BytesPerPixel() > 0)
	{
		switch (this->format)
		{
		case FormatAlpha:
		case FormatRGB:
		case FormatRGBA:
			break;
		default:
			DKLog("Invalid texture format!\n");
			return NULL;
		}

		struct ImageFormat
		{
			const wchar_t* name;
			ENUM_CXIMAGE_FORMATS value;
		};
		std::initializer_list<ImageFormat> formatList =
		{
			{ L"bmp", CXIMAGE_FORMAT_BMP },
			{ L"jpg", CXIMAGE_FORMAT_JPG },
			{ L"png", CXIMAGE_FORMAT_PNG },
			{ L"tif", CXIMAGE_FORMAT_TIF },
			{ L"tga", CXIMAGE_FORMAT_TGA }
		};

		ENUM_CXIMAGE_FORMATS imageFormat = CXIMAGE_FORMAT_UNKNOWN;
		for (auto& f : formatList)
		{
			if (format.CompareNoCase(f.name) == 0)
			{
				imageFormat = f.value;
				break;
			}
		}
		if (imageFormat == CXIMAGE_FORMAT_UNKNOWN)
		{
			DKLog("Unsupported file format!\n");
			return NULL;
		}

		// create dummy FBO, and copy pixels.
		glGetError();

		glBindTexture(GL_TEXTURE_2D, 0);
		GLuint fbo = 0;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->resourceId, 0);

		size_t bpp = this->BytesPerPixel();
		size_t bufferSize = this->width * this->height * bpp;
		void* data = malloc(bufferSize);
		GLenum fmt = Private::GetTextureFormatGLValue(this->format);
		GLenum t = Private::GetTextureTypeGLValue(this->type);

		glReadPixels(0, 0, this->width, this->height, fmt, t, data);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);

		GLenum err = glGetError();

		DKRenderState& state = DKOpenGLContext::RenderState();
		state.BindFrameBuffer(0);
		state.BindTexture(GL_TEXTURE_2D, 0);

		DKObject<DKData> result = NULL;

		if (err == GL_NO_ERROR)
		{
			CxImage image;
			if (image.CreateFromArray((uint8_t*)data, this->width, this->height, bpp * 8, this->width * bpp, false))
			{
				uint8_t* output = NULL;
				int32_t size = 0;

				image.SwapRGB2BGR();
				if (image.Encode(output, size, imageFormat))
				{
					result = DKBuffer::Create(output, size).SafeCast<DKData>();
					image.FreeMemory(output);
				}
			}
		}
		else
		{
			const char* errString = DKRenderState::GetErrorString(err);
			DKLog("Error: %s\n", errString);
		}
		free(data);
		return result;
	}
	return NULL;
}

bool DKTexture2D::Deserialize(const DKXMLElement* e, DKResourceLoader* loader)
{
	if (e == NULL || e->name.CompareNoCase(L"DKTexture2D"))
		return false;

	DKString objectName = L"";
	DKString src = L"";

	for (int i = 0; i < e->attributes.Count(); i++)
	{
		if (e->attributes.Value(i).name.CompareNoCase(L"name") == 0)
			objectName = e->attributes.Value(i).value;
		else if (e->attributes.Value(i).name.CompareNoCase(L"src") == 0)
			src = e->attributes.Value(i).value;
	}
	if (src.Length() > 0) // image is not XML, open stream instead of using loader.
	{
		if (loader)
		{
			DKObject<DKStream> stream = loader->OpenResourceStream(src);
			if (stream)
			{
				Private::TextureInfo ti;
				bool ret = false;

				DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
				if (ds)
				{
					const DKData* data = ds->DataSource();
					ret = Private::CreateTexture(data->LockShared(), data->Length(), ti);
					data->UnlockShared();
				}
				else
				{
					DKObject<DKBuffer> data = DKBuffer::Create(stream);
					if (data)
					{
						ret = Private::CreateTexture(data->LockShared(), data->Length(), ti);
						data->UnlockShared();
					}
				}
				if (ret)
				{
					// reset object
					if (this->resourceId)
						glDeleteTextures(1, &this->resourceId);

					this->SetName(objectName);
					this->resourceId = ti.resourceId;
					this->format = ti.format;
					this->type = ti.type;
					this->width = ti.width;
					this->height = ti.height;
					this->depth = 1;
					this->components = ti.components;

					loader->AddResource(objectName, this);	// add to pool.
					return true;
				}
			}
			else
			{
				DKLog("DKTexture2D::Deserialize error: Cannot open file:%ls\n", (const wchar_t*)objectName);
			}
		}
		DKLog("DKTexture2D::Deserialize error: Cannot load external file: %ls\n", (const wchar_t*)objectName);
		return NULL;
	}

	DKString content = L"";
	for (int i = 0; i < e->nodes.Count(); i++)
	{
		if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeCData)
		{
			content = e->nodes.Value(i).SafeCast<DKXMLCData>()->value;
			break;
		}
	}

	if (content.Length() > 0)
	{
		DKObject<DKBuffer> data = DKBuffer::DecodeDecompress(content);
		if (data)
		{
			DKObject<DKTexture2D> tex = NULL;
			Private::TextureInfo ti;
			bool ret = Private::CreateTexture(data->LockShared(), data->Length(), ti);
			data->UnlockShared();
			if (ret)
			{
				// reset object
				if (this->resourceId)
					glDeleteTextures(1, &this->resourceId);

				this->SetName(objectName);
				this->resourceId = ti.resourceId;
				this->format = ti.format;
				this->type = ti.type;
				this->width = ti.width;
				this->height = ti.height;
				this->depth = 1;
				this->components = ti.components;
				return true;
			}
			else
			{
				DKLog("DKTexture2D::Deserialize error: Resource:%ls loaded, but cannot create image.\n", (const wchar_t*)objectName);
			}
		}
		else
		{
			DKLog("DKTexture2D::Deserialize error: Resource:%ls Cannot open data.", (const wchar_t*)objectName);
		}
	}
	return false;
}

DKObject<DKXMLElement> DKTexture2D::SerializeXML(DKSerializer::SerializeForm sf) const
{
	bool externalContent = true;

	if (Name().Length() > 0)
	{
		DKObject<DKXMLElement> e = DKObject<DKXMLElement>::New();
		e->name = L"DKTexture2D";
		DKXMLAttribute name = {0, L"name", Name()};
		e->attributes.Add(name);

		if (externalContent)
		{
			DKXMLAttribute src = {0, L"src", Name()};
			e->attributes.Add(src);
		}
		else
		{
			if (this->width > 0 && this->height > 0 && this->components > 0)
			{
				this->Bind();

				DKBuffer data(NULL, this->width * this->height * this->components);
				glGetError();
				void* buffer = data.LockExclusive();

				GLenum fmt = Private::GetTextureFormatGLValue(this->format);
				GLenum t = Private::GetTextureTypeGLValue(this->type);

				glReadPixels(0, 0, width, height, fmt, t, buffer);
				data.UnlockExclusive();
				GLenum err = glGetError();
				if (err != GL_NO_ERROR)
				{
					DKObject<DKXMLCData> cdata = DKObject<DKXMLCData>::New();
					data.CompressEncode(cdata->value);
					e->nodes.Add(cdata.SafeCast<DKXMLNode>());
				}
				else
				{
					DKLog("Error: glReadPixels failed!\n");
				}
			}
		}
		return e;
	}
	return NULL;
}

DKObject<DKSerializer> DKTexture2D::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKTexture2D* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKTexture2D");
			this->Bind(L"super", target->DKTexture::Serializer(), NULL);

			return this;
		}
	private:
		DKObject<DKTexture2D> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
