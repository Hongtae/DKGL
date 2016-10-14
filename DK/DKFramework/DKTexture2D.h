//
//  File: DKTexture2D.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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

namespace DKGL
{
	class DKGL_API DKTexture2D : public DKTexture
	{
	public:
		DKTexture2D(void);
		~DKTexture2D(void);

		static DKObject<DKTexture2D> Create(int width, int height, Format imageFormat, Type dataType, const void* data = NULL);
		static DKObject<DKTexture2D> Create(const DKString& file, int width, int height);
		static DKObject<DKTexture2D> Create(const DKString& file);
		static DKObject<DKTexture2D> Create(const DKData* data, int width, int height);
		static DKObject<DKTexture2D> Create(const DKData* data);
		static DKObject<DKTexture2D> Create(DKStream* stream, int width, int height);
		static DKObject<DKTexture2D> Create(DKStream* stream);

		void SetPixelData(const DKRect& rc, const void* data);
		DKObject<DKData> CopyPixelData(const DKRect& rc, Format format=FormatUnknown, Type type=TypeUnsignedByte) const;

		DKObject<DKTexture2D> Duplicate(void) const;

		// CreateImageFileData: this function will write to file, which assumes format as RGB or RGBA.
		DKObject<DKData> CreateImageFileData(const DKString& format) const;

		bool Deserialize(const DKXMLElement* e, DKResourceLoader* loader);
		DKObject<DKXMLElement> SerializeXML(DKSerializer::SerializeForm sf) const;
		DKObject<DKSerializer> Serializer(void);
	};
}
