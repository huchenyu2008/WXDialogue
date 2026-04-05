#ifndef _WXDL_STATE_H_
#define _WXDL_STATE_H_
#include "define.h"
#include "hash.h"

// 状态机
typedef struct WXDLstate WXDLstate;

// 每个文本节点
typedef struct WXDLtext_node
{
	// 节点类型
	WXDLflag type;
		// 文本
		#define WXDL_TEXT_NODE_TEXT 0
		// 签名
		#define WXDL_TEXT_NODE_SIGN 1

	// type == WXDL_TEXT_NODE_TEXT : 此为字符串文本
	// type == WXDL_TEXT_NODE_SIGN : 此为数据标签的名称(全局签名表名称)
	WXDLchar* text;

	// 储存标签数据
	// type == WXDL_TEXT_NODE_TEXT 时, 常态为NULL
	struct WXDLhash* data;

	struct WXDLtext_node* next;

	// 使用的独立签名表名称
	struct WXDLarr* use_local_tables;
}WXDLtext_node;

typedef struct WXDLtext WXDLtext;

#define WXDL_NEW_TEXT_NODE(type, text, data) (WXDLtext_node){.type = type, .text = text, .data = data, .next = NULL};

// 创建状态机
WXDIALOGUE_API WXDLstate* wxdl_new_state();

// 释放状态机
// 注意它会把所以你提供的字典(签名表和全局数据表)释放, 避免访问无效内存
WXDIALOGUE_API void wxdl_free_state(WXDLstate* _state);

// 根据签名表生成一个表
WXDIALOGUE_API WXDLhash* wxdl_state_gen_sign_table(WXDLstate* _state);

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