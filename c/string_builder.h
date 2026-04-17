#ifndef _WXDIALOGUE_STRING_BUILDER_H_
#define _WXDIALOGUE_STRING_BUILDER_H_
#include "define.h"
#include "hash.h"

#define _WXDL_INIT_STRING(_builder, _str, _cstr) \
    str = (WXDLstring*)wxdl_malloc(sizeof(WXDLstring));\
        _str->str = wxdl_new_str(_cstr);\
        _str->hashcode = wxdl_str_hashcode(_cstr);\
        _str->builder = _builder;\
        _str->len = (WXDLu32)wxdl_str_len(_cstr);\
        _str->refcount = 1;\

#define _WXDL_INIT_STRING_WITH_CODE(_builder, _str, _cstr, _hashcode) \
    str = (WXDLstring*)wxdl_malloc(sizeof(WXDLstring));\
        _str->str = wxdl_new_str(_cstr);\
        _str->hashcode = _hashcode;\
        _str->builder = _builder;\
        _str->len = (WXDLu32)wxdl_str_len(_cstr);\
        _str->refcount = 1;\

// 传进的_cstr不会拷贝
#define _WXDL_INIT_STRING_WITH_REF_AND_CODE(_builder, _str, _cstr, _hashcode) \
    str = (WXDLstring*)wxdl_malloc(sizeof(WXDLstring));\
        _str->str = _cstr;\
        _str->hashcode = _hashcode;\
        _str->builder = _builder;\
        _str->len = (WXDLu32)wxdl_str_len(_cstr);\
        _str->refcount = 1;\

// 获取全局的字符串构建器
WXDIALOGUE_API WXDLstring_builder* wxdl_get_global_builder();

// 创建一个字符串构建器
WXDIALOGUE_API WXDLstring_builder* wxdl_new_builder();

// 获得一个字符串引用, 没有则创建
WXDIALOGUE_API WXDLstring* wxdl_build_string(WXDLstring_builder* _sb, const WXDLchar* _str);

// 用一个字符串对象创建字符串引用, 已经有了就删掉传进来的字符串
WXDIALOGUE_API WXDLstring* wxdl_try_gen_build_string(WXDLstring_builder* _sb, WXDLchar* _str);

// 添加字符串引用计数
WXDIALOGUE_API WXDLstring* wxdl_string_ref(WXDLstring* _str);

// 字符串引用间的比较，如果为同个builder效率则很快
WXDIALOGUE_API WXDLbool wxdl_string_ref_cmp(const WXDLstring* _str, const WXDLstring* _str2);
// 字符串引用与C字符串间的比较，效率和正常的比较一样快
WXDIALOGUE_API WXDLbool wxdl_string_ref_cmp_cstr(const WXDLstring* _str, const WXDLchar* _str2);

// 释放字符串引用
WXDIALOGUE_API void wxdl_free_string(WXDLstring* _str);



#endif
