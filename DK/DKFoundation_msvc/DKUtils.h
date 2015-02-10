//
//  File: DKUtils.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKArray.h"
#include "DKMap.h"
#include "DKString.h"


namespace DKFoundation
{
	DKLIB_API unsigned int DKRandom(void);
	DKLIB_API DKString DKTemporaryDirectory(void);
	DKLIB_API DKArray<DKString> DKProcessArguments(void);
	DKLIB_API DKMap<DKString, DKString> DKProcessEnvironments(void);
}
