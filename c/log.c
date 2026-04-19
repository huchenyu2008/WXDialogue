
#include "./log.h"
#include "define.h"
#include "arr.h"
#include <stdio.h>


typedef struct WXDLloader
{
	// 使用的状态机
	WXDLstate* state;

	// 解析文本
	WXDLchar* text;
	WXDLu64 text_size;

	// 当前解析位置
	WXDLu64 ptr;

    // 当前行数
    WXDLu64 line;

    // 当前行数开始字节
    WXDLu64 line_start;

	// 日志输出缓存
	WXDLchar* log_buff;
	WXDLu64 log_buff_size;

    // 当前日志长度
    WXDLu64 log_len;

    // 当前使用标签
    WXDLhash* psign;

    // 使用的本地签名表
    WXDLarr* use_local_signs;

    // 文件名称
    const WXDLchar* where;

    // 是否直接运行
    WXDLbool is_running_call;

    // 当前的call层数
    WXDLu64 call_layer_count;

    // 用户数据
    WXDLptr userdata;

    WXDLstring_builder* builder;
}WXDLloader;

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
    if (loader->log_buff == NULL) return;


    WXDLcall* c = (WXDLcall*)wxdl_loader_userdata(loader);

    int off = 0;
    int d = (int)_wxdl_get_enter_pos(loader->text, loader->line, loader->line_start);
    int x = (int)(loader->ptr - loader->line_start);

    _wxdl_limit_send(loader->text, loader->line_start, &off, d, x);

    if (c == NULL)
        loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "WXDL Error file %s, pos (%lld, %d):\n", where, loader->line, x);
    else
        loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "WXDL Error file %s, pos (%lld, %d), call '%s' :\n", where, loader->line, x, c->name->str);
    loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|    %.*s\n", d - off, loader->text + loader->line_start + (WXDLu64)off);
    loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|%*s~~~^\n", x - off, "");
    loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|error info : %s\n", text);
    loader->log_buff[loader->log_len] = '\0';
    return;
}

void wxdl_log_call_error(WXDLloader* loader, const WXDLchar* text)
{
    if (loader->log_buff == NULL) return;

    WXDLcall* c = (WXDLcall*)wxdl_loader_userdata(loader);

    if (loader->is_running_call && c == NULL)
    {
        wxdl_log_error(loader, loader->where, text);
        return;
    }

    if (c != NULL && c->where != NULL)
        loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "WXDL Call Error file %s, pos (%d, %d), call '%s' :\n", c->where->str, c->line, c->xpos, c->name->str);
    else
        loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "WXDL Call Error file unkown, pos (%d, %d), call '%s' :\n", c->line, c->xpos, c->name->str);
    loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|    Detailed error location is not exposed in call mode.\n");
    loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    loader->log_len += (WXDLu64)snprintf(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|error info : %s\n", text);
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
