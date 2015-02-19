﻿//
//  File: DKMath.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include <math.h>
#include <float.h>
#include "../DKInclude.h"

// DKMath.h
// predefined math macros.

#define DKL_MATH_E			2.71828182845904523536028747135266250   /* e */
#define DKL_MATH_LOG2E		1.44269504088896340735992468100189214   /* log 2e */
#define DKL_MATH_LOG10E		0.434294481903251827651128918916605082  /* log 10e */
#define DKL_MATH_LN2		0.693147180559945309417232121458176568  /* log e2 */
#define DKL_MATH_LN10		2.30258509299404568401799145468436421   /* log e10 */
#define DKL_MATH_PI			3.14159265358979323846264338327950288   /* pi */
#define DKL_MATH_PI_2		1.57079632679489661923132169163975144   /* pi/2 */
#define DKL_MATH_PI_4		0.785398163397448309615660845819875721  /* pi/4 */
#define DKL_MATH_1_PI		0.318309886183790671537767526745028724  /* 1/pi */
#define DKL_MATH_2_PI		0.636619772367581343075535053490057448  /* 2/pi */
#define DKL_MATH_2_SQRTPI	1.12837916709551257389615890312154517   /* 2/sqrt(pi) */
#define DKL_MATH_SQRT2		1.41421356237309504880168872420969808   /* sqrt(2) */
#define DKL_MATH_SQRT1_2	0.707106781186547524400844362104849039  /* 1/sqrt(2) */

#define DKL_PI						DKL_MATH_PI
#define DKL_DEGREE_TO_RADIAN(r)		((r) * DKL_PI / 180.0)
#define DKL_RADIAN_TO_DEGREE(r)		((r) * 180.0 / DKL_PI)