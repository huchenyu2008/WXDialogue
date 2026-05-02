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
#define WXDL_VERSION 2002

// 调用函数最大参数数量
#define WXDL_FUNC_MAX_PARAM_COUNT 65534

#define WXDL_FUNC_NAME_GET_GLOBAL_VAR "VAR"

#define WXDL_FUNC_NAME_IF "IF"

#define WXDL_FUNC_NAME_BLOCK "BLOCK"

#define WXDL_FUNC_NAME_PRINT "PRINT"

#define WXDL_FUNC_NAME_REG "REG"

#define WXDL_FUNC_NAME_ROOT "ROOT"

#define WXDL_FUNC_NAME_VEC "VEC"

// string
#define WXDL_FUNC_NAME_STR_CMP "STRCMP"

#define WXDL_FUNC_NAME_STR_CAT "STRCAT"

#define WXDL_FUNC_NAME_STR_LEN "STRLEN"

// num op
#define WXDL_FUNC_NAME_ADD "ADD"

#define WXDL_FUNC_NAME_SUB "SUB"

#define WXDL_FUNC_NAME_MUL "MUL"

#define WXDL_FUNC_NAME_DIV "DIV"

#define WXDL_FUNC_NAME_MOD "MOD"

#define WXDL_FUNC_NAME_AND "AND"

#define WXDL_FUNC_NAME_OR "OR"

#define WXDL_FUNC_NAME_BAND "BAND"

#define WXDL_FUNC_NAME_BOR "BOR"

#define WXDL_FUNC_NAME_NOT "NOT"

// num func
#define WXDL_FUNC_NAME_POW "POW"

#define WXDL_FUNC_NAME_LOG "LOG"

#define WXDL_FUNC_NAME_LOG2 "LOG2"

#define WXDL_FUNC_NAME_LOG10 "LOG10"

#define WXDL_FUNC_NAME_SQRT "SQRT"

#define WXDL_FUNC_NAME_SIN "SIN"

#define WXDL_FUNC_NAME_COS "COS"

#define WXDL_FUNC_NAME_TAN "TAN"

#define WXDL_FUNC_NAME_ASIN "ASIN"

#define WXDL_FUNC_NAME_ACOS "ACOS"

#define WXDL_FUNC_NAME_ATAN "ATAN"

// 缓存大小
#define WXDL_BUFF_SIZE 4096

// 寄存器数量
#define WXDL_REG_SIZE 8

// 无效的PID
#define WXDL_INVALID_PID ((WXDLu32)-1)


#if defined(_MSC_VER)
    #include <intrin.h>
    // Windows MSVC
    #define CPU_PAUSE() _mm_pause()

#elif defined(__GNUC__) || defined(__clang__)
    // Linux/Mac GCC/Clang
    #if defined(__x86_64__) || defined(__i386__)
        #define CPU_PAUSE() __asm__ volatile ("pause" ::: "memory")
    #elif defined(__aarch64__)
        #define CPU_PAUSE() __asm__ volatile ("isb" ::: "memory")
    #elif defined(__arm__)
        #define CPU_PAUSE() __asm__ volatile ("yield" ::: "memory")
    #else
        #define CPU_PAUSE() ((void)0)
    #endif
#else
    #define CPU_PAUSE() ((void)0)
#endif

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

// 状态机
typedef struct WXDLstate WXDLstate;

// log缓存
typedef struct WXDLlogbuff
{
    WXDLchar* logbuff;
    WXDLu64 buffsize;
    WXDLu64 bufflen;
}WXDLlogbuff;

// 每个pid独自的资源
typedef struct WXDLthread_resoucre WXDLthread_resoucre;

// 每个文本节点
typedef struct WXDLtext_node
{
	// 节点类型
	WXDLflag type;
		// 文本
		#define WXDL_TEXT_NODE_TEXT 0
		// 签名
		#define WXDL_TEXT_NODE_SIGN 1

	// type == WXDL_TEXT_NODE_TEXT : 此为字符串文本
	// type == WXDL_TEXT_NODE_SIGN : 此为数据标签的名称(全局签名表名称)
	WXDLchar* text;

	// 储存标签数据
	// type == WXDL_TEXT_NODE_TEXT 时, 常态为NULL
	struct WXDLhash* data;

	struct WXDLtext_node* next;

	// 使用的独立签名表名称
	struct WXDLarr* use_local_tables;
}WXDLtext_node;

// 文本块们
typedef struct WXDLtext WXDLtext;

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
	    // 函数不更改参数
	    #define  WXDL_FLAG_FUNC_NOT_SET_PARAM 8
	WXDLdata data;
}WXDLvalue;

typedef struct WXDLhash_node
{
	WXDLstring* k;
	WXDLvalue v;
	struct WXDLhash_node* next;
}WXDLhash_node;

// 调用函数
typedef WXDLerror (*WXDLfunction)(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* _pres);

// 函数调用信息
typedef struct WXDLcall
{
    WXDLvalue* argv;
    WXDLu32 argc;
    WXDLu32 max_argc;
    WXDLfunction func;

    WXDLstring* name;
	WXDLstring* where;
	WXDLu32 line;
	WXDLu32 xpos;
	WXDLu32 xpos_st;

	// 是否不会操作参数
	WXDLbool is_const_param;
	WXDLint32 refcount;
}WXDLcall;

typedef struct WXDLfunction_info
{
    WXDLfunction func;
    WXDLbool is_change_param;
}WXDLfunction_info;

// 文本缓存组
typedef struct WXDLbuff_set WXDLbuff_set;

// 文本缓存
typedef struct WXDLbuff WXDLbuff;

#endif //_WX_WXDIALOGUE_DEFINE_H_
