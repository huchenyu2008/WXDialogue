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
#define WXDL_ERR_LOG(state, call, pres, text, ...) \
    {\
        static char _wxdl_log_info_buff[1024];\
        snprintf(_wxdl_log_info_buff, 1024, text, __VA_ARGS__);\
        wxdl_log_call_error(state, call, _wxdl_log_info_buff, pres);\
    }
#define WXDL_LOG_WRITE(loader, where, text, ...) \
    {\
        static char _wxdl_log_info_buff[1024];\
        snprintf(_wxdl_log_info_buff, 1024, text, __VA_ARGS__);\
        wxdl_log_error(loader, where, _wxdl_log_info_buff);\
    }
#else
#define WXDL_ERR_LOG(state, call, pres, text, ...) \
    {\
        static char _wxdl_log_info_buff[1024];\
        snprintf(_wxdl_log_info_buff, 1024, text, ##__VA_ARGS__);\
        wxdl_log_call_error(state, call, _wxdl_log_info_buff, pres);\
    }

#define WXDL_LOG_WRITE(loader, where, text, ...) \
    {\
        static char _wxdl_log_info_buff[1024];\
        snprintf(_wxdl_log_info_buff, 1024, text, ##__VA_ARGS__);\
        wxdl_log_error(loader, where, _wxdl_log_info_buff);\
    }
#endif
// 生成错误信息
WXDIALOGUE_API void wxdl_log_error(WXDLloader* loader, const WXDLchar* where, const WXDLchar* text);

// Call时用的, 生成错误信息
// 注意！loader->userdata必须为WXDLCall类型，不然就为空
// 函数wxdl_hash_copy_running和wxdl_arr_copy_running的实现
// 就是利用这个让调用的错误输出用错误的文件位置
WXDIALOGUE_API void wxdl_log_call_error(WXDLstate* state, WXDLcall* call, const WXDLchar* text, WXDLthread_resoucre* pres);

WXDIALOGUE_API const char* wxdl_get_type_str(int flag);

#endif
