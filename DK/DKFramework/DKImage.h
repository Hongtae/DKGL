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
		DKImage();
		~DKImage();

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

		uint32_t Width() const				{ return width; }
		uint32_t Height() const				{ return height; }
		size_t BytesPerPixel() const;
		bool IsValid() const;

		static DKObject<DKImage> Create(const DKString& path);
		static DKObject<DKImage> Create(DKStream* stream);
		static DKObject<DKImage> Create(DKData* data);
		static DKObject<DKImage> Create(const void* data, size_t length);
		static DKObject<DKImage> Create(uint32_t width, uint32_t height, PixelFormat format, const void* data);

		/// Resize image using CPU.
		/// If given operation-queue is valid, this operation will be processed with multi-threaded. 
		DKObject<DKImage> Resample(uint32_t width, uint32_t height, PixelFormat format, Interpolation, DKOperationQueue*) const;

		/// encode image data to export (eg, save to common image file formats like png, jpeg)
		DKObject<DKData> EncodeData(const DKString& format, DKOperationQueue*) const;

		DKObject<DKSerializer> Serializer() override;

	private:
		uint32_t width;
		uint32_t height;
		PixelFormat format;
		void* data;
	};
}
