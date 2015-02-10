//
//  File: DKTexture2D.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKSize.h"
#include "DKRect.h"
#include "DKResource.h"
#include "DKColor.h"
#include "DKTexture.h"

////////////////////////////////////////////////////////////////////////////////
// DKTexture2D
// 2d texture class.
// object can be loaded from various image format.
// (jpg, png, bmp, gif, tga, etc.)
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKTexture2D : public DKTexture
	{
	public:
		DKTexture2D(void);
		~DKTexture2D(void);

		static DKFoundation::DKObject<DKTexture2D> Create(int width, int height, Format imageFormat, Type dataType, const void* data = NULL);
		static DKFoundation::DKObject<DKTexture2D> Create(const DKFoundation::DKString& file, int width, int height);
		static DKFoundation::DKObject<DKTexture2D> Create(const DKFoundation::DKString& file);
		static DKFoundation::DKObject<DKTexture2D> Create(const DKFoundation::DKData* data, int width, int height);
		static DKFoundation::DKObject<DKTexture2D> Create(const DKFoundation::DKData* data);
		static DKFoundation::DKObject<DKTexture2D> Create(DKFoundation::DKStream* stream, int width, int height);
		static DKFoundation::DKObject<DKTexture2D> Create(DKFoundation::DKStream* stream);

		void SetPixelData(const DKRect& rc, const void* data);
		DKFoundation::DKObject<DKFoundation::DKData> CopyPixelData(const DKRect& rc, Format format=FormatUnknown, Type type=TypeUnsignedByte) const;

		DKFoundation::DKObject<DKTexture2D> Duplicate(void) const;

		DKFoundation::DKObject<DKFoundation::DKData> CreateImageFileData(const DKFoundation::DKString& format) const;

		bool Deserialize(const DKFoundation::DKXMLElement* e, DKResourceLoader* loader);
		DKFoundation::DKObject<DKFoundation::DKXMLElement> SerializeXML(DKSerializer::SerializeForm sf) const;
		DKFoundation::DKObject<DKSerializer> Serializer(void);
	};
}
