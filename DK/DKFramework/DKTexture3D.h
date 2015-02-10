//
//  File: DKTexture3D.h
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
// DKTexture3D
// NOT IMPLEMENTED YET. DO NOT USE.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	// NOT IMPLEMENTED
	class DKLIB_API DKTexture3D : public DKTexture
	{
	public:
		DKTexture3D(void);
		~DKTexture3D(void);


		DKFoundation::DKObject<DKSerializer> Serializer(void);
	};
}
