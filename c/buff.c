
#include "buff.h"
#include "arr.h"
#include "define.h"
#include "std.h"

// 扩容因子
#define _WXDL_BUFF_EXT_SIZE_FATOR 1.5

typedef struct WXDLbuff_set
{
    WXDLarr* buffs;
    // 写入指针
    WXDLu64 write_ptr;
}WXDLbuff_set;

typedef struct WXDLbuff
{
    WXDLchar* buff;
    // 缓存大小
    WXDLu32 buffsize;
    // 当前可写入大小
    WXDLu32 can_write_size;
}WXDLbuff;

// buff_set=========================================================================

void _wxdl_buff_ext(WXDLbuff_set* _buff)
{
    if (_buff == NULL) return;

    WXDLu64 st = wxdl_arr_size(_buff->buffs);
    WXDLu64 ed = 8;
    if (st != 0) ed = (WXDLu64)(st * _WXDL_BUFF_EXT_SIZE_FATOR);
    for (WXDLu64 i = st; i < ed; i++)
    {
        WXDLbuff* b = wxdl_new_buff(WXDL_BUFF_SIZE);
        wxdl_arr_add_ptr(_buff->buffs, b);
    }
}

WXDLbuff_set* wxdl_new_buff_set()
{
    WXDLbuff_set* b = (WXDLbuff_set*)wxdl_malloc(sizeof(WXDLbuff_set));
    b->buffs = wxdl_new_arr(8, NULL);
    b->write_ptr = 0;
    _wxdl_buff_ext(b);
    return b;
}

void wxdl_free_buff_set(WXDLbuff_set* _buff)
{
    if (_buff == NULL) return;

    for (WXDLu64 i = 0; i < wxdl_arr_size(_buff->buffs); i++)
    {
        WXDLbuff* b = (WXDLbuff*)WXDL_V_PTR(*wxdl_arr_at(_buff->buffs, i));
        wxdl_free_buff(b);
    }
    wxdl_free_arr(_buff->buffs);
    wxdl_free(_buff);
}

WXDLu64 wxdl_buff_set_size(WXDLbuff_set* _buff)
{
    if (_buff == NULL) return 0;
    return wxdl_arr_size(_buff->buffs);
}

WXDLbuff* wxdl_buff_set_at(WXDLbuff_set* _buff, WXDLu64 _index)
{
    if (_buff == NULL || wxdl_arr_size(_buff->buffs) <= _index) return NULL;
    return (WXDLbuff*)WXDL_V_PTR(*wxdl_arr_at(_buff->buffs, _index));
}

WXDLbool wxdl_buff_set_next(WXDLbuff_set* _buff)
{
    if (_buff == NULL) return WXDL_FALSE;

    _buff->write_ptr += 1;
    if (wxdl_arr_size(_buff->buffs) <= _buff->write_ptr)
    {
        _wxdl_buff_ext(_buff);
        return WXDL_TRUE;
    }
    return WXDL_FALSE;
}

WXDLbuff* wxdl_buff_set_write_chr(WXDLbuff_set* _buff, WXDLchar _c)
{
    if (_buff == NULL) return NULL;
    WXDLbuff* b = (WXDLbuff*)WXDL_V_PTR(*wxdl_arr_at(_buff->buffs, _buff->write_ptr));
    if (wxdl_buff_check_size(b, 1) == 0)
    {
        wxdl_buff_set_next(_buff);
        b = (WXDLbuff*)WXDL_V_PTR(*wxdl_arr_at(_buff->buffs, _buff->write_ptr));
    }


    wxdl_buff_unsafe_write(b, &_c, 1);
    return b;
}

WXDLbuff* wxdl_buff_set_write(WXDLbuff_set* _buff, const WXDLchar* _text, WXDLu64 _size)
{
    if (_buff == NULL) return NULL;

    WXDLu64 off = 0;
    WXDLu32 wc = 0;
    WXDLbuff* b = (WXDLbuff*)WXDL_V_PTR(*wxdl_arr_at(_buff->buffs, _buff->write_ptr));
    while (_size != off)
    {
        wc = wxdl_buff_check_size(b, (WXDLu32)_size);
        if (wc == 0)
        {
            wxdl_buff_set_next(_buff);
            b = (WXDLbuff*)WXDL_V_PTR(*wxdl_arr_at(_buff->buffs, _buff->write_ptr));
        }
        else
        {
            wxdl_buff_unsafe_write(b, _text + off, wc);
            off += (WXDLu64)wc;
        }
    }

    return b;
}

// buff=========================================================================

WXDLbuff* wxdl_new_buff(WXDLu32 _buffsize)
{
    WXDLbuff* b = (WXDLbuff*)wxdl_malloc(sizeof(WXDLbuff));
    b->buff = (WXDLchar*)wxdl_malloc(_buffsize);
    wxdl_set(b->buff, 0, _buffsize);
    b->can_write_size = _buffsize;
    b->buffsize = _buffsize;
    return b;
}

void wxdl_free_buff(WXDLbuff* _buff)
{
    if (_buff == NULL) return;

    wxdl_free(_buff->buff);
    wxdl_free(_buff);
}

WXDLchar* wxdl_buff_get(WXDLbuff* _buff)
{
    if (_buff == NULL) return NULL;
    return  _buff->buff;
}

WXDLu32 wxdl_buff_check_size(WXDLbuff* _buff, WXDLu32 _write_size)
{
    if (_buff == NULL) return 0;

    if (_buff->can_write_size < _write_size) return _buff->can_write_size;
    else return _write_size;
}

void wxdl_buff_unsafe_write(WXDLbuff* _buff, const WXDLchar* _text, WXDLu32 _size)
{
    if (_buff == NULL) return;
    wxdl_copy(_buff->buff + (_buff->buffsize - _buff->can_write_size), (const WXDLptr)_text, _size);
    _buff->can_write_size -= _size;
}
