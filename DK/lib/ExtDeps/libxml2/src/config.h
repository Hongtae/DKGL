#pragma once

////////////////////////////////////////////////////////////////////////////////
// 2013-05-21: iconv 제거
// 2012-10-24: libxml2-2.9.0 으로 버전업 하면서 다시 만듦.

#if defined(_WIN32)

#define HAVE_ISINF
#define HAVE_ISNAN
#define HAVE_ZLIB_H
#define HAVE_LIBZ
#define HAVE_CTYPE_H
#define HAVE_STDARG_H
#define HAVE_MALLOC_H
#define HAVE_ERRNO_H
#define HAVE_SYS_STAT_H
#define HAVE_STAT
#define HAVE__STAT
#define HAVE_STDLIB_H
#define HAVE_TIME_H
#define HAVE_FCNTL_H
#define HAVE_STRFTIME
#define HAVE_STRING_H
#define HAVE_SNPRINTF
#define HAVE_SPRINTF
#define HAVE_SSCANF
#define HAVE_VFPRINTF
#define HAVE_VSNPRINTF
#define HAVE_VSPRINTF
#define HAVE_PRINTF
#define HAVE_LIMITS_H
#define HAVE_LOCALTIME
#define HAVE_MATH_H
#define HAVE_MEMORY_H
#define HAVE_STRDUP
#define HAVE_STRERROR
#define HAVE_FINITE
#define HAVE_FLOAT_H
#define HAVE_FPRINTF
#define HAVE_FTIME

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <io.h>
#include <direct.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#ifdef NEED_SOCKETS
#include <wsockcompat.h>
#endif

#ifndef isinf
#define isinf(d) ((_fpclass(d) == _FPCLASS_PINF) ? 1 : ((_fpclass(d) == _FPCLASS_NINF) ? -1 : 0))
#endif
/* _isnan(x) returns nonzero if (x == NaN) and zero otherwise. */
#ifndef isnan
#define isnan(d) (_isnan(d))
#endif

#define mkdir(p,m) _mkdir(p)
#define snprintf _snprintf
#define vsnprintf(b,c,f,a) _vsnprintf(b,c,f,a)
#define HAVE_WIN32_THREADS

#pragma comment(lib, "Ws2_32.lib")

#else	// if defined(_WIN32)

#define HAVE_ARPA_INET_H 1
#define HAVE_ARPA_NAMESER_H 1
#define HAVE_CTYPE_H 1
#define HAVE_DIRENT_H 1
#define HAVE_DLFCN_H 1
#define HAVE_DLOPEN
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_FINITE 1
#define HAVE_FLOAT_H 1
#define HAVE_FPRINTF 1
#define HAVE_FTIME 1
#define HAVE_GETADDRINFO /**/
#define HAVE_GETTIMEOFDAY 1
#define HAVE_INTTYPES_H 1
#define HAVE_ISASCII 1
#define HAVE_ISINF /**/
#define HAVE_ISNAN /**/
#define HAVE_LIBPTHREAD /**/
#define HAVE_LIBZ 1
#define HAVE_LIMITS_H 1
#define HAVE_LOCALTIME 1
#define HAVE_MALLOC_H /**/
#define HAVE_MATH_H 1
#define HAVE_MEMORY_H 1
#define HAVE_MMAP 1
#define HAVE_MUNMAP 1
#define HAVE_NETDB_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_POLL_H 1
#define HAVE_PRINTF 1
#define HAVE_PTHREAD_H /**/
#define HAVE_PUTENV 1
#define HAVE_RAND 1
#define HAVE_RAND_R 1
#define HAVE_RESOLV_H 1
#define HAVE_SIGNAL 1
#define HAVE_SIGNAL_H 1
#define HAVE_SNPRINTF 1
#define HAVE_SPRINTF 1
#define HAVE_SRAND 1
#define HAVE_SSCANF 1
#define HAVE_STAT 1
#define HAVE_STDARG_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRFTIME 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRNDUP 1
#define HAVE_SYS_MMAN_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIMEB_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TIME 1
#define HAVE_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_VA_COPY 1
#define HAVE_VFPRINTF 1
#define HAVE_VSNPRINTF 1
#define HAVE_VSPRINTF 1
#define HAVE_ZLIB_H 1

#define STDC_HEADERS 1
#define SUPPORT_IP6 /**/

#define XML_SOCKLEN_T socklen_t

#endif

#define LIBXML_THREAD_ENABLED
#define LIBXML_STATIC

#include <libxml/xmlversion.h>
