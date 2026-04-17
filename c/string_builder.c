#ifndef _WXDIALOGUE_STRING_BUILDER_C_
#define _WXDIALOGUE_STRING_BUILDER_C_
#include "./string_builder.h"
#include "define.h"
#include "hash.h"
#include "std.h"
#include <stdio.h>

typedef struct WXDLstring_builder
{
    WXDLhash* hash;
}WXDLstring_builder;

WXDLstring_builder* _wxdl_global_builder = NULL;

WXDLstring_builder* wxdl_get_global_builder()
{
    if (_wxdl_global_builder == NULL)
    {
        _wxdl_global_builder = wxdl_new_builder();
    }

    return _wxdl_global_builder;
}

WXDLstring_builder* wxdl_new_builder()
{
    WXDLstring_builder* bs = (WXDLstring_builder*)wxdl_malloc(sizeof(WXDLstring_builder));
    bs->hash = wxdl_new_hash(32, bs);
    return bs;
}

WXDLstring* wxdl_build_string(WXDLstring_builder* _sb, const WXDLchar* _str)
{
    WXDLhash_node* n = wxdl_hash_find(_sb->hash, _str);
    WXDLstring* str = NULL;
    if (n != NULL)
    {
        str = WXDL_NODE_STR_REF(n);
        str->refcount += 1;
    }
    else
    {
        n = _wxdl_builder_hash_add(_sb->hash, (WXDLchar*)_str, wxdl_str_hashcode(_str), WXDL_FALSE);
        str = WXDL_NODE_STR_REF(n);
        //printf("new %s %lld\n", _str, str);
    }
    return str;
}

WXDLstring* wxdl_try_gen_build_string(WXDLstring_builder* _sb, WXDLchar* _str)
{
    WXDLhash_node* n = wxdl_hash_find(_sb->hash, _str);
    WXDLstring* str = NULL;
    if (n != NULL)
    {
        wxdl_free(_str);
        str = WXDL_NODE_STR_REF(n);
        str->refcount += 1;
    }
    else
    {
        n = _wxdl_builder_hash_add(_sb->hash, (WXDLchar*)_str, wxdl_str_hashcode(_str), WXDL_TRUE);
        str = WXDL_NODE_STR_REF(n);
        //printf("new %s %lld\n", _str, str);
    }
    return str;
}

WXDLbool wxdl_string_ref_cmp(const WXDLstring* _str, const WXDLstring* _str2)
{
    if (_str == NULL || _str2 == NULL) return WXDL_FALSE;

    if (_str->builder == _str2->builder)
    {
        return (_str == _str2);
    }
    else
    {
        return (_str->hashcode == _str2->hashcode && wxdl_str_cmp(_str->str, _str2->str));
    }
}

WXDLbool wxdl_string_ref_cmp_cstr(const WXDLstring* _str, const WXDLchar* _str2)
{
    if (_str == NULL) return WXDL_FALSE;
    return wxdl_str_cmp(_str->str, _str2);
}

WXDLstring* wxdl_string_ref(WXDLstring* _str)
{
    if (_str != NULL)
    {
        _str->refcount += 1;
    }
    return _str;
}

void wxdl_free_string(WXDLstring* _str)
{
    if (_str != NULL)
    {
        _str->refcount -= 1;
        //printf("count down %s %lld\n", _str->str, _str->refcount);
        if (_str->refcount < 1)
        {
            wxdl_hash_sr_remove(_str->builder->hash, _str);
            wxdl_free(_str->str);
            wxdl_free(_str);
        }
    }
}

#endif
