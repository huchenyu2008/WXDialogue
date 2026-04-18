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
#define WXDL_VERSION 1002

// 调用函数最大参数数量
#define WXDL_FUNC_MAX_PARAM_COUNT 65534

#define WXDL_FUNC_NAME_GET_GLOBAL_VAR "getvar"

// 缓存大小
#define WXDL_BUFF_SIZE 4096

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

// 数组
typedef struct WXDLarr WXDLarr;

// 表(哈希表)
typedef struct WXDLhash WXDLhash;

// 字符串构建器
typedef struct WXDLstring_builder WXDLstring_builder;

// 加载器
typedef struct  WXDLloader WXDLloader;

// 保存的状态机
typedef struct WXDLsaver WXDLsaver;

// 数据块
typedef struct WXDLblock WXDLblock;


// 字符串引用
typedef struct WXDLstring
{
	WXDLchar* str;
	WXDLu64 hashcode;
	WXDLu32 refcount;
	WXDLu32 len;
	WXDLstring_builder* builder;
}WXDLstring;

// 数据
typedef union WXDLdata
{
	WXDLbool b;
	WXDLint i;
	WXDLfloat f;
	WXDLu64 u;
	WXDLptr p;

	WXDLstring* s;

	WXDLhash* d;

	WXDLarr* a;

	struct WXDLcall* c;
}WXDLdata;

// 数值
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
		#define WXDL_TYPE_CALL 8
	// 标识
	WXDLflag flag;
	WXDLdata data;
}WXDLvalue;

// 调用函数
typedef WXDLerror (*WXDLfunction)(struct WXDLloader* loader, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret);

// 函数调用信息
typedef struct WXDLcall
{
    WXDLvalue* argv;
    WXDLu32 argc;
    WXDLu32 max_argc;
    WXDLfunction func;

    WXDLstring_builder* builder;
    WXDLstring* name;
	WXDLstring* where;
	WXDLu32 line;
	WXDLu32 xpos;
}WXDLcall;

// 文本缓存组
typedef struct WXDLbuff_set WXDLbuff_set;

// 文本缓存
typedef struct WXDLbuff WXDLbuff;

#endif //_WX_WXDIALOGUE_DEFINE_H_
