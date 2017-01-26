//
//  File: DKImage.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKRect.h"

namespace DKFramework
{
	/// a graphics image data represents 2D picture.
	class DKGL_API DKImage : public DKResource
	{
	public:
		DKImage(void);
		~DKImage(void);

		enum PixelFormat
		{
			Gray,		///< 1byte per pixel
			RGB,		///< 3bytes per pixel
			RGBA,		///< 4bytes per pixel
			FloatGray,	///< 4bytes per pixel
			FloatRGB,	///< 12bytes per pixel
			FloatRGBA,	///< 16bytes per pixel
		};

		enum Interpolation
		{
			Nearest,
			Bilinear,
			Bicubic,
			Spline,
			Lanczos,
			Gaussian,
			Quadratic,
		};

		bool IsValid(void) const;
		size_t BytesPerPixel(void) const;

		bool LoadFromFile(const DKString& path);
		bool LoadFromStream(DKStream* stream);
		bool LoadFromData(DKData* data);
		bool LoadFromData(const void* data, size_t length);
		bool LoadFromPixelData(uint32_t width, uint32_t height, PixelFormat, const void*p);

		bool Resample(uint32_t width, uint32_t height, Interpolation);

		DKObject<DKData> EncodeData(const DKString& format) const;


		DKObject<DKSerializer> Serializer(void) override;
	private:
		void* data;
		PixelFormat format;
		uint32_t width;
		uint32_t height;
	};
}
