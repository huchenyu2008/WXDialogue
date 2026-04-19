#ifndef _WXDIALOGUE_CALL_H_
#define _WXDIALOGUE_CALL_H_
#include "define.h"

// 添加函数调用信息, _argv将会直接被memcpy, 小心提前释放
WXDIALOGUE_API WXDLcall* wxdl_new_call(const WXDLchar* _name, WXDLfunction _func, WXDLvalue* _argv, WXDLu32 _argc, WXDLstring_builder* _builder);

// _is_free_param为是否销毁参数
WXDIALOGUE_API void wxdl_free_call(WXDLcall* _call, WXDLbool _is_free_param);

WXDIALOGUE_API WXDLcall* wxdl_call_copy(WXDLcall* _v);

// 调用函数
// _after_destroy是是否在调用后释放call
// 正常_after_destroy都是启用的, 除了在call期间的get_param操作
WXDIALOGUE_API WXDLerror wxdl_call_ext(WXDLcall* _v, struct WXDLloader* _loader, WXDLvalue* _ret, WXDLbool _after_destroy);

// 调用函数
// 这不会释放call
WXDIALOGUE_API WXDLerror wxdl_call(WXDLcall* _v, struct WXDLloader* _loader, WXDLvalue* _ret);

// 在后面追加参数
WXDIALOGUE_API WXDLvalue* wxdl_call_add_null(WXDLcall* _c);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_bool(WXDLcall* _c, WXDLbool _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_int(WXDLcall* _c, WXDLint _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_u64(WXDLcall* _c, WXDLu64 _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_float(WXDLcall* _c, WXDLfloat _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_str(WXDLcall* _c, const WXDLchar* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_str_ref(WXDLcall* _c, WXDLstring* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_hash(WXDLcall* _c, WXDLhash* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_arr(WXDLcall* _c, WXDLarr* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_call(WXDLcall* _c, WXDLcall* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_add_call_ref(WXDLcall* _c, WXDLcall* _v);

// 设置置顶位置参数
WXDIALOGUE_API WXDLvalue* wxdl_call_set_null(WXDLcall* _c, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_bool(WXDLcall* _c, WXDLu64 _index, WXDLbool _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_int(WXDLcall* _c, WXDLu64 _index, WXDLint _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_u64(WXDLcall* _c, WXDLu64 _index, WXDLu64 _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_float(WXDLcall* _c, WXDLu64 _index, WXDLfloat _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_str(WXDLcall* _c, WXDLu64 _index, const WXDLchar* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_str_ref(WXDLcall* _c, WXDLu64 _index, WXDLstring* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_hash(WXDLcall* _c, WXDLu64 _index, WXDLhash* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_arr(WXDLcall* _c, WXDLu64 _index, WXDLarr* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_call(WXDLcall* _c, WXDLu64 _index, WXDLcall* _v);

WXDIALOGUE_API WXDLvalue* wxdl_call_set_call_ref(WXDLcall* _c, WXDLu64 _index, WXDLcall* _v);
// 获取调用的参数数量
WXDIALOGUE_API WXDLu64 wxdl_call_param_size(WXDLcall* _c);

WXDIALOGUE_API WXDLfunction wxdl_call_param_func(WXDLcall* _c);

WXDIALOGUE_API WXDLvalue* wxdl_param_at(WXDLcall* _c, WXDLu64 _index);

// 获取参数(运行call，返回返回值，将返回值设置给_v)
WXDIALOGUE_API WXDLvalue* wxdl_param_running(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API WXDLbool wxdl_param_bool(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API WXDLint wxdl_param_int(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API WXDLfloat wxdl_param_float(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API const WXDLchar* wxdl_param_str(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API WXDLstring* wxdl_param_str_ref(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API WXDLhash* wxdl_param_hash(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API WXDLarr* wxdl_param_arr(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API WXDLcall* wxdl_param_call(struct WXDLloader* _loader, WXDLvalue* _v);

WXDIALOGUE_API WXDLvalue* wxdl_param_value(struct WXDLloader* _loader, WXDLvalue* _v);

// 确定参数类型
WXDIALOGUE_API WXDLbool wxdl_param_check(const WXDLvalue* _argv, WXDLu32 _argc, const WXDLflag* flags, WXDLu32 flag_size);


#endif
