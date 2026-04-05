#ifndef _WXDL_ITERATOR_H_
#define _WXDL_ITERATOR_H_

#include "define.h"

typedef struct WXDLiterator WXDLiterator;

typedef WXDLbool(*WXDLiterator_move_func)(WXDLiterator*);

typedef WXDLvalue*(*WXDLiterator_get_func)(WXDLiterator*);

typedef void (*WXDLiterator_free_func)(WXDLiterator*);

// 通用迭代器
typedef struct WXDLiterator
{
	const WXDLchar* magic;

	// 可用变量位
	// 设置变量时, 会选择该变量设置
	WXDLvalue* _v0;

	// 可用变量位
	// 无法修改的变量
	WXDLptr _v1;

	// 可用变量数量
	WXDLu64 v_count;

	WXDLiterator_move_func next_func;

	WXDLiterator_move_func last_func;

	WXDLiterator_get_func get_func;

	WXDLiterator_free_func free_func;

	WXDLptr data;

	// arr : 当前数组索引
	// hash : 当前字典顺序序号
	WXDLptr user1;

	// hash : 迭代到表的位置后一位
	WXDLptr user2;

	// hash : 当前指向节点
	WXDLptr user3;

}WXDLiterator;

// 初始化迭代器
WXDIALOGUE_API WXDLbool wxdl_iterator_init(WXDLiterator* _ite);

// 释放迭代器
WXDIALOGUE_API void wxdl_iterator_free(WXDLiterator* _ite);

// 指向下一个元素
// 返回是否成功
WXDIALOGUE_API WXDLbool wxdl_iterator_next(WXDLiterator* _ite);

// 指向上一个元素
// 注 : 字典不支持该迭代
// 返回是否成功
WXDIALOGUE_API WXDLbool wxdl_iterator_last(WXDLiterator* _ite);

// 获取迭代器当前指向数据
WXDIALOGUE_API WXDLvalue* wxdl_iterator_get(WXDLiterator* _ite);


// 设置迭代器当前指向数据

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_null(WXDLiterator* _ite);

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_bool(WXDLiterator* _ite, WXDLbool _v);

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_int(WXDLiterator* _ite, WXDLint _v);

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_u64(WXDLiterator* _ite, WXDLu64 _v);

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_float(WXDLiterator* _ite, WXDLfloat _v);

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_str(WXDLiterator* _ite, const WXDLchar* _v);

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_str_ref(WXDLiterator* _ite, WXDLchar* _v);

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_hash(WXDLiterator* _ite, struct WXDLhash* _v);

WXDIALOGUE_API WXDLvalue* wxdl_set_iterator_data_arr(WXDLiterator* _ite, struct WXDLarr* _v);

#endif
