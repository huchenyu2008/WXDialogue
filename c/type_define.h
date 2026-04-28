#ifndef _WXDIALOGUE_TYPE_DEF_H_
#define _WXDIALOGUE_TYPE_DEF_H_

#include "./define.h"

// base============================================================================================

typedef struct WXDLhash
{
	WXDLstring_builder* builder;
	WXDLhash_node** table;
	WXDLu32 table_size;
	WXDLu32 size;
	WXDLu32 lock;
	WXDLint32 refcount;
	WXDLu32 lockthread;
}WXDLhash;

// state============================================================================================

typedef struct WXDLstate
{
	// 全局数据
	// 用于代替数据别名
	// 比如 : color.black 数值为 0x000000
	WXDLhash* global;

	// 签名表(用于核对签名是否有误)
	WXDLhash* signs;

	// 非全局签名表
	// 使用要声明(用于支持一些非全局标签的类型检查使用)
	WXDLhash* local_signs;

	// 全部函数
	WXDLhash* funcs;

	// log缓存
	WXDLlogbuff logbuff;

	// 文本的节点
	// 注意, 每次解析都会将其释放掉
	// 如果需要长期使用, 请调用函数获得托管权
	WXDLtext_node* texts;

	WXDLtext_node* end_text;

	// pid的资源们
	WXDLthread_resoucre* pres;
	// pid可用链表
	WXDLu32* pres_uses;
	// 当前第一个可用的pid
	WXDLu32 pres_fisrt_use;

	WXDLu32 pid_size;
	WXDLu32 pid_max_size;

	WXDLstring_builder* builder;
}WXDLstate;

typedef struct WXDLtext
{
	WXDLtext_node* texts;
	WXDLtext_node* end_text;


	WXDLu64 size;
}WXDLtext;

typedef struct WXDLthread_resoucre
{
    WXDLu32 pid;
    WXDLcall* caller;
    // 当解析标签时，其默认为NULL
    WXDLhash* root;
    WXDLvalue R[WXDL_REG_SIZE];
    WXDLu64 inner_layer;
    // 在解析时才用的到
    // 用于辅助log
    WXDLchar* text;
    WXDLu64 text_size;
}WXDLthread_resoucre;

// loader====================================================================================================================================
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

    // 当前使用标签
    WXDLhash* psign;

    // 使用的本地签名表
    WXDLarr* use_local_signs;

    // 文件名称
    WXDLstring* where;

    // 是否直接运行
    WXDLbool is_running_call;

    // 当前的call层数
    WXDLu64 call_layer_count;

    // 使用的pid
    WXDLu32 pid;

    WXDLthread_resoucre* pres;

    // log缓存
    WXDLlogbuff* logbuff;

    // 用户数据
    WXDLptr userdata;

    WXDLstring_builder* builder;
}WXDLloader;

typedef struct WXDLblock
{
    WXDLhash* data;
    // data是否引用
    WXDLbool is_ref;
}WXDLblock;

#endif // _LGDIALOGUE_WXDL_H_
