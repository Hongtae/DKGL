//
//  File: DKString.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKStringUE.h"
#include "DKStringU8.h"
#include "DKStringW.h"

////////////////////////////////////////////////////////////////////////////////
// DKString
// basic string class.
// using utf-16 for Win32, else utf-32 (for Unix, Mac OS X) with current implementation.
// using wchar_t for character type. (see DKStringW.h)
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	typedef DKStringW DKString;
}
