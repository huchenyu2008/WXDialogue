#ifndef _WX_WXDIALOGUE_DEFINE_H_
#define _WX_WXDIALOGUE_DEFINE_H_

// wx dialogue export
#ifdef _WXDIALOGUE_DLL
#ifdef _MSC_VER
#define WXDIALOGUE_API _declspec(dllexport)

#else
#define WXDIALOGUE_API
#endif

#else
#ifdef _MSC_VER
#define WXDIALOGUE_API _declspec(dllimport)
#else
#define WXDIALOGUE_API
#endif
#endif
#include <stdio.h>
#define WXDL_VERSION 1


typedef void* WXDLptr;

typedef int WXDLbool;

typedef int WXDLint32;

typedef double WXDLfloat;

typedef long long WXDLint;

typedef unsigned long long WXDLu64;

typedef unsigned int WXDLu32;

typedef char WXDLchar;

typedef int WXDLflag;

typedef long long WXDLerror;

#define WXDL_FALSE 0
#define WXDL_TRUE 1

typedef union WXDLdata
{
	WXDLbool b;
	WXDLint i;
	WXDLfloat f;
	WXDLu64 u;
	WXDLchar* s;
	WXDLptr p;

	struct WXDLhash* d;

	struct WXDLarr* a;
}WXDLdata;


typedef struct WXDLarr WXDLarr;
typedef struct WXDLhash WXDLhash;

typedef struct WXDLvalue
{
	// 类型
	WXDLflag type;		
		#define WXDL_TYPE_NULL 0
		#define WXDL_TYPE_BOOL 1
		#define WXDL_TYPE_INT 2
		#define WXDL_TYPE_FLOAT 3
		#define WXDL_TYPE_STR 4
		#define WXDL_TYPE_ARR 5
		#define WXDL_TYPE_DIC 6
		#define WXDL_TYPE_PTR 7
	// 标识
	WXDLflag flag;
	WXDLdata data;
}WXDLvalue;

#endif //_WX_WXDIALOGUE_DEFINE_H_