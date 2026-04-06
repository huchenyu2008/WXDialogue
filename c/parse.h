#ifndef _WXDL_PARSE_H_
#define _WXDL_PARSE_H_
#include "hash.h"
#include "state.h"

typedef struct  WXDLloader WXDLloader;

// 解析文本
// _text_size 为0时会自动计算长度g
WXDIALOGUE_API WXDLtext* wxdl_parse(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, WXDLchar* _log_buff, WXDLu64 _lo_max_size, const WXDLchar* _where);

// 解析块(用于另类的格式文件)
WXDIALOGUE_API WXDLhash* wxdl_parse_block(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, WXDLchar* _log_buff, WXDLu64 _lo_max_size, const WXDLchar* _where);

#endif