#if !defined(__INCLUDED_TYPE_DEFINE____H)
#define __INCLUDED_TYPE_DEFINE____H

typedef int INT32;
typedef unsigned int UINT32;

typedef short INT16;
typedef unsigned short UINT16;

typedef char CHAR;
typedef signed char	 SCHAR;
typedef unsigned char UCHAR;

typedef long LONG;
typedef unsigned long ULONG;

typedef unsigned char UINT8;
typedef char INT8;

#ifndef bool
#define bool	CHAR
#endif

#ifndef _ARC_COMPILER
#ifndef true
#define true	1
#endif

#ifndef false
#define false	0
#endif
#endif ////#ifndef _ARC_COMPILER
#endif  //__INCLUDED_TYPE_DEFINE_H