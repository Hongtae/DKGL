#ifndef TIF_CONFIG_H_INCLUDED
#define TIF_CONFIG_H_INCLUDED

#ifdef _WIN32

#define HAVE_ASSERT_H 1
#define HAVE_FCNTL_H 1
#define HAVE_IEEEFP 1
#define HAVE_JBG_NEWLEN 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_IO_H 1
#define HAVE_SEARCH_H 1
#define HAVE_SETMODE 1

#define SIZEOF_INT 4
#define SIZEOF_LONG 4

/////////////////////////////////////////////////////////////////////////////////////
#define TIFF_INT8_T signed char
#define TIFF_UINT8_T unsigned char
#define TIFF_INT16_T signed short
#define TIFF_UINT16_T unsigned short
#define TIFF_INT32_FORMAT "%d"
#define TIFF_INT32_T signed int
#define TIFF_UINT32_FORMAT "%u"
#define TIFF_UINT32_T unsigned int
#define TIFF_INT64_FORMAT "%I64d"
#define TIFF_INT64_T signed __int64
#define TIFF_UINT64_FORMAT "%I64u"
#define TIFF_UINT64_T unsigned __int64

#if defined(_WIN64)
#define TIFF_SSIZE_T signed __int64
#else
#define TIFF_SSIZE_T signed int
#endif

#if defined(_WIN64)
#define TIFF_SSIZE_FORMAT "%I64d"
#else
#define TIFF_SSIZE_FORMAT "%ld"
#endif

#define TIFF_PTRDIFF_T long

#if _MSC_VER < 1900
#define snprintf _snprintf 
#endif

#define TIF_PLATFORM_CONSOLE

#ifndef __cplusplus
# ifndef inline
#  define inline __inline
# endif
#endif

#define lfind _lfind

#else		/* ifdef _WIN32 */

#define HAVE_ASSERT_H 1
#define HAVE_FCNTL_H 1
#define HAVE_GETOPT 1
#define HAVE_IEEEFP 1
#define HAVE_MMAP 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_STRING_H 1
#define HAVE_UNISTD_H 1
#define STDC_HEADERS 1

#ifdef __LP64__
#define SIZEOF_SIGNED_INT			4
#define SIZEOF_SIGNED_LONG			8
#define SIZEOF_SIGNED_LONG_LONG		8
#define SIZEOF_SIGNED_SHORT			2
#define SIZEOF_UNSIGNED_CHAR_P		8
#define SIZEOF_UNSIGNED_INT			4
#define SIZEOF_UNSIGNED_LONG		8
#define SIZEOF_UNSIGNED_LONG_LONG	8
#define SIZEOF_UNSIGNED_SHORT		2
#else
#define SIZEOF_SIGNED_INT			4
#define SIZEOF_SIGNED_LONG			4
#define SIZEOF_SIGNED_LONG_LONG		8
#define SIZEOF_SIGNED_SHORT			2
#define SIZEOF_UNSIGNED_CHAR_P		4
#define SIZEOF_UNSIGNED_INT			4
#define SIZEOF_UNSIGNED_LONG		4
#define SIZEOF_UNSIGNED_LONG_LONG	8
#define SIZEOF_UNSIGNED_SHORT		2
#endif

#include <stdint.h>
#define TIFF_INT16_T		int16_t
#define TIFF_INT32_FORMAT	"%d"
#define TIFF_INT32_T		int32_t
#define TIFF_INT64_FORMAT	"%lld"
#define TIFF_INT64_T		int64_t
#define TIFF_INT8_T			int8_t
#define TIFF_PTRDIFF_FORMAT	"%ld"
#define TIFF_PTRDIFF_T		ptrdiff_t
#define TIFF_SSIZE_FORMAT	"%ld"
#define TIFF_SSIZE_T		ssize_t
#define TIFF_UINT16_T		uint16_t
#define TIFF_UINT32_FORMAT	"%u"
#define TIFF_UINT32_T		uint32_t
#define TIFF_UINT64_FORMAT	"%llu"
#define TIFF_UINT64_T		uint64_t
#define TIFF_UINT8_T		uint8_t


#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

#endif

#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#if defined(_M_X64) || defined(_M_IX86) || defined(__i386__)
#define __LITTLE_ENDIAN__	1
#else
#define __BIG_ENDIAN__		1
#endif
#endif

/* Native cpu byte order: 1 if big-endian (Motorola) or 0 if little-endian
 (Intel) */
/* Set the native cpu bit order (FILLORDER_LSB2MSB or FILLORDER_MSB2LSB) */
#if defined(__LITTLE_ENDIAN__)
#define HOST_BIGENDIAN 0
#define HOST_FILLORDER FILLORDER_LSB2MSB
#elif defined(__BIG_ENDIAN__)
#define HOST_BIGENDIAN 1
#define HOST_FILLORDER FILLORDER_MSB2LSB
#define WORDS_BIGENDIAN 1
#else
#error Unknown byte order
#endif


#define PACKAGE "tiff"
#define PACKAGE_BUGREPORT "tiff@lists.maptools.org"
#define PACKAGE_NAME "LibTIFF Software"
#define PACKAGE_STRING "LibTIFF Software 4.0.3"
#define PACKAGE_TARNAME "tiff"
#define PACKAGE_URL ""
#define PACKAGE_VERSION "4.0.3"
#define VERSION "4.0.3"

#define STRIPCHOP_DEFAULT TIFF_STRIPCHOP
#define STRIP_SIZE_DEFAULT 8192

#define CHECK_JPEG_YCBCR_SUBSAMPLING 1
#define DEFAULT_EXTRASAMPLE_AS_ALPHA 1

#define CCITT_SUPPORT 1
#define COLORIMETRY_SUPPORT 1
#define CMYK_SUPPORT 1
#define ICC_SUPPORT 1
#define IPTC_SUPPORT 1
#define LOGLUV_SUPPORT 1
#define LZW_SUPPORT 1
#define MDI_SUPPORT 1
#define NEXT_SUPPORT 1
#define PACKBITS_SUPPORT 1
#define PHOTOSHOP_SUPPORT 1
#define PIXARLOG_SUPPORT 1
#define SUBIFD_SUPPORT 1
#define THUNDER_SUPPORT 1
#define YCBCR_SUPPORT 1
#define ZIP_SUPPORT 1

#endif  /* ifndef TIF_CONFIG_H_INCLUDED */
