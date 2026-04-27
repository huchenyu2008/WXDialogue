#ifndef _WXDL_STATE_H_
#define _WXDL_STATE_H_
#include "define.h"
#include "hash.h"


#define WXDL_NEW_TEXT_NODE(type, text, data) (WXDLtext_node){.type = type, .text = text, .data = data, .next = NULL};


// state======================================================================================================

// 创建状态机
WXDIALOGUE_API WXDLstate* wxdl_new_state(WXDLstring_builder* builder);

// 释放状态机
// 注意它会把所以你提供的字典(签名表和全局数据表)释放, 避免访问无效内存
WXDIALOGUE_API void wxdl_free_state(WXDLstate* _state);

// 根据签名表生成一个表
WXDIALOGUE_API WXDLhash* wxdl_state_gen_sign_table(WXDLstate* _state);

// 设置log缓存
WXDIALOGUE_API WXDLlogbuff* wxdl_state_set_logbuff(WXDLstate* _state, WXDLchar* _logbuff, WXDLu64 _buffsize);

// 获取log缓存
WXDIALOGUE_API WXDLlogbuff* wxdl_state_logbuff(WXDLstate* _state);

// 清空log缓存
WXDIALOGUE_API WXDLlogbuff* wxdl_state_clear_logbuff(WXDLstate* _state);

// 获取一个pid
WXDIALOGUE_API WXDLu32 wxdl_state_new_pid(WXDLstate* _state);

// 归还一个pid
WXDIALOGUE_API void wxdl_state_free_pid(WXDLstate* _state, WXDLu32 _pid);

// 获取指定pid的资源
// 注意这个不会检查你给pid是否分配...（为了效率）
WXDIALOGUE_API WXDLthread_resoucre* wxdl_state_pid(WXDLstate* _state, WXDLu32 _pid);

// 指定pid是否已被分配
WXDIALOGUE_API WXDLbool wxdl_state_pid_vaild(WXDLstate* _state, WXDLu32 _pid);

// 添加一个全局变量集
// 部分关键词名称不可使用 如 : false, true, null等
WXDIALOGUE_API WXDLhash_node* wxdl_state_add_global(WXDLstate* _state, const WXDLchar* _name, WXDLhash* _hash);

// 添加全局签名表
WXDIALOGUE_API WXDLhash* wxdl_state_add_sign(WXDLstate* _state, const WXDLchar* _sign, WXDLhash* _table);

// 获取独立标签表的表
WXDIALOGUE_API WXDLhash* wxdl_state_get_local_signs_table(WXDLstate* _state);

// 获取全局标签表的表
WXDIALOGUE_API WXDLhash* wxdl_state_get_signs_table(WXDLstate* _state);

// 获取指定名称全局标签表
WXDIALOGUE_API WXDLhash* wxdl_state_get_sign(WXDLstate* _state, const WXDLchar* _sign);

// 添加独立签名表(用于支持一些非全局标签的类型检查使用)
// 查找是顺序下去的
WXDIALOGUE_API WXDLhash* wxdl_state_add_local_sign(WXDLstate* _state, const WXDLchar* _name, WXDLhash* _table);

// 获取指定名称独立标签表
WXDIALOGUE_API WXDLhash* wxdl_state_get_local_sign(WXDLstate* _state, const WXDLchar* _name);

// 获取全局变量表
WXDIALOGUE_API WXDLhash* wxdl_state_get_global(WXDLstate* _state);

WXDIALOGUE_API WXDLhash_node* wxdl_state_add_func(WXDLstate* _state, const WXDLchar* _name, WXDLfunction func, WXDLbool _is_change_param);

WXDIALOGUE_API WXDLfunction_info wxdl_state_get_func(WXDLstate* _state, const WXDLchar* _name);

// 获取全局函数表
WXDIALOGUE_API WXDLhash* wxdl_state_get_func_table(WXDLstate* _state);

// 获取文本构建器
WXDIALOGUE_API WXDLstring_builder* wxdl_state_get_string_builder(WXDLstate* _state);

// WXDLtext===============================================================

// 创建文本
WXDIALOGUE_API WXDLtext* wxdl_new_text();

WXDIALOGUE_API void wxdl_free_text(WXDLtext* _text);

// 添加文本节点
WXDIALOGUE_API WXDLtext_node* wxdl_text_add(WXDLtext* _text, WXDLtext_node* _node);

// 获取头节点
WXDIALOGUE_API WXDLtext_node* wxdl_text_head(WXDLtext* _text);

// 获取指定位置节点
// _index大于大小时, 返回最后一个节点
WXDIALOGUE_API WXDLtext_node* wxdl_text_at(WXDLtext* _text, WXDLu64 _index);

// 将指定节点运行
// 通过索引运行
// 当节点为sign时, 会将其中数据设置到_vartable中, 前提该数据存在于_vartable, 且不为表(因为会往下迭代)
// _vartable一般是wxdl_state_gen_sign_table生成的
WXDIALOGUE_API void wxdl_text_runing_at(WXDLtext* _text, WXDLu64 _index, WXDLhash* _vartable);

// 获取节点数量
WXDIALOGUE_API WXDLu64 wxdl_text_size(WXDLtext* _text);



#endif
