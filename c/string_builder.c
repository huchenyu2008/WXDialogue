#ifndef _WXDIALOGUE_STRING_BUILDER_C_
#define _WXDIALOGUE_STRING_BUILDER_C_
#include "./string_builder.h"
#include "define.h"
#include "hash.h"
#include "iterator.h"
#include "std.h"
#include <stdio.h>

typedef struct WXDLstring_builder
{
    WXDLhash* hash;
    WXDLu32 recount;
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
    bs->recount = 1;
    return bs;
}

WXDLstring_builder* wxdl_builder_ref(WXDLstring_builder* _builder)
{
    if (_builder != NULL)
    {
        _builder->recount += 1;
    }
    return _builder;
}

void wxdl_free_builder(WXDLstring_builder* _builder)
{
    if (_builder == NULL) return;

    _builder->recount -= 1;
    if (_builder->recount > 0) return;

    // 将string里的builder都设为null
    WXDLiterator* ite = wxdl_hash_ite(_builder->hash);
    do {
        WXDLstring* k = wxdl_hash_ite_key(ite);
        if (k != NULL)
        {
            k->builder = NULL;
        }
    }while (wxdl_iterator_next(ite));

    wxdl_free_hash(_builder->hash);
    wxdl_free(_builder);
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
        //printf("new %s\n", _str);
    }
    return str;
}

WXDLstring* wxdl_try_gen_build_string(WXDLstring_builder* _sb, WXDLchar* _str)
{
    return  wxdl_try_gen_build_string_with_hashcode(_sb, _str, wxdl_str_hashcode(_str));
}

WXDLstring* wxdl_try_gen_build_string_with_hashcode(WXDLstring_builder* _sb, WXDLchar* _str, WXDLu64 hashcode)
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
        n = _wxdl_builder_hash_add(_sb->hash, (WXDLchar*)_str, hashcode, WXDL_TRUE);
        str = WXDL_NODE_STR_REF(n);
        //printf("new %s\n", _str);
    }
    return str;
}

WXDLbool wxdl_string_ref_cmp(const WXDLstring* _str, const WXDLstring* _str2)
{
    if (_str == NULL || _str2 == NULL) return WXDL_FALSE;

    if (_str->builder == _str2->builder)
    {
        return (_str == _str2) ? 0 : 1;
    }
    else
    {
        return (_str->hashcode == _str2->hashcode && wxdl_str_cmp(_str->str, _str2->str) == 0) ? 0 : 1;;
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
            //printf("free %s\n", _str->str);
            if (_str->builder != NULL)
            {
                WXDLhash_node* n = wxdl_hash_sr_remove(_str->builder->hash, _str);
                wxdl_free(n);
            }
            wxdl_free(_str->str);
            wxdl_free(_str);
        }
    }
}

#endif
