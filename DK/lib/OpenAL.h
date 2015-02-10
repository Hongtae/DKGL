#pragma once

////////////////////////////////////////////////////////////////////////////////
// OpenAL 관련
// 맥을 제외한 나머지는 OpenAL-soft 를 사용한다.
// OpenAL-soft 는 LGPL 라이센스 이기 때문에 dynamic-lib 를 사용해야 한다.
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
//#define AL_LIBTYPE_STATIC			// static-lib 을 사용하려면 주석 해제함.
#include "OpenAL/include/AL/al.h"
#include "OpenAL/include/AL/alc.h"
#include "OpenAL/include/AL/alext.h"
#endif
