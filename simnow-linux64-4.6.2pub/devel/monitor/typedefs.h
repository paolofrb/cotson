/* 
** Copyright 1995 - 2004 ADVANCED MICRO DEVICES, INC.  All Rights Reserved. 
*/

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#ifdef VERSION
	static char const *typedef_rcsid = "$Id: typedefs.h,v 1.6 2004/03/23 00:26:41 eboleyn Exp $";
#endif

/* This file contains only simple type defines (requiring no other include files) */

#include "basictypes.h"

#undef TRUE
#undef FALSE
#define TRUE	1
#define FALSE	0

#define TRY try
#define CATCH(class, e) catch(class * e) {
	#define END_CATCH }

#define _STRINGIFY(x) #x
// The following will do one level of indirection and apply _STRINGIFY
#define _STRINGIFY_IND(y) _STRINGIFY(y)

#define TRACE0(sz) printf("%s", sz)
#define TRACE printf

#if defined( WIN32 ) || defined( _M_AMD64 )

	#include "windows.h"
	#include "guiddef.h"
	#define snprintf _snprintf
#define strdup _strdup
	#ifdef _WINDLL
		#ifndef HIDE_EXPORTS
			#define	EXPORTCLASS __declspec(dllexport)
			#define DLLEXPORT __declspec(dllexport)
		#else
			#define	EXPORTCLASS
		#endif //HIDE_EXPORTS
	#else
		#define EXPORTCLASS __declspec(dllimport)
		#define DLLIMPORT __declspec(dllimport)
	#endif

#ifdef _SIMNOWDLL
	#define SIMNOWDLLEXPORT __declspec(dllexport)
#else
	#define SIMNOWDLLEXPORT __declspec(dllimport)
#endif

#ifdef _SERVICELIBDLL
	#define	SERVLIBEXPCLASS __declspec(dllexport)
#else  
	#define SERVLIBEXPCLASS __declspec(dllimport)
#endif

	#ifdef _SNFWORK
	#define FWORKAPI __declspec(dllexport)
	#else
	#define FWORKAPI __declspec(dllimport)
	#endif

	#define sign64(num64) num64##i64
	#ifdef usign64
		#undef usign64
	#endif
	#define usign64(num64) num64##ui64

	#define FMT64 "I64"

#else
	#define EXPORTCLASS
	#define DLLEXPORT 
	#define DLLIMPORT extern
	#define APIENTRY
	#define FWORKAPI 
	#define SIMNOWDLLEXPORT
	#define SERVLIBEXPCLASS

	#define MAX_UINT64 0xffffffffffffffffLL
	#define MAX_INT64  0x7fffffffffffffffLL
	#define MIN_UINT64 0x0000000000000000LL
	#define MIN_INT64  0x8000000000000000LL
	#define MAX_PATH          260
	#define NO_ERROR 0

	#define MACRO_itoa_10(_b, _v) (void)sprintf(_b, "%d", _v)
	#define MACRO_itoa_16(_b, _v) (void)sprintf(_b, "%x", _v)
	#define _itoa(_v, _b, _r) MACRO_itoa_##_r(_b, _v)
	#define strnicmp strncasecmp
	#define stricmp strcasecmp
	#define _snprintf snprintf
#define _strdup strdup
	#define _strnicmp strncasecmp
	#define _stricmp strcasecmp
	#define _vsnprintf vsnprintf

	/*basic file operation stuff */
	#define FILE_CURRENT SEEK_CUR
	#define FILE_BEGIN SEEK_SET
	#define FILE_END SEEK_END
	#define INVALID_HANDLE_VALUE NULL
	#define INVALID_SET_FILE_POINTER (-1)
	#define CREATE_ALWAYS 1
	#define OPEN_EXISTING 2
	#define GENERIC_READ                     (0x80000000L)
	#define GENERIC_WRITE                    (0x40000000L)
	#define GENERIC_EXECUTE                  (0x20000000L)
	#define GENERIC_ALL                      (0x10000000L)

	#define PAGE_NOACCESS          0x01     
	#define PAGE_READONLY          0x02     
	#define PAGE_READWRITE         0x04     
	#define PAGE_WRITECOPY         0x08     
	#define PAGE_EXECUTE           0x10     
	#define PAGE_EXECUTE_READ      0x20     
	#define PAGE_EXECUTE_READWRITE 0x40     
	#define PAGE_EXECUTE_WRITECOPY 0x80     
	#define PAGE_GUARD            0x100     
	#define PAGE_NOCACHE          0x200     
	#define PAGE_WRITECOMBINE     0x400 

	#define FILE_SHARE_READ                 0x00000001  
	#define FILE_SHARE_WRITE                0x00000002  
	#define FILE_SHARE_DELETE               0x00000004  
	#define FILE_ATTRIBUTE_READONLY             0x00000001  
	#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
	#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
	#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
	#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
	#define FILE_ATTRIBUTE_DEVICE               0x00000040  
	#define FILE_ATTRIBUTE_NORMAL               0x00000080  
	#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
	#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
	#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
	#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
	#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
	#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
	#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000  

	#define FILE_MAP_WRITE                      0x00000002

	#define Sleep(i) sleep(i)	
	#define CloseHandle(H) fclose((FILE*)H)
	#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
	#define CALLBACK

	#define FMT64 "ll"

	#define sign64(num64) num64##ll
	#define usign64(num64) num64##ull

	// some types defined in windef.h
	typedef void *LPVOID;
	typedef void *HANDLE;
	typedef int MSG;
	typedef void* HICON;
	typedef const char* LPCTSTR;
	typedef const char* LPCSTR;
	typedef char* LPSTR;
	typedef char TCHAR, *PTCHAR;
	typedef char* PTSTR;
	typedef unsigned char BYTE;
	typedef BYTE* PBYTE;
	typedef PBYTE LPBYTE;
	typedef int* LPBOOL;
	typedef unsigned int DWORD;
	typedef unsigned int UINT;
	typedef int LONG;
	typedef LONG* PLONG;
	typedef UINT64 DWORD64;
	typedef UINT ULONG;
	typedef void VOID;
	typedef void *PVOID;
	typedef void *HANDLE;
	typedef UINT64 SIZE_T;
	DECLARE_HANDLE(HINSTANCE);
	typedef HINSTANCE HMODULE;      /* HMODULEs can be used in place of HINSTANCEs */
	typedef int (*FARPROC)();

	typedef INT64 __int64; 

	typedef union _LARGE_INTEGER
	{
		struct
		{
			DWORD LowPart;
			LONG HighPart;
		} u;
		INT64 QuadPart;
	} LARGE_INTEGER;

	typedef LARGE_INTEGER *PLARGE_INTEGER;

#endif

typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef UINT64 ADDRTYPE;

typedef unsigned int *          PUINT32;
typedef signed int *            PINT32;
typedef unsigned short *        PUINT16;
typedef signed short *          PINT16;
typedef unsigned char *         PUINT8;
typedef signed char *           PINT8;
typedef float *                 PFLOAT32;
typedef double *                PFLOAT64;
typedef UINT16*					PFLOAT16;

typedef struct UINT128_
{
	UINT64 lsw;
	UINT64 msw;
#ifdef __cplusplus
	inline UINT128_ operator=(const UINT64 &lo) {msw=0; lsw=lo; return (*this);}
	inline bool operator==(const UINT128_ &val) { return( (lsw == val.lsw) && (msw == val.msw) ); }
	inline bool operator!=(const UINT128_ &val) { return( (lsw != val.lsw) || (msw != val.msw) ); }
	inline UINT128_ &operator>>=(const int &count)
	{
		if( count < 64 )
		{
			UINT64 tmp;
			tmp = msw << (64 - count);
			msw >>= count;
			lsw >>= count;
			lsw |= tmp;
		}
		else if( count < 128 )
		{
			lsw = msw >> (count - 64);
			msw = 0;
		}
		else
			msw = lsw = 0;

		return( *this );
	}
	inline UINT128_ &operator<<=(const int &count)
	{
		if( count < 64 )
		{
			UINT64 tmp;
			tmp = lsw >> (64 - count);
			msw <<= count;
			msw |= tmp;
			lsw <<= count;
		}
		else if( count < 128 )
		{
			msw = lsw << (count - 64);
			lsw = 0;
		}
		else
			msw = lsw = 0;

		return( *this );
	}
#endif //#ifdef __cplusplus
} UINT128;

typedef struct INT128_
{
	UINT64 lsw;
	INT64 msw;
#ifdef __cplusplus
	INT128_() {msw=0;lsw=0;}
	INT128_(INT64 n) {msw=((n&0x8000000000000000)?0xFFFFFFFFFFFFFFFF:0); lsw=n;}
	INT128_(UINT64 n) {msw=0; lsw=n;}
	inline INT128_ operator=(const UINT64 &lo) {msw=0; lsw=lo; return (*this);}
	inline INT128_ operator=(const INT64  &lo) {msw=((lo&0x8000000000000000)?0xFFFFFFFFFFFFFFFF:0); lsw=lo; return (*this);}
	inline INT128_ operator=(const INT128_ &n)  {msw=n.msw; lsw=n.lsw; return (*this);}
	inline bool operator<(const INT128_ &n)  { if (msw == n.msw) return (lsw<n.lsw); else return (msw<n.msw);}
	inline bool operator<(const INT64 &n)  { INT128_ r; r=n; return ((*this)<r);}
	inline bool operator<(const UINT64 &n)  { INT128_ r; r=n; return ((*this)<r);}
	inline bool operator>(const INT128_ &n)  { if (msw == n.msw) return (lsw>n.lsw); else return (msw>n.msw);}
	inline bool operator>(const INT64 &n)  { INT128_ r; r=n; return ((*this)>r);}
	inline bool operator>(const UINT64 &n)  { INT128_ r; r=n; return ((*this)>r);}
	inline bool operator==(const INT128_ &val) { return( (lsw == val.lsw) && (msw == val.msw) ); }
	inline bool operator!=(const INT128_ &val) { return( (lsw != val.lsw) || (msw != val.msw) ); }
	inline INT128_ operator+(const INT128_ &val)
	{
		INT128_ r;
		r.lsw = lsw + val.lsw;
		r.msw = msw + val.msw;
		if(r.lsw < lsw)
			++r.msw;
		return r;
	}
	inline INT128_ operator+(const INT64 &val)  
	{	
		INT128_ r;
		r.msw=((val&0x8000000000000000)?0xFFFFFFFFFFFFFFFF:0); r.lsw=val; 
		return (*this+r);
	}
	inline INT128_ operator+(const UINT64 &val)  
	{	
		INT128_ r;
		r.msw=0;
		r.lsw=val; 
		return (*this+r);
	}
	inline INT128_ operator-(const INT64 &val)
	{
		INT64 r = -val;		
		return (*this+r);
	}
	inline INT128_ operator-(const INT128_ &val)
	{
		INT128_ r = val;		
		r.msw^=0xFFFFFFFFFFFFFFFF;
		r.lsw^=0xFFFFFFFFFFFFFFFF;
		r = r + (UINT64)1;
		return (*this+r);
	}
	inline INT128_ &operator>>=(const int &count)
	{
		if( count < 64 )
		{
			UINT64 tmp;
			tmp = msw << (64 - count);
			msw = ((INT64)msw) >> count;
			lsw >>= count;
			lsw |= tmp;
		}
		else if( count < 128 )
		{
			lsw = msw >> (count - 64);
			msw = 0;
		}
		else
			msw = lsw = 0;

		return( *this );
	}
	inline INT128_ &operator<<=(const int &count)
	{
		if( count < 64 )
		{
			UINT64 tmp;
			tmp = lsw >> (64 - count);
			msw <<= count;
			msw |= tmp;
			lsw <<= count;
		}
		else if( count < 128 )
		{
			msw = lsw << (count - 64);
			lsw = 0;
		}
		else
			msw = lsw = 0;

		return( *this );
	}
#endif //#ifdef __cplusplus
} INT128;

typedef struct FLOAT80
{
	UINT8 buffer[10];
} *PFLOAT80;

typedef enum {IOBYTE, IOWORD, IODWORD} IOSIZE;	// Data Size for IO Transfers.

#define uInt128	UINT128
#define Int128	INT128
#define uInt64	UINT64
#define Int64	INT64
#define uInt32	UINT32
#define Int32	INT32
#define uInt16	UINT16
#define Int16	INT16
#define uInt8	UINT8
#define Int8	INT8
#define Float32	FLOAT32
#define Float64	FLOAT64

#define byte uInt8
#define boolean unsigned
typedef int		BOOL;								
#define RESULT UINT32
#define INVALID_VALUE -1

#define WORD UINT16

typedef WORD HPORT;
#define HPOS UINT16		//Handle to position of the sub-device


#ifndef ATOM
#define ATOM WORD
#endif

#ifndef GUID_DEFINED
#define GUID_DEFINED

typedef struct _GUID {          // size is 16
    DWORD Data1;
    WORD   Data2;
    WORD   Data3;
    BYTE  Data4[8];
} GUID;

#endif // !GUID_DEFINED

#if defined(WIN32)	// Microsoft Visual C/C++ only
#ifndef ASSERT
#ifdef _DEBUG
#pragma warning (disable: 4553 4552) //for warnings during assert
#define ASSERT(exp) exp
#else
#define ASSERT(exp)
#endif
#endif
#endif

#ifndef __cplusplus
#define public
#define private static
#endif

#endif //#ifndef _TYPEDEFS_H_
