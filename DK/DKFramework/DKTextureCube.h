//
//  File: DKTextureCube.h
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

///////////////////////////////////////////////////////////////////////////////
// DKTextureCube
// cube texture.
//
// Note:
//    This class is not tested.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKRenderTarget;
	class DKLIB_API DKTextureCube : public DKTexture
	{
	public:
		DKTextureCube(void);
		~DKTextureCube(void);

		static DKFoundation::DKObject<DKTextureCube> Create(int width, int height, Format imageFormat, Type dataType);

		DKRenderTarget* GetRenderTarget(void);

		DKFoundation::DKObject<DKSerializer> Serializer(void);

	private:
		DKFoundation::DKObject<DKRenderTarget>	renderTarget;
	};
}
