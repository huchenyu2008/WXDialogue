#ifndef _WXDIALOGUE_ARR_H_
#define _WXDIALOGUE_ARR_H_
#include "./define.h"

#include "./iterator.h"


typedef struct WXDLarr
{
	WXDLvalue* data;
	WXDLu64 size;
	WXDLu64 max_size;
	WXDLstring_builder* builder;
	WXDLu64 refcount;
}WXDLarr;

// 创建数组
WXDIALOGUE_API WXDLarr* wxdl_new_arr(WXDLu64 _size, WXDLstring_builder* _builder);

// 引用数组
WXDIALOGUE_API WXDLarr* wxdl_arr_ref(WXDLarr* _arr);

// 拷贝数组
// _loader为NULL时, 不会运行call元素
WXDIALOGUE_API WXDLarr* wxdl_arr_copy_running(WXDLarr* _arr, struct WXDLloader* _loader);

// 拷贝数组
WXDIALOGUE_API WXDLarr* wxdl_arr_copy(WXDLarr* _arr);

// 清空数组
WXDIALOGUE_API void wxdl_arr_clear(WXDLarr* _arr);

// 清空数组
// 但不会释放资源
// 用于整个数组是引用数据, 或原始数据
WXDIALOGUE_API void _wxdl_arr_clear_not_free(WXDLarr* _arr);

// 销毁数组
WXDIALOGUE_API void wxdl_free_arr(WXDLarr* _arr);

// 获取数组元素
WXDIALOGUE_API WXDLvalue* wxdl_arr_at(WXDLarr* _arr, WXDLu64 _index);

// 获取数组大小
WXDIALOGUE_API WXDLu64 wxdl_arr_size(WXDLarr* _arr);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_null(WXDLarr* _arr, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_bool(WXDLarr* _arr, WXDLbool _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_int(WXDLarr* _arr, WXDLint _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_u64(WXDLarr* _arr, WXDLu64 _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_float(WXDLarr* _arr, WXDLfloat _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_str(WXDLarr* _arr, const WXDLchar* _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_str_ref(WXDLarr* _arr, WXDLstring* _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_hash(WXDLarr* _arr, WXDLhash* _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_arr(WXDLarr* _arr, WXDLarr* _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_call(WXDLarr* _arr, WXDLcall* _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_call_ref(WXDLarr* _arr, WXDLcall* _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_ptr(WXDLarr* _arr, WXDLptr _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_insert_v(WXDLarr* _arr, WXDLvalue* _v, WXDLu64 _index);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_null(WXDLarr* _arr);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_bool(WXDLarr* _arr, WXDLbool _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_int(WXDLarr* _arr, WXDLint _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_u64(WXDLarr* _arr, WXDLu64 _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_float(WXDLarr* _arr, WXDLfloat _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_str(WXDLarr* _arr, const WXDLchar* _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_str_ref(WXDLarr* _arr, WXDLstring* _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_hash(WXDLarr* _arr, WXDLhash* _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_arr(WXDLarr* _arr, WXDLarr* _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_call(WXDLarr* _arr, WXDLcall* _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_call_ref(WXDLarr* _arr, WXDLcall* _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_ptr(WXDLarr* _arr, WXDLptr _v);

WXDIALOGUE_API WXDLvalue* wxdl_arr_add_value(WXDLarr* _arr, WXDLvalue* _v);

// 将元素卸下, 但不销毁
WXDIALOGUE_API WXDLvalue wxdl_arr_remove(WXDLarr* _arr, WXDLu64 _index);

// 将元素卸下, 并销毁
WXDIALOGUE_API void wxdl_arr_delete(WXDLarr* _arr, WXDLu64 _index);


// 生成数组迭代器
WXDIALOGUE_API WXDLiterator* wxdl_arr_ite(WXDLarr* _arr);

#endif
