#ifndef __BASICTYPES_H__
#define __BASICTYPES_H__

// This file includes typedefs for the built-in integer and floats of various sizes, and
// MAX_* and MIN_* constants for integers.  Nothing else should go here.

typedef unsigned int            UINT32;
typedef signed int              INT32;
typedef unsigned short          UINT16;
typedef signed short            INT16;
typedef unsigned char           UINT8;
typedef signed char             INT8;
typedef float                   FLOAT32;
typedef double                  FLOAT64;

#define MAX_UINT32 (0xffffffff)
#define MAX_INT32  (0x7fffffff)
#define MAX_UINT16 (0xffff)
#define MAX_INT16  (0x7fff)
#define MAX_UINT8  (0xff)
#define MAX_INT8   (0x7f)
 
#define MIN_UINT32 (0)
#define MIN_INT32  (-MAX_INT32 - 1)
#define MIN_UINT16 (0)
#define MIN_INT16  (-MAX_INT16 - 1)
#define MIN_UINT8  (0)
#define MIN_INT8   (-MAX_INT8 - 1)

#if defined( WIN32 ) || defined( _M_AMD64 )
	typedef unsigned __int64		UINT64;
	typedef __int64					INT64;

	#define MAX_UINT64 0xffffffffffffffffi64
	#define MAX_INT64  0x7fffffffffffffffi64
	#define MIN_UINT64 0x0000000000000000i64
	#define MIN_INT64  0x8000000000000000i64
#else
	#ifdef __cplusplus
		typedef unsigned long long int	UINT64;
		typedef long long int			INT64;
	#else
		typedef unsigned long int	UINT64;
		typedef long int			INT64;
	#endif

	#define MAX_UINT64 0xffffffffffffffffLL
	#define MAX_INT64  0x7fffffffffffffffLL
	#define MIN_UINT64 0x0000000000000000LL
	#define MIN_INT64  0x8000000000000000LL
#endif



#endif //#ifndef __BASICTYPES_H__
