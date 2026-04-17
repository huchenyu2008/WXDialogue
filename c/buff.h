#ifndef _WXDIALOGUE_BUFF_H_
#define _WXDIALOGUE_BUFF_H_
#include "define.h"

// 创建新的缓存组
WXDIALOGUE_API WXDLbuff_set* wxdl_new_buff_set();

// 销毁缓存组
WXDIALOGUE_API void wxdl_free_buff_set(WXDLbuff_set* _buff);

// 获取有多少个缓存
WXDIALOGUE_API WXDLu64 wxdl_buff_set_size(WXDLbuff_set* _buff);

// 获取指定缓存
WXDIALOGUE_API WXDLbuff* wxdl_buff_set_at(WXDLbuff_set* _buff, WXDLu64 _index);

// 缓存组的写入指针指向下一个缓存
// 返回是否进行扩容
WXDIALOGUE_API WXDLbool wxdl_buff_set_next(WXDLbuff_set* _buff);

// 往缓存中写一个字
// 返回最后写入的缓存
WXDIALOGUE_API WXDLbuff* wxdl_buff_set_write_chr(WXDLbuff_set* _buff, WXDLchar _c);

// 往缓存中写多个字
// 返回写入的缓存
WXDIALOGUE_API WXDLbuff* wxdl_buff_set_write(WXDLbuff_set* _buff, const WXDLchar* _c, WXDLu64 _size);



// 创建新的缓存
WXDIALOGUE_API WXDLbuff* wxdl_new_buff(WXDLu32 _buffsize);

// 销毁缓存
WXDIALOGUE_API void wxdl_free_buff(WXDLbuff* _buff);

// 获取缓存
WXDIALOGUE_API WXDLchar* wxdl_buff_get(WXDLbuff* _buff);

// 判断当前缓存是否可写指定字节
WXDIALOGUE_API WXDLu32 wxdl_buff_check_size(WXDLbuff* _buff, WXDLu32 _write_size);

// 写入指定字节数据(不会检查!!!)
WXDIALOGUE_API void wxdl_buff_unsafe_write(WXDLbuff* _buff, const WXDLchar* _text, WXDLu32 _size);

#endif
