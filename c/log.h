#ifndef _WXDIALOGUE_LOG_H_
#define _WXDIALOGUE_LOG_H_
#include "./define.h"
#include "parse.h"

typedef struct WXDLlog
{
    WXDLchar* buff;
    WXDLint buff_size;
}WXDLlog;

#ifdef _MSC_VER
#define WXDL_LOG_WRITE(loader, text, ...) \
    {\
        static char _wxdl_log_info_buff[128];\
        sprintf_s(_wxdl_log_info_buff, 1024, text, __VA_ARGS__);\
        wxdl_log_error(loader, _wxdl_log_info_buff);\
    }
#else
#define WXDL_LOG_WRITE(loader, text, ...) \
    {\
        static char _wxdl_log_info_buff[128];\
        sprintf_s(_wxdl_log_info_buff, 1024, text, ##__VA_ARGS__);\
        wxdl_log_error(loader, _wxdl_log_info_buff);\
    }
#endif
// 生成错误信息
// code 数据文本
// line 错误行数
// line_st 当前行开始位置
// error_pos 错误的位置
// text 信息
WXDIALOGUE_API void wxdl_log_error(WXDLloader* loader, const WXDLchar* text);

WXDIALOGUE_API const char* wxdl_get_type_str(int flag);

#endif
