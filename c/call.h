#ifndef _WXDIALOGUE_CALL_H_
#define _WXDIALOGUE_CALL_H_
#include "define.h"

// 添加函数调用信息, _argv将会直接被memcpy, 小心提前释放
// _argc也可以用于提前创建内存，当_argv为null时
// 生成的默认你的函数会操作参数
WXDIALOGUE_API WXDLcall* wxdl_new_call(const WXDLchar* _name, WXDLfunction_info* _func, WXDLvalue* _argv, WXDLu32 _argc, WXDLstring_builder* _builder);

WXDIALOGUE_API WXDLcall* wxdl_call_ref(WXDLcall* _call);

// 标记call会操作参数
WXDIALOGUE_API void wxdl_call_set_const_param(WXDLcall* _call, WXDLbool _is_const_param);

// _is_free_param为是否销毁参数
WXDIALOGUE_API void wxdl_free_call(WXDLcall* _call);

WXDIALOGUE_API WXDLcall* wxdl_call_copy(WXDLcall* _v);

// 调用函数
// _is_ref_param是是否在call时使用原本的call的param, 为否的话则拷贝参数
WXDIALOGUE_API WXDLerror wxdl_call_ext(WXDLcall* _v, struct WXDLstate* _state, WXDLvalue* _ret, WXDLthread_resoucre* _pres, WXDLbool _is_ref_param);

// 调用函数
// 这不会释放call
WXDIALOGUE_API WXDLerror wxdl_call(WXDLcall* _v, struct WXDLstate* _state, WXDLvalue* _ret, WXDLthread_resoucre* _pres);

// 清空参数
WXDIALOGUE_API void wxdl_call_clear(WXDLcall* _c);

// 将参数卸下, 但不销毁
WXDIALOGUE_API WXDLvalue wxdl_call_remove(WXDLcall* _call, WXDLu64 _index);

// 将参数卸下, 并销毁
WXDIALOGUE_API void wxdl_call_delete(WXDLcall* _call, WXDLu64 _index);

// 在后面追加参数
WXDIALOGUE_API WXDLvalue* wxdl_call_add_null(WXDLcall* _c);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_bool(WXDLcall* _c, WXDLbool _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_int(WXDLcall* _c, WXDLint _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_u64(WXDLcall* _c, WXDLu64 _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_float(WXDLcall* _c, WXDLfloat _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_str(WXDLcall* _c, const WXDLchar* _v, WXDLstring_builder* _builder);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_str_ref(WXDLcall* _c, WXDLstring* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_hash(WXDLcall* _c, WXDLhash* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_arr(WXDLcall* _c, WXDLarr* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_call(WXDLcall* _c, WXDLcall* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_value(WXDLcall* _c, WXDLvalue* _v);

// 设置置顶位置参数
WXDIALOGUE_API WXDLvalue* wxdl_call_set_null(WXDLcall* _c, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_bool(WXDLcall* _c, WXDLu64 _index, WXDLbool _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_int(WXDLcall* _c, WXDLu64 _index, WXDLint _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_u64(WXDLcall* _c, WXDLu64 _index, WXDLu64 _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_float(WXDLcall* _c, WXDLu64 _index, WXDLfloat _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_str(WXDLcall* _c, WXDLu64 _index, const WXDLchar* _v, WXDLstring_builder* _builder);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_str_ref(WXDLcall* _c, WXDLu64 _index, WXDLstring* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_hash(WXDLcall* _c, WXDLu64 _index, WXDLhash* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_arr(WXDLcall* _c, WXDLu64 _index, WXDLarr* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_call(WXDLcall* _c, WXDLu64 _index, WXDLcall* _v);

// 浅拷贝的
WXDIALOGUE_API WXDLvalue* wxdl_call_set_value(WXDLcall* _c, WXDLu64 _index, WXDLvalue* _v);

// 获取调用的参数数量
WXDIALOGUE_API WXDLu64 wxdl_call_param_size(WXDLcall* _c);

WXDIALOGUE_API WXDLfunction wxdl_call_param_func(WXDLcall* _c);

WXDIALOGUE_API WXDLvalue* wxdl_param_at(WXDLcall* _c, WXDLu64 _index);

// 获取参数(运行call，返回返回值，将返回值设置给_v)
WXDIALOGUE_API WXDLerror wxdl_param_running(struct WXDLstate* _state, WXDLvalue* _v, WXDLvalue* _pv, WXDLthread_resoucre* _pres);

WXDIALOGUE_API WXDLerror wxdl_param_bool(struct WXDLstate* _state, WXDLvalue* _v, WXDLbool* _pv, WXDLthread_resoucre* _pres);

WXDIALOGUE_API WXDLerror wxdl_param_int(struct WXDLstate* _state, WXDLvalue* _v, WXDLint* _pv, WXDLthread_resoucre* _pres);

WXDIALOGUE_API WXDLerror wxdl_param_float(struct WXDLstate* _state, WXDLvalue* _v, WXDLfloat* _pv, WXDLthread_resoucre* _pres);

WXDIALOGUE_API WXDLerror wxdl_param_str(struct WXDLstate* _state, WXDLvalue* _v, const WXDLchar** _pv, WXDLthread_resoucre* _pres);

// 注意返回的值需要手动销毁
WXDIALOGUE_API WXDLerror wxdl_param_str_ref(struct WXDLstate* _state, WXDLvalue* _v, WXDLstring** _pv, WXDLthread_resoucre* _pres);

// 注意返回的值需要手动销毁
WXDIALOGUE_API WXDLerror wxdl_param_hash(struct WXDLstate* _state, WXDLvalue* _v, WXDLhash** _pv, WXDLthread_resoucre* _pres);

// 注意返回的值需要手动销毁
WXDIALOGUE_API WXDLerror wxdl_param_arr(struct WXDLstate* _state, WXDLvalue* _v, WXDLarr** _pv, WXDLthread_resoucre* _pres);

// 注意返回的值需要手动销毁
WXDIALOGUE_API WXDLerror wxdl_param_call(struct WXDLstate* _state, WXDLvalue* _v, WXDLcall** _pv, WXDLthread_resoucre* _pres);

// 注意返回的值需要手动销毁
WXDIALOGUE_API WXDLerror wxdl_param_value(struct WXDLstate* _state, WXDLvalue* _v, WXDLvalue* _pv, WXDLthread_resoucre* _pres);

// 确定参数类型
WXDIALOGUE_API WXDLbool wxdl_param_check(const WXDLvalue* _argv, WXDLu32 _argc, const WXDLflag* flags, WXDLu32 flag_size);


#endif
