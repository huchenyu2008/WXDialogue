#ifndef _WXDIALOGUE_LOG_C_
#define _WXDIALOGUE_LOG_C_
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
    int off = 0;
    int d = (int)_wxdl_get_enter_pos(loader->text, loader->line, loader->line_start);
    int x = (int)(loader->ptr - loader->line_start);
    _wxdl_limit_send(loader->text, loader->line_start, &off, d, x);
    loader->log_len += (WXDLu64)sprintf_s(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "WXDL Error file %s, pos (%lld, %lld):\n", where, loader->line, loader->ptr - loader->line_start);
    loader->log_len += (WXDLu64)sprintf_s(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|    %.*s\n", d - off, loader->text + loader->line_start + (WXDLu64)off);
    loader->log_len += (WXDLu64)sprintf_s(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|%*s~~~^\n", x - off, "");
    loader->log_len += (WXDLu64)sprintf_s(loader->log_buff + loader->log_len, loader->log_buff_size - loader->log_len, "|error info : %s\n", text);
    loader->log_buff[loader->log_len] = '\0';
    return;
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

#endif
