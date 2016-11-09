//
//  File: DKUtils.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKArray.h"
#include "DKMap.h"
#include "DKString.h"


namespace DKFoundation
{
	DKGL_API uint32_t DKRandom(void);
	DKGL_API DKString DKTemporaryDirectory(void);
	DKGL_API DKArray<DKString> DKProcessArguments(void);
	DKGL_API DKMap<DKString, DKString> DKProcessEnvironments(void);
	DKGL_API uint32_t DKNumberOfProcessors(void);
}
