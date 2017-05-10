#pragma once

////////////////////////////////////////////////////////////////////////////////
// OpenAL library
// Project DK requires OpenAL-soft except for darwin (OS X / iOS).
// You may want to use dynamic-library(dll) version of OpenAL,
// because it was released under 'LGPL' licensed.
// see OpenAL/COPYING
////////////////////////////////////////////////////////////////////////////////

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#include <OpenAL/AL.h>
#include <OpenAL/ALC.h>

#if TARGET_OS_IPHONE
#include <OpenAL/oalMacOSX_OALExtensions.h>
#include <OpenAL/oalStaticBufferExtension.h>
#else
#include <OpenAL/MacOSX_OALExtensions.h>
#endif

#else // if defined(__APPLE__) && defined(__MACH__)
//#define AL_LIBTYPE_STATIC			// uncomment if you need to use static-lib of OpenAL-soft.
#include "../../Libs/OpenAL/include/AL/al.h"
#include "../../Libs/OpenAL/include/AL/alc.h"
#include "../../Libs/OpenAL/include/AL/alext.h"
#endif
