//
//  File: DKImage.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "../Libs/libpng/png.h"
#include "../Libs/jpeg/jpeglib.h"

#include "DKImage.h"

namespace DKFramework
{
	namespace Private
	{
		enum {
			FormatPNG,
			FormatJPEG,
			FormatTGA,
			FormatBMP
		};

		inline size_t BytesPerPixel(DKImage::PixelFormat format)
		{
			switch (format)
			{
			case DKImage::R8:		return 1;
			case DKImage::RG8:		return 2;
			case DKImage::RGB8:		return 3;
			case DKImage::RGBA8:	return 4;
			case DKImage::R16:		return 2;
			case DKImage::RG16:		return 4;
			case DKImage::RGB16:	return 6;
			case DKImage::RGBA16:	return 8;
			case DKImage::R32:		return 4;
			case DKImage::RG32:		return 8;
			case DKImage::RGB32:	return 12;
			case DKImage::RGBA32:	return 16;
			case DKImage::R32F:		return 4;
			case DKImage::RG32F:	return 8;
			case DKImage::RGB32F:	return 12;
			case DKImage::RGBA32F:	return 16;
			}
			return 0;
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKImage::DKImage(void)
	: data(NULL)
	, width(0)
	, height(0)
	, format(R8)
{
}

DKImage::~DKImage(void)
{
	if (data)
		DKFree(data);
}

bool DKImage::LoadFromPixelData(uint32_t w, uint32_t h, PixelFormat fmt, const void* p)
{
	size_t bpp = Private::BytesPerPixel(fmt);
	if (bpp && w && h)
	{
		size_t imageSize = w * h * bpp;

		void* imageData = DKMalloc(imageSize);
		if (imageData)
		{
			if (this->data)
				DKFree(this->data);

			this->data = imageData;
			this->width = w;
			this->height = h;

			if (p)
			{
				memcpy(data, p, imageSize);
			}
			else
			{
				memset(data, 0, imageSize);
			}
			return true;
		}
		else
		{
			DKLogE("Out of memory!");
		}
	}
	else
	{
		DKLogE("DKImage error: Invalid format!");
	}
	return false;
}

bool DKImage::LoadFromFile(const DKString &path)
{
	if (path.Length() == 0)
		return false;

	DKObject<DKFileMap> file = DKFileMap::Open(path, 0, false);
	if (file)
		return LoadFromData(file.SafeCast<DKData>());
	return false;
}

bool DKImage::LoadFromData(const void *p, size_t length)
{
	if (p == NULL || length == 0)
		return false;

	DKObject<DKData> data = DKData::StaticData(p, length);
	if (data)
		return LoadFromData(data);
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

	return false;
}

DKObject<DKData> DKImage::EncodeData(const DKString& format) const
{
	return NULL;
}

size_t DKImage::BytesPerPixel(void) const
{
	size_t bpp = Private::BytesPerPixel(format);
	DKASSERT_DESC_DEBUG(bpp, "Invalid format");
	return bpp;
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
