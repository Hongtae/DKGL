//
//  File: DKTextureCube.h
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

///////////////////////////////////////////////////////////////////////////////
// DKTextureCube
// cube texture.
//
// Note:
//    This class is not tested.
///////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKRenderTarget;
	class DKGL_API DKTextureCube : public DKTexture
	{
	public:
		DKTextureCube(void);
		~DKTextureCube(void);

		static DKObject<DKTextureCube> Create(int width, int height, Format imageFormat, Type dataType);

		DKRenderTarget* GetRenderTarget(void);

		DKObject<DKSerializer> Serializer(void);

	private:
		DKObject<DKRenderTarget>	renderTarget;
	};
}
