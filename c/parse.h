#ifndef _WXDL_PARSE_H_
#define _WXDL_PARSE_H_
#include "define.h"
#include "hash.h"
#include "state.h"


// parse=========================================================================================================================================================================

// 解析文本
// _text_size 为0时会自动计算长度g
// _pid 是通过wxdl_state_new_pid获取, 保证多线程安全操作的
// 假如你的文本没那么复杂, 比如没用寄存器什么的, 那将_pid设置为 WXDL_INVAILD_PID
// 用寄存器的话就加, 因为寄存器在pid的资源里
WXDIALOGUE_API WXDLtext* wxdl_parse(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, const WXDLchar* _where, WXDLu32 _pid, WXDLlogbuff* _logbuff);


// block===========================================================================================================

// 解析块(用于另类的格式文件)
// func_running 是是否里面运行内嵌的函数
// _pid 是通过wxdl_state_new_pid获取, 保证多线程安全操作的
// 假如你的文本没那么复杂, 比如没用寄存器什么的, 那将_pid设置为 WXDL_INVAILD_PID
// 用寄存器的话就加, 因为寄存器在pid的资源里
WXDIALOGUE_API WXDLblock* wxdl_parse_block(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, WXDLbool _func_running, const WXDLchar* _where, WXDLu32 _pid, WXDLlogbuff* _logbuff);

// 运行块(复制块中的表, 并将运行数据写入)
// 这个在wxdl_parse_block的func_running为WXDL_FALSE才会有用
// _pid 是通过wxdl_state_new_pid获取, 保证多线程安全操作的
// 假如你的文本没那么复杂, 比如没用寄存器什么的, 那将_pid设置为 WXDL_INVAILD_PID
// 用寄存器的话就加, 因为寄存器在pid的资源里
WXDIALOGUE_API WXDLhash* wxdl_block_running(WXDLstate* _state, WXDLblock* _block, WXDLu32 _pid);

// 创建新的块, 用户用不到
WXDIALOGUE_API WXDLblock* wxdl_new_block(WXDLhash* _refhash, WXDLstring_builder* _builder);

// 设置 其中的数据表
WXDIALOGUE_API WXDLhash* wxdl_block_set_data(WXDLblock* _block, WXDLhash* _refhash);

// 获取其中的数据表
WXDIALOGUE_API WXDLhash* wxdl_block_data(WXDLblock* _block);

// 释放块
WXDIALOGUE_API void wxdl_free_block(WXDLblock* _block);
// parse end=========================================================================================================================================================================



// save=========================================================================================================================================================================

typedef struct WXDLsave_opinion
{
    WXDLbool open_format;
    WXDLu32 expand_arr_size;
    WXDLu32 expand_param_size;
}WXDLsave_opinion;

// 创建序列化器
WXDIALOGUE_API WXDLsaver* wxdl_new_saver(WXDLblock* _block, const WXDLsave_opinion* opinion);

// 输出到指定缓存组中
WXDIALOGUE_API WXDLu64 wxdl_saver_output(WXDLsaver* _saver, WXDLbuff_set* _buff);

// 销毁序列化器
WXDIALOGUE_API void wxdl_free_saver(WXDLsaver* _saver);

// save end=========================================================================================================================================================================


// 通过路径获得表中的节点
// len可以为0，自动判断
WXDIALOGUE_API WXDLhash_node* wxdl_hash_path(WXDLhash* _hash, const WXDLchar* _path, WXDLu64 _len);

// loader============================================================================================================

// 获全局取状态
WXDIALOGUE_API WXDLstate* wxdl_loader_state(struct WXDLloader* _loader);

// 获取用户数据
WXDIALOGUE_API WXDLptr wxdl_loader_userdata(struct WXDLloader* _loader);

// 设置用户数据(推荐别碰)
WXDIALOGUE_API WXDLptr wxdl_set_loader_userdata(struct WXDLloader* _loader, WXDLptr _ptr);

// 获取字符串构建器
WXDIALOGUE_API WXDLstring_builder* wxdl_set_loader_builder(struct WXDLloader* _loader);
#endif
