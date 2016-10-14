//
//  File: DKTexture3D.h
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
// DKTexture3D
// NOT IMPLEMENTED YET. DO NOT USE.
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	// NOT IMPLEMENTED
	class DKGL_API DKTexture3D : public DKTexture
	{
	public:
		DKTexture3D(void);
		~DKTexture3D(void);


		DKObject<DKSerializer> Serializer(void);
	};
}
