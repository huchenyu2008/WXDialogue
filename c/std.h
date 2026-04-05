#ifndef _WXDIALOGUE_STD_H_
#define _WXDIALOGUE_STD_H_
#include "./define.h"

WXDIALOGUE_API WXDLptr wxdl_malloc(WXDLint _malloc_size);

WXDIALOGUE_API void wxdl_value_copy(WXDLvalue* _v1, WXDLvalue* _v2);

WXDIALOGUE_API void wxdl_free(WXDLptr _p);

WXDIALOGUE_API void wxdl_free_value(WXDLvalue* _pv);

WXDIALOGUE_API void wxdl_set(WXDLptr _buff, WXDLint32 _v, WXDLu64 _size);

WXDIALOGUE_API void wxdl_copy(WXDLptr _buff, const WXDLptr _copy, WXDLu64 _size);

WXDIALOGUE_API WXDLchar* wxdl_new_str(const WXDLchar* _str);

WXDIALOGUE_API WXDLu64 wxdl_str_len(const WXDLchar* _str);

WXDIALOGUE_API WXDLbool wxdl_str_cmp(const WXDLchar* _s1, const WXDLchar* _s2);

WXDIALOGUE_API WXDLu64 wxdl_str_hashcode(const WXDLchar* _str);

// 判断一个类型是否可以转为另一个类型
WXDIALOGUE_API WXDLbool wxdl_is_type_convert(WXDLflag _t1, WXDLflag _t2);

#endif