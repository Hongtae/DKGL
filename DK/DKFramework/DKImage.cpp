//
//  File: DKImage.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKImage.h"

using namespace DKFramework;

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

bool DKImage::LoadFromPixelData(uint32_t width, uint32_t height, PixelFormat format, const void* data)
{
	return false;
}

bool DKImage::LoadFromFile(const DKString &path)
{
	if (path.Length() == 0)
		return false;


	return false;
}

bool DKImage::LoadFromData(const void *data, size_t length)
{
	if (data == NULL || length == 0)
		return false;


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
