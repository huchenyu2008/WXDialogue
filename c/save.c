
#include "arr.h"
#include "buff.h"
#include "define.h"
#include "hash.h"
#include "iterator.h"
#include "parse.h"
#include "std.h"
#include "string_builder.h"
#include <stdio.h>

typedef struct WXDLsaver
{
    // 使用的块
    WXDLblock* block;

    // 块的数据
    WXDLhash* data;

    // 输出的缓冲
    WXDLbuff_set* buff;

    WXDLsave_opinion opinion;
}WXDLsaver;

WXDLsaver* wxdl_new_saver(WXDLblock* _block, const WXDLsave_opinion* _opinion)
{
    if (_block == NULL) return NULL;
    WXDLsaver* s = (WXDLsaver*)wxdl_malloc(sizeof(WXDLsaver));
    s->block = _block;
    s->data = wxdl_block_data(_block);
    s->buff = NULL;
    if (_opinion != NULL)
    {
        wxdl_copy(&s->opinion, (const WXDLptr)_opinion, sizeof(WXDLsave_opinion));
    }
    else
    {
        s->opinion.open_format = WXDL_TRUE;
        s->opinion.expand_arr_size = 5;
        s->opinion.expand_param_size = 7;
    }

    return s;
}

void wxdl_free_saver(WXDLsaver* _saver)
{
    if (_saver == NULL) return;

    wxdl_free(_saver);
}

// out=====================================================================================
// 转移符号
char* _wxdl_parse_esc_code_char(char c)
{
    switch (c)
    {
    case '\n':
        return "\\n";

    case '\t':
        return "\\t";

    case '\b':
        return "\\b";

    case '\\':
        return "\\\\";

    case '\"':
        return "\\\"";

    case '\'':
        return "\\\'";

    case '\r':
        return "\\r";
    default:
        return "\\";
    }
}

int _wxdl_check_esc_code_char(char c)
{
    switch (c)
    {
    case '\n':
    case '\t':
    case '\b':
    case '\\':
    case '"':
    case '\'':
    case '\r':
        return 1;
    default:
        return 0;
    }
}

WXDLu64 _wxdl_saver_out_null(WXDLsaver* _saver)
{
    wxdl_buff_set_write(_saver->buff, "null", 4);
    return 4;
}

WXDLu64 _wxdl_saver_out_bool(WXDLsaver* _saver, WXDLbool _v)
{
    const WXDLchar p[2][6] = {"false\0", "true\0"};
    WXDLbool b = 0;
    if (_v) b = 1;
    WXDLu64 s = wxdl_str_len(p[b]);
    wxdl_buff_set_write(_saver->buff, p[b], s);
    return s;
}

WXDLu64 _wxdl_saver_out_tab(WXDLsaver* _saver, WXDLu64 _deep)
{
    if (_saver->opinion.open_format == WXDL_FALSE) return 0;
    for (WXDLu64 i = 0; i < _deep; i++)
        wxdl_buff_set_write_chr(_saver->buff, '\t');
    return _deep;
}

WXDLu64 _wxdl_saver_out_enter(WXDLsaver* _saver)
{
    if (_saver->opinion.open_format == WXDL_FALSE) return 0;
    wxdl_buff_set_write_chr(_saver->buff, '\n');
    return 1;
}

WXDLu64 _wxdl_saver_out_int(WXDLsaver* _saver, WXDLint _v)
{
    char buff[256];
    WXDLu64 s = sprintf_s(buff, sizeof(buff), "%lld", _v);
    wxdl_buff_set_write(_saver->buff, buff, s);
    return s;
}

WXDLu64 _wxdl_saver_out_float(WXDLsaver* _saver, WXDLfloat _v)
{
    char buff[256];
    WXDLu64 s = sprintf_s(buff, sizeof(buff), "%lf", _v);
    wxdl_buff_set_write(_saver->buff, buff, s);
    return s;
}

WXDLu64 _wxdl_saver_out_id(WXDLsaver* _saver, WXDLstring* _v)
{
    WXDLu64 s = 0;
    for (WXDLu64 i = 0; i < _v->len; i++)
    {
        WXDLchar c = _v->str[i];
        if (_wxdl_check_esc_code_char(c))
        {
            wxdl_buff_set_write(_saver->buff, _wxdl_parse_esc_code_char(c), 2);
            s += 2;
        }
        else
        {
            wxdl_buff_set_write_chr(_saver->buff, c);
            s += 1;
        }
    }
    return s;
}

WXDLu64 _wxdl_saver_out_str(WXDLsaver* _saver, WXDLstring* _v)
{
    WXDLu64 s = 2;

    wxdl_buff_set_write_chr(_saver->buff, '\"');
    s += _wxdl_saver_out_id(_saver, _v);
    wxdl_buff_set_write_chr(_saver->buff, '\"');

    return s;
}

WXDLu64 _wxdl_saver_out_str_id(WXDLsaver* _saver, WXDLstring* _v)
{
    WXDLu64 s = 2;

    wxdl_buff_set_write_chr(_saver->buff, '\'');
    s += _wxdl_saver_out_id(_saver, _v);
    wxdl_buff_set_write_chr(_saver->buff, '\'');

    return s;
}

WXDLu64 _wxdl_saver_out_hash(WXDLsaver* _saver, WXDLhash* _v, WXDLu64 _deep)
{
    WXDLu64 _wxdl_saver_out_item(WXDLsaver* _saver, WXDLstring* _k, WXDLvalue* _v, WXDLu64 _deep);
    WXDLu64 s = 2;
    WXDLvalue* pv;
    WXDLiterator* ite;

    s += _wxdl_saver_out_tab(_saver, _deep);
    wxdl_buff_set_write_chr(_saver->buff, '{');

    ite = wxdl_hash_ite(_v);
    WXDLu64 i = 0;
    do {
        pv = wxdl_iterator_get(ite);
        if (pv != NULL)
        {
            if (i > 0)
            {
                wxdl_buff_set_write_chr(_saver->buff, ','), s += 1;
                s += _wxdl_saver_out_enter(_saver);
            }
            _wxdl_saver_out_item(_saver, wxdl_hash_ite_key(ite), pv, _deep + 1);
            i++;
        }
    }while (wxdl_iterator_next(ite));
    s += _wxdl_saver_out_enter(_saver);

    s += _wxdl_saver_out_tab(_saver, _deep);
    wxdl_buff_set_write_chr(_saver->buff, '}');
    s += _wxdl_saver_out_enter(_saver);

    return s;
}

WXDLu64 _wxdl_saver_out_arr(WXDLsaver* _saver, WXDLarr* _v, WXDLu64 _deep)
{
    WXDLu64 _wxdl_saver_out_value(WXDLsaver* _saver, WXDLvalue* _v, WXDLu64 _deep);

    WXDLu64 s = 2;
    WXDLvalue* pv;
    WXDLiterator* ite;

    s += _wxdl_saver_out_tab(_saver, _deep);
    wxdl_buff_set_write_chr(_saver->buff, '[');
    s += _wxdl_saver_out_enter(_saver);

    WXDLu64 arrlen = wxdl_arr_size(_v);

    WXDLu64 i = 0;
    ite = wxdl_arr_ite(_v);
    do {
        pv = wxdl_iterator_get(ite);
        if (pv != NULL)
        {
            if (i > 0)
            {
                wxdl_buff_set_write(_saver->buff, ", ", 2), s += 2;
                if (arrlen >= _saver->opinion.expand_arr_size)
                    s += _wxdl_saver_out_enter(_saver);
            }
            _wxdl_saver_out_value(_saver, pv, _deep);
            i++;
        }
    }while (wxdl_iterator_next(ite));

    if (arrlen >= _saver->opinion.expand_arr_size)
        s += _wxdl_saver_out_tab(_saver, _deep);
    wxdl_buff_set_write_chr(_saver->buff, ']');
    s += _wxdl_saver_out_enter(_saver);

    return s;
}

WXDLu64 _wxdl_saver_out_call(WXDLsaver* _saver, WXDLcall* _v, WXDLu64 _deep)
{
    WXDLu64 _wxdl_saver_out_value(WXDLsaver* _saver, WXDLvalue* _v, WXDLu64 _deep);

    WXDLu64 s = 3;
    WXDLvalue* pv;

    wxdl_buff_set_write_chr(_saver->buff, '@');
    s += _wxdl_saver_out_id(_saver, _v->name);
    wxdl_buff_set_write_chr(_saver->buff, '(');

    WXDLu64 arrlen = _v->argc;

    if (arrlen >= _saver->opinion.expand_param_size)
        s += _wxdl_saver_out_enter(_saver);


    WXDLu64 j = 0;
    for (WXDLu64 i = 0; i < arrlen; i++)
    {
        pv = &_v->argv[i];

        if (j > 0)
        {
            wxdl_buff_set_write(_saver->buff, ", ", 2), s += 2;
            if (arrlen >= _saver->opinion.expand_arr_size)
                s += _wxdl_saver_out_enter(_saver);
        }
        _wxdl_saver_out_value(_saver, pv, _deep);
        if (_saver->opinion.open_format)
        {
            if (arrlen >= _saver->opinion.expand_param_size)
                s += _wxdl_saver_out_enter(_saver);
        }
        j++;
    }

    if (arrlen >= _saver->opinion.expand_param_size)
        s += _wxdl_saver_out_tab(_saver, _deep);
    wxdl_buff_set_write_chr(_saver->buff, ')');

    return s;
}
WXDLu64 _wxdl_saver_out_value(WXDLsaver* _saver, WXDLvalue* _v, WXDLu64 _deep)
{
    WXDLu64 s = 0;
    switch (WXDL_V_TYPE(*_v))
    {
    case WXDL_TYPE_NULL:
        s += _wxdl_saver_out_null(_saver);
        break;

    case WXDL_TYPE_BOOL:
        s += _wxdl_saver_out_bool(_saver, WXDL_V_BOOL(*_v));
        break;

    case WXDL_TYPE_INT:
        s += _wxdl_saver_out_int(_saver, WXDL_V_INT(*_v));
        break;

    case WXDL_TYPE_FLOAT:
        s += _wxdl_saver_out_float(_saver, WXDL_V_FLOAT(*_v));
        break;

    case WXDL_TYPE_STR:
        s += _wxdl_saver_out_str(_saver, WXDL_V_STR(*_v));
        break;

    case WXDL_TYPE_DIC:
        s += _wxdl_saver_out_hash(_saver, WXDL_V_DIC(*_v), _deep);
        break;

    case WXDL_TYPE_ARR:
        s += _wxdl_saver_out_arr(_saver, WXDL_V_ARR(*_v), _deep);
        break;

    case WXDL_TYPE_CALL:
    {
        // 对于之前转为call的路径表达式，让他重回原来的样子
        WXDLcall* c = WXDL_V_CALL(*_v);
        if (wxdl_string_ref_cmp_cstr(c->name, WXDL_FUNC_NAME_GET_GLOBAL_VAR) == 0 && c->argc > 0 && WXDL_V_TYPE(c->argv[0]) == WXDL_TYPE_STR)
        {
            s += _wxdl_saver_out_id(_saver, WXDL_V_STR(c->argv[0]));
        }
        else s += _wxdl_saver_out_call(_saver, c, _deep);
        break;
    }
    default:
        s += _wxdl_saver_out_null(_saver);
        break;
    }

    return s;
}

WXDLu64 _wxdl_saver_out_item(WXDLsaver* _saver, WXDLstring* _k, WXDLvalue* _v, WXDLu64 _deep)
{
    WXDLu64 s = 5;

    s += _wxdl_saver_out_enter(_saver);
    s += _wxdl_saver_out_tab(_saver, _deep);
    s += _wxdl_saver_out_str_id(_saver, _k);


    wxdl_buff_set_write(_saver->buff, " : ", 3);

    _wxdl_saver_out_value(_saver, _v, _deep);

    return s;
}

WXDLu64 wxdl_saver_output(WXDLsaver* _saver, WXDLbuff_set* _buff)
{
    WXDLhash* d = wxdl_block_data(_saver->block);
    if (_saver == NULL || _buff == NULL || d == NULL) return 0;

    _saver->buff = _buff;

    return _wxdl_saver_out_hash(_saver, d, 0);
}
