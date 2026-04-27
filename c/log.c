
#include "./log.h"
#include "define.h"
#include "type_define.h"
#include "arr.h"
#include "parse.h"
#include "state.h"
#include <stdio.h>


WXDLint _wxdl_get_enter_pos(const WXDLchar* code, WXDLint line, WXDLint line_st)
{
    WXDLint i;
    for (i = 0; code[line_st + i] != '\0'; i++)
    {
        if (code[line_st + i] == '\n') break;
    }

    return i;
}

extern WXDLu64 _wxdl_get_utf8_len(const WXDLchar* _text);

// 限制输出
void _wxdl_limit_send(const WXDLchar* text, WXDLu64 line_st, int* off, int d, int x)
{
    const int max_w = 160;
    if (d > max_w)
    {
        if (d - x >= d - max_w)
        {
            *off = d - max_w;
        }
        else
        {
            if (x - max_w / 2 < 0)
                *off = 0;
            else
                *off = x - max_w / 2;
        }
    }

    // 放在utf8乱码
    WXDLu64 i = 0;
    for (;i < d; i += _wxdl_get_utf8_len(text + line_st + i))
    {
        if ((i - line_st) >= *off) break;
    }
    *off = (int)i;
}

void wxdl_log_error(WXDLloader* loader, const WXDLchar* where, const WXDLchar* text)
{
    WXDLstate* state = wxdl_loader_state(loader);
    WXDLlogbuff* lb = loader->logbuff;
    if (lb == NULL || lb->logbuff == NULL) return;


    WXDLcall* c = (WXDLcall*)wxdl_loader_userdata(loader);

    int off = 0;
    int d = (int)_wxdl_get_enter_pos(loader->text, loader->line, loader->line_start);
    int x = (int)(loader->ptr - loader->line_start);

    _wxdl_limit_send(loader->text, loader->line_start, &off, d, x);

    if (c == NULL)
        lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "WXDL Error file %s, pos (%lld, %d):\n", where, loader->line, x);
    else
        lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "WXDL Error file %s, pos (%lld, %d), call '%s' :\n", where, loader->line, x, c->name->str);
    lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "|    %.*s\n", d - off, loader->text + loader->line_start + (WXDLu64)off);
    lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "|%*s~~~^\n", x - off, "");
    lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "|error info : %s\n", text);
    lb->logbuff[lb->bufflen] = '\0';
    return;
}

void wxdl_log_call_error(WXDLstate* state, WXDLcall* call, const WXDLchar* text, WXDLu32 pid)
{
    WXDLlogbuff* lb = wxdl_state_logbuff(state);
    if (lb == NULL) return;

    WXDLcall* c = call;

    if (c != NULL && c->where != NULL)
        lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "WXDL Call Error file %s, pos (%d, %d), call '%s' :\n", c->where->str, c->line, c->xpos, c->name->str);
    else
        lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "WXDL Call Error file unkown, pos (%d, %d), call '%s' :\n", c->line, c->xpos, c->name->str);
    WXDLthread_resoucre* res = wxdl_state_pid(state, pid);

    if (res != NULL)
    {
        int off = 0;
        int d = (int)_wxdl_get_enter_pos(res->text, c->line, c->xpos_st);
        int x = (int)(c->xpos);

        _wxdl_limit_send(res->text, c->xpos_st, &off, d, x);
        lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "|    %.*s\n", d - off, res->text + c->xpos_st + (WXDLu64)off);
        lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "|%*s~~~^\n", x - off, "");
    }
    else
    {
        lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "|    Detailed error location is not exposed in call mode.\n");
        lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "|    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    }
    lb->bufflen += (WXDLu64)snprintf(lb->logbuff + lb->bufflen, lb->buffsize - lb->bufflen, "|error info : %s\n", text);
}

const char* wxdl_get_type_str(int flag)
{
    switch (flag)
    {
    case WXDL_TYPE_NULL:
        return "null";
    case WXDL_TYPE_ARR:
        return "array";
    case WXDL_TYPE_BOOL:
        return "bool";
    case WXDL_TYPE_DIC:
        return "table";
    case WXDL_TYPE_FLOAT:
        return "float";
    case WXDL_TYPE_INT:
        return "int";
    case WXDL_TYPE_PTR:
        return "ptr";
    case WXDL_TYPE_STR:
        return "string";
    default:
        return "unknown";
    }
}
