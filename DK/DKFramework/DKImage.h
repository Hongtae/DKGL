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
			R8,				///< 1 byte per pixel, uint8
			RG8,			///< 2 bytes per pixel, uint8
			RGB8,			///< 3 bytes per pixel, uint8
			RGBA8,			///< 4 bytes per pixel, uint8
			R16,			///< 2 byte per pixel, uint16
			RG16,			///< 4 bytes per pixel, uint16
			RGB16,			///< 6 bytes per pixel, uint16
			RGBA16,			///< 8 bytes per pixel, uint16
			R32,			///< 4 byte per pixel, uint32
			RG32,			///< 8 bytes per pixel, uint32
			RGB32,			///< 12 bytes per pixel, uint32
			RGBA32,			///< 16 bytes per pixel, uint32
			R32F,			///< 4 bytes per pixel, float32
			RG32F,			///< 8 bytes per pixel, float32
			RGB32F,			///< 12 bytes per pixel, float32
			RGBA32F,		///< 16 bytes per pixel, float32
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

		uint32_t Width(void) const				{ return width; }
		uint32_t Height(void) const				{ return height; }
		size_t BytesPerPixel(void) const;

		bool LoadFromFile(const DKString& path);
		bool LoadFromStream(DKStream* stream);
		bool LoadFromData(DKData* data);
		bool LoadFromData(const void* data, size_t length);
		bool LoadFromPixelData(uint32_t width, uint32_t height, PixelFormat format, const void* data);

		/// resize image using CPU
		bool Resample(uint32_t width, uint32_t height, Interpolation);

		/// encode image data to export (eg, save to common image file formats like png, jpeg)
		DKObject<DKData> EncodeData(const DKString& format) const;

		DKObject<DKSerializer> Serializer(void) override;

	private:
		uint32_t width;
		uint32_t height;
		PixelFormat format;
		void* data;
	};
}
