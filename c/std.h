#ifndef _WXDIALOGUE_STD_H_
#define _WXDIALOGUE_STD_H_
#include "./define.h"



#define WXDL_V_TYPE(v) (v).type

#define WXDL_V_FLAG(v) (v).flag


// get value's data

#define WXDL_V_INT(v) (v).data.i

#define WXDL_V_UINT(v) (v).data.u

#define WXDL_V_FLOAT(v) (v).data.f

#define WXDL_V_BOOL(v) (v).data.b

#define WXDL_V_STR(v) (v).data.s

#define WXDL_V_ARR(v) (v).data.a

#define WXDL_V_DIC(v) (v).data.d

#define WXDL_V_CALL(v) (v).data.c

#define WXDL_V_PTR(v) (v).data.p
// set value's data

#define WXDL_V_SET_NULL(v) ((v).data.p = 0, WXDL_V_TYPE(v) = WXDL_TYPE_NULL, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_INT(v, n) ((v).data.i = n, WXDL_V_TYPE(v) = WXDL_TYPE_INT, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_UINT(v, n) ((v).data.u = n, WXDL_V_TYPE(v) = WXDL_TYPE_INT, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_FLOAT(v, n) ((v).data.f = n, WXDL_V_TYPE(v) = WXDL_TYPE_FLOAT, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_BOOL(v, n) ((v).data.b = n, WXDL_V_TYPE(v) = WXDL_TYPE_BOOL, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_STR(v, n) ((v).data.s = n, WXDL_V_TYPE(v) = WXDL_TYPE_STR, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_ARR(v, n) ((v).data.a = n, WXDL_V_TYPE(v) = WXDL_TYPE_ARR, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_DIC(v, n) ((v).data.d = n, WXDL_V_TYPE(v) = WXDL_TYPE_DIC, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_CALL(v, n) ((v).data.c = n, WXDL_V_TYPE(v) = WXDL_TYPE_CALL, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_PTR(v, n) ((v).data.c = n, WXDL_V_TYPE(v) = WXDL_TYPE_PTR, WXDL_V_FLAG(v) = 0)

#define WXDL_V_SET_ARR_REF(v, n) ((v).data.a = n, WXDL_V_TYPE(v) = WXDL_TYPE_ARR, WXDL_V_FLAG(v) = 1)

#define WXDL_V_SET_DIC_REF(v, n) ((v).data.d = n, WXDL_V_TYPE(v) = WXDL_TYPE_DIC, WXDL_V_FLAG(v) = 1)

#define WXDL_V_SET_CALL_REF(v, n) ((v).data.c = n, WXDL_V_TYPE(v) = WXDL_TYPE_CALL, WXDL_V_FLAG(v) = 1)


// 获取当前线程id
WXDIALOGUE_API WXDLu32 wxdl_get_thread_id();

// 分配内存
WXDIALOGUE_API WXDLptr wxdl_malloc(WXDLint _malloc_size);

// 深拷贝
// _pid 是通过wxdl_state_new_pid获取, 保证多线程安全操作的
// 假如你的文本没那么复杂, 比如没用寄存器什么的, 那将_pid设置为 WXDL_INVALID_PID
// 用寄存器的话就加, 因为寄存器在pid的资源里
WXDIALOGUE_API void wxdl_value_copy_running(WXDLvalue* _v1, WXDLvalue* _v2, struct WXDLstate* _state, WXDLthread_resoucre* _pres);

// 深拷贝
WXDIALOGUE_API void wxdl_value_copy(WXDLvalue* _v1, WXDLvalue* _v2);

// 浅拷贝
// _pid 是通过wxdl_state_new_pid获取, 保证多线程安全操作的
// 假如你的文本没那么复杂, 比如没用寄存器什么的, 那将_pid设置为 WXDL_INVALID_PID
// 用寄存器的话就加, 因为寄存器在pid的资源里
WXDIALOGUE_API void wxdl_value_shallow_copy_running(WXDLvalue* _v1, WXDLvalue* _v2, struct WXDLstate* _state, WXDLthread_resoucre* _pres);

// 浅拷贝
WXDIALOGUE_API void wxdl_value_shallow_copy(WXDLvalue* _v1, WXDLvalue* _v2);

WXDIALOGUE_API void wxdl_free(WXDLptr _p);

WXDIALOGUE_API void wxdl_free_value(WXDLvalue* _pv);

// 删除一个数组的变量
WXDIALOGUE_API void wxdl_free_value_arr(WXDLvalue* _pv, WXDLu64 count);

WXDIALOGUE_API void wxdl_set(WXDLptr _buff, WXDLint32 _v, WXDLu64 _size);

WXDIALOGUE_API void wxdl_copy(WXDLptr _buff, const WXDLptr _copy, WXDLu64 _size);

WXDIALOGUE_API WXDLchar* wxdl_new_str(const WXDLchar* _str);

WXDIALOGUE_API WXDLu64 wxdl_str_len(const WXDLchar* _str);

// 0是相等
WXDIALOGUE_API WXDLbool wxdl_str_cmp(const WXDLchar* _s1, const WXDLchar* _s2);

WXDIALOGUE_API WXDLu64 wxdl_str_hashcode(const WXDLchar* _str);

// 判断一个类型是否可以转为另一个类型
WXDIALOGUE_API WXDLbool wxdl_is_type_convert(WXDLflag _t1, WXDLflag _t2);

// value的值获取

WXDIALOGUE_API WXDLbool wxdl_value_bool(WXDLvalue* _v);

WXDIALOGUE_API WXDLint wxdl_value_int(WXDLvalue* _v);

WXDIALOGUE_API WXDLu64 wxdl_value_u64(WXDLvalue* _v);

WXDIALOGUE_API WXDLfloat wxdl_value_float(WXDLvalue* _v);

WXDIALOGUE_API const WXDLchar* wxdl_value_str(WXDLvalue* _v);

WXDIALOGUE_API WXDLstring* wxdl_value_str_ref(WXDLvalue* _v);

WXDIALOGUE_API WXDLhash* wxdl_value_hash(WXDLvalue* _v);

WXDIALOGUE_API WXDLarr* wxdl_value_arr(WXDLvalue* _v);

WXDIALOGUE_API WXDLcall* wxdl_value_call(WXDLvalue* _v);

#endif
