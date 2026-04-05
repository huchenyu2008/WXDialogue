#ifndef _WXDL_PARSE_H_
#define _WXDL_PARSE_H_
#include "hash.h"
#include "state.h"

typedef struct  WXDLloader WXDLloader;

// 解析文本
// _text_size 为0时会自动计算长度
WXDIALOGUE_API WXDLtext* wxdl_parse(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, WXDLchar* _log_buff, WXDLu64 _log_max_size);

#endif