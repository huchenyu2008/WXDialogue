#ifndef _WXDIALOGUE_LIB_H_
#define _WXDIALOGUE_LIB_H_
#include "./define.h"
#include "state.h"

WXDLerror _wxdl_lib_getvar(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLu32 pid);

// 初始化标准库
WXDIALOGUE_API void wxdl_init_std_lib(WXDLstate* _state);


#endif
