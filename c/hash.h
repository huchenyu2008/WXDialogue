#ifndef _WXDIALOGUE_HASH_H_
#define _WXDIALOGUE_HASH_H_
#include "./define.h"
#include "iterator.h"

typedef struct WXDLhash WXDLhash;

typedef struct WXDLhash_node
{
	WXDLchar* k;
	WXDLvalue v;
	struct WXDLhash_node* next;
}WXDLhash_node;

#define WXDL_NODE_NEXT(n) n->next

#define WXDL_NODE_KEY(n) n->k

#define WXDL_NODE_TYPE(n) n->v.type

#define WXDL_NODE_FLAG(n) n->v.flag

// get node value's data

#define WXDL_NODE_INT(n) n->v.data.i

#define WXDL_NODE_UINT(n) n->v.data.u

#define WXDL_NODE_FLOAT(n) n->v.data.f

#define WXDL_NODE_BOOL(n) n->v.data.b

#define WXDL_NODE_STR(n) n->v.data.s

#define WXDL_NODE_ARR(n) n->v.data.a

#define WXDL_NODE_DIC(n) n->v.data.d

// 创建字典
WXDIALOGUE_API WXDLhash* wxdl_new_hash(WXDLu32 _table_size);

// 拷贝字典
WXDIALOGUE_API WXDLhash* wxdl_hash_copy(WXDLhash* _hash);

// 清空字典
WXDIALOGUE_API void wxdl_hash_clear(WXDLhash* _hash);

// 销毁字典
WXDIALOGUE_API void wxdl_free_hash(WXDLhash* _hash);

// 获取字典数据
WXDIALOGUE_API WXDLhash_node* wxdl_hash_find(WXDLhash* _hash, const WXDLchar* _key);

// 获取字典元素数量
WXDIALOGUE_API WXDLu64 wxdl_hash_size(WXDLhash* _hash);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_null(WXDLhash* _hash, const WXDLchar* _key);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_bool(WXDLhash* _hash, const WXDLchar* _key, WXDLbool _v);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_int(WXDLhash* _hash, const WXDLchar* _key, WXDLint _v);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_u64(WXDLhash* _hash, const WXDLchar* _key, WXDLu64 _v);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_float(WXDLhash* _hash, const WXDLchar* _key, WXDLfloat _v);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_str(WXDLhash* _hash, const WXDLchar* _key, const WXDLchar* _v);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_str_ref(WXDLhash* _hash, const WXDLchar* _key, WXDLchar* _v);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_hash(WXDLhash* _hash, const WXDLchar* _key, struct WXDLhash* _v);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_arr(WXDLhash* _hash, const WXDLchar* _key, struct WXDLarr* _v);

WXDIALOGUE_API WXDLhash_node* wxdl_hash_add_v(WXDLhash* _hash, const WXDLchar* _key, struct WXDLvalue* _v);

// 设置目标hash表中已有的数据, 如果新数据的hash表有这个的话
// 不会设置表, 但会通过表往下迭代设置其属性
// 注 还会进行类型比较
WXDIALOGUE_API void wxdl_hash_set_has_data(WXDLhash* _h1, WXDLhash* _h2);

// 将元素卸下, 但不销毁
WXDIALOGUE_API WXDLhash_node* wxdl_hash_remove(WXDLhash* _arr, const WXDLchar* _key);

// 将元素卸下, 并销毁
WXDIALOGUE_API void wxdl_hash_delete(WXDLhash* _arr, const WXDLchar* _key);

//===========================================
// hash node
//===========================================

// 创建字典节点
WXDIALOGUE_API WXDLhash_node* wxdl_new_node();

WXDIALOGUE_API void wxdl_free_node(WXDLhash_node* _n);

WXDIALOGUE_API void wxdl_set_node_null(WXDLhash_node* _n);

WXDIALOGUE_API void wxdl_set_node_bool(WXDLhash_node* _n, WXDLbool _v);

WXDIALOGUE_API void wxdl_set_node_int(WXDLhash_node* _n, WXDLint _v);

WXDIALOGUE_API void wxdl_set_node_u64(WXDLhash_node* _n, WXDLu64 _v);

WXDIALOGUE_API void wxdl_set_node_float(WXDLhash_node* _n, WXDLfloat _v);

WXDIALOGUE_API void wxdl_set_node_str(WXDLhash_node* _n, const WXDLchar* _v);

WXDIALOGUE_API void wxdl_set_node_str_ref(WXDLhash_node* _n, WXDLchar* _v);

WXDIALOGUE_API void wxdl_set_node_hash(WXDLhash_node* _n, struct WXDLhash* _v);

WXDIALOGUE_API void wxdl_set_node_arr(WXDLhash_node* _n, struct WXDLarr* _v);

WXDIALOGUE_API void wxdl_set_node_v(WXDLhash_node* _n, WXDLvalue* _v);


// 生成字典迭代器
WXDIALOGUE_API WXDLiterator* wxdl_hash_ite(WXDLhash* _hash);

// 获取当前迭代到的节点的键
WXDIALOGUE_API const WXDLchar* wxdl_hash_ite_key(WXDLiterator* _ite);

#endif