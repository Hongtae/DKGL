//
//  File: DKImage.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "../Libs/Inc_CxImage.h"
#include "DKImage.h"

using namespace DKFramework;

DKImage::DKImage(void)
: data(NULL)
, width(0)
, height(0)
{
}

DKImage::~DKImage(void)
{
	if (data)
		DKFree(data);
}

bool DKImage::LoadFromPixelData(uint32_t width, uint32_t height, PixelFormat, const void* p)
{
	return false;
}

bool DKImage::LoadFromFile(const DKString &path)
{
	if (path.Length() == 0)
		return false;

	CxImage image;
#ifdef _WIN32
	bool loaded = image.Load((const wchar_t*)path, CXIMAGE_FORMAT_UNKNOWN);
#else
	bool loaded = image.Load((const char*)DKStringU8(path), CXIMAGE_FORMAT_UNKNOWN);
#endif
	if (loaded)
	{
		uint32_t w = image.GetWidth();
		uint32_t h = image.GetHeight();
		void* pixels = NULL;
		if (image.AlphaIsValid())
		{
			uint32_t length = w * h * 4;
			pixels = DKMalloc(length);
			CxMemFile cf((uint8_t*)pixels, length);

			if (image.Encode2RGBA(&cf, false))
			{
				this->width = w;
				this->height = h;
				this->format = RGBA;
				if (this->data)
					DKFree(this->data);
				this->data = pixels;
				return true;
			}
			DKFree(pixels);
		}
		else
		{
			uint32_t length = w * h * 3;
			pixels = DKMalloc(length);
			CxMemFile cf((uint8_t*)pixels, length);

			if (image.Encode2RGB(&cf, false))
			{
				this->width = w;
				this->height = h;
				this->format = RGB;
				if (this->data)
					DKFree(this->data);
				this->data = pixels;
				return true;
			}
		}
		if (pixels)
			DKFree(pixels);
	}
	return false;
}

bool DKImage::LoadFromData(const void *data, size_t length)
{
	if (data == NULL || length == 0)
		return false;

	CxImage image;
	if (image.Decode((uint8_t*)data, length, CXIMAGE_FORMAT_UNKNOWN))
	{
		uint32_t w = image.GetWidth();
		uint32_t h = image.GetHeight();
		void* pixels = NULL;
		if (image.AlphaIsValid())
		{
			uint32_t length = w * h * 4;
			pixels = DKMalloc(length);
			CxMemFile cf((uint8_t*)pixels, length);

			if (image.Encode2RGBA(&cf, false))
			{
				this->width = w;
				this->height = h;
				this->format = RGBA;
				if (this->data)
					DKFree(this->data);
				this->data = pixels;
				return true;
			}
			DKFree(pixels);
		}
		else
		{
			uint32_t length = w * h * 3;
			pixels = DKMalloc(length);
			CxMemFile cf((uint8_t*)pixels, length);

			if (image.Encode2RGB(&cf, false))
			{
				this->width = w;
				this->height = h;
				this->format = RGB;
				if (this->data)
					DKFree(this->data);
				this->data = pixels;
				return true;
			}
		}
		if (pixels)
			DKFree(pixels);
	}
	return false;
}

bool DKImage::LoadFromStream(DKStream* stream)
{
	DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
	if (ds)
		return LoadFromData(ds->Data());
	return LoadFromData(DKBuffer::Create(stream));
}

bool DKImage::LoadFromData(DKData* data)
{
	DKObject<DKData> d2 = data;
	DKDataReader reader(data);
	return LoadFromData(reader.Bytes(), reader.Length());
}

bool DKImage::Resample(uint32_t w, uint32_t h, Interpolation intp)
{
	if (IsValid() && w > 0 && h > 0)
	{
		if (w != width || h != height)
		{
			size_t bpp = this->BytesPerPixel() * 8;
			CxImage image;
			if (image.CreateFromArray((uint8_t*)data, this->width, this->height, bpp, this->width * bpp, false))
			{
				CxImage::InterpolationMethod im = CxImage::IM_BICUBIC2;
				switch (intp)
				{
					case Nearest: im = CxImage::IM_NEAREST_NEIGHBOUR; break;
					case Bilinear: im = CxImage::IM_BILINEAR; break;
					case Bicubic: im = CxImage::IM_BICUBIC2; break;
					case Spline: im = CxImage::IM_BSPLINE; break;
					case Lanczos: im = CxImage::IM_LANCZOS; break;
					case Gaussian: im = CxImage::IM_GAUSSIAN; break;
					case Quadratic: im = CxImage::IM_QUADRATIC; break;
				}

				if (image.Resample2(w, h, im, CxImage::OM_REPEAT))
				{
					w = image.GetWidth();
					h = image.GetHeight();
					void* pixels = NULL;
					if (image.AlphaIsValid())
					{
						uint32_t length = w * h * 4;
						pixels = DKMalloc(length);
						CxMemFile cf((uint8_t*)pixels, length);

						if (image.Encode2RGBA(&cf, false))
						{
							this->width = w;
							this->height = h;
							this->format = RGBA;
							if (this->data)
								DKFree(this->data);
							this->data = pixels;
							return true;
						}
						DKFree(pixels);
					}
					else
					{
						uint32_t length = w * h * 3;
						pixels = DKMalloc(length);
						CxMemFile cf((uint8_t*)pixels, length);

						if (image.Encode2RGB(&cf, false))
						{
							this->width = w;
							this->height = h;
							this->format = RGB;
							if (this->data)
								DKFree(this->data);
							this->data = pixels;
							return true;
						}
					}
					if (pixels)
						DKFree(pixels);
				}
			}
		}
	}
	return false;
}

DKObject<DKData> DKImage::EncodeData(const DKString& format) const
{
	if (IsValid())
	{
		struct ImageFormat
		{
			const wchar_t* name;
			ENUM_CXIMAGE_FORMATS value;
		};
		std::initializer_list<ImageFormat> formatList =
		{
			{ L"bmp", CXIMAGE_FORMAT_BMP },
			{ L"jpg", CXIMAGE_FORMAT_JPG },
			{ L"jpeg", CXIMAGE_FORMAT_JPG },
			{ L"png", CXIMAGE_FORMAT_PNG },
			{ L"tif", CXIMAGE_FORMAT_TIF },
			{ L"tiff", CXIMAGE_FORMAT_TIF },
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
		size_t bpp = this->BytesPerPixel() * 8;
		CxImage image;
		if (image.CreateFromArray((uint8_t*)data, this->width, this->height, bpp, this->width * bpp, false))
		{
			uint8_t* output = NULL;
			int32_t size = 0;

			DKObject<DKData> result = NULL;

			image.SwapRGB2BGR();
			if (image.Encode(output, size, imageFormat))
			{
				result = DKBuffer::Create(output, size).SafeCast<DKData>();
				image.FreeMemory(output);
			}
			
			return result;
		}
	}
	return NULL;
}

bool DKImage::IsValid(void) const
{
	return width > 0 && height > 0 && data != nullptr;
}

size_t DKImage::BytesPerPixel(void) const
{
	switch (format)
	{
		case Gray: return 1;
		case RGB: return 3;
		case RGBA: return 4;
		case FloatGray: return 4;
		case FloatRGB: return 12;
		case FloatRGBA: return 16;
	}
	return 1;
}

DKObject<DKSerializer> DKImage::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKImage* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKImage");
			this->Bind(L"super", target->DKResource::Serializer(), NULL);

			return this;
		}
	private:
		DKObject<DKImage> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
