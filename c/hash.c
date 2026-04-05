
#include "std.h"
#include "hash.h"
#include "arr.h"
#include <stdlib.h>

typedef struct WXDLhash
{
	WXDLhash_node** table;
	WXDLu32 table_size;
	WXDLu32 size;
}WXDLhash;

#define _WXDL_HASH_EXT_SIZE_FATOR 1.5

#define _WXDL_HASH_EXT_FATOR 0.85

// function=======================================================================================================================

// hash============================================================================

WXDLhash* wxdl_new_hash(WXDLu32 _table_size)
{
	WXDLhash* h = (WXDLhash*)wxdl_malloc(sizeof(WXDLhash));
	h->table = wxdl_malloc(sizeof(WXDLhash_node*) * _table_size);
	wxdl_set(h->table, 0, sizeof(WXDLhash_node*) * _table_size);
	h->table_size = _table_size;
	h->size = 0;

	return h;
}

WXDLhash* wxdl_hash_copy(WXDLhash* _hash)
{
	if (_hash == NULL)
		return NULL;

	WXDLhash* h = wxdl_new_hash(_hash->table_size);
	WXDLu64 count = _hash->size;
	h->size = _hash->size;
	for (WXDLu64 i = 0; count > 0; i++)
	{
		WXDLhash_node* n1 = _hash->table[i], *ln = NULL, *n;

		
		while (n1 != NULL)
		{
			n = wxdl_new_node();
			n->k = wxdl_new_str(n1->k);
			n->next = NULL;
			wxdl_value_copy(&n->v, &n1->v);
			// 判断是否为头节点
			if (ln == NULL)
			{
				h->table[i] = n;
			}
			else
			{
				ln->next = n;
			}

			ln = n;
			n1 = WXDL_NODE_NEXT(n1);
			count -= 1;
		}
	}

	return h;
}

void wxdl_hash_clear(WXDLhash* _hash)
{
	if (_hash == NULL)
		return;

	WXDLu64 size = _hash->size;
	for (int i = 0; size > 0; i++)
	{
		WXDLhash_node* n = _hash->table[i], * n2;
		for (; n != NULL;)
		{
			n2 = WXDL_NODE_NEXT(n);
			wxdl_free_node(n);
			n = n2;
			size -= 1;
		}
	}
}

void wxdl_free_hash(WXDLhash* _hash)
{
	if (_hash == NULL)
		return;
	
	wxdl_hash_clear(_hash);
	
	wxdl_free(_hash->table);

	wxdl_free(_hash);
	
}

WXDLhash_node* wxdl_hash_find(WXDLhash* _hash, const WXDLchar* _key)
{
	if (_hash == NULL || _key == NULL)
		return NULL;

	WXDLu64 code = wxdl_str_hashcode(_key) % _hash->table_size;

	WXDLhash_node* n = _hash->table[code];
	while (n != NULL)
	{
		if (wxdl_str_cmp(n->k, _key) == 0)
		{
			return n;
		}
		n = WXDL_NODE_NEXT(n);
	}
	return NULL;
}

WXDLu64 wxdl_hash_size(WXDLhash* _hash)
{
	if (_hash == NULL)
		return 0;
	return _hash->size;
}

// 用于扩容辅助
void _wxdl_hash_add_last_node(WXDLhash* _hash, WXDLhash_node* _node)
{
	WXDLu64 code = wxdl_str_hashcode(_node->k) % _hash->table_size;

	WXDLhash_node** pn = &(_hash->table[code]);

	WXDL_NODE_NEXT(_node) = *pn;
	*pn = _node;
}

// 扩容大小
void _wxdl_hash_ext(WXDLhash* _hash, WXDLu64 _new_size)
{
	if (_new_size <= _hash->table_size)
		return;

	WXDLhash_node** ln = _hash->table;
	_hash->table_size = (WXDLu32)_new_size;
	_hash->table = wxdl_malloc(sizeof(WXDLhash_node*) * _hash->table_size);
	wxdl_set(_hash->table, 0, sizeof(WXDLhash_node*) * _hash->table_size);
	
	for (WXDLu64 i = 0, j = _hash->size; j > 0; i++)
	{
		WXDLhash_node* n = ln[i];
		while (n != NULL)
		{
			WXDLhash_node* n2 = WXDL_NODE_NEXT(n);

			_wxdl_hash_add_last_node(_hash, n);
			n = n2;
			j -= 1;
		}
	}

	wxdl_free(ln);
}

// 尝试添加, 判断是否要扩容
void _wxdl_hash_check_size(WXDLhash* _hash, WXDLu64 _add_size)
{
	WXDLu64 ns = _hash->size + _add_size;
	if (ns > _hash->table_size * _WXDL_HASH_EXT_FATOR)
	{
		// 计算扩大容量
		WXDLu64 size = (WXDLu64)(_hash->table_size * _WXDL_HASH_EXT_SIZE_FATOR);
		while (size < ns)
			size = (WXDLu64)(size * _WXDL_HASH_EXT_SIZE_FATOR);
		_wxdl_hash_ext(_hash, size);
	}
}

// add=====================================================

WXDLhash_node* wxdl_hash_add_null(WXDLhash* _hash, const WXDLchar* _key)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	_wxdl_hash_check_size(_hash, 1);

	WXDLu64 code = wxdl_str_hashcode(_key) % _hash->table_size;

	WXDLhash_node** pn = &(_hash->table[code]);
	WXDLhash_node** lpn = NULL;

	// 查找重名节点, 若没有则添加
	while (*pn != NULL && wxdl_str_cmp(WXDL_NODE_KEY((*pn)), _key) != 0)
	{
		
		lpn = pn;
		pn = &(WXDL_NODE_NEXT((*pn)));
	}

	
	// 判断该名称节点是否存在
	if (*pn == NULL)
	{
		WXDLhash_node* n = wxdl_new_node();
		if (lpn != NULL)
			WXDL_NODE_NEXT((*lpn)) = n;
		else
			*pn = n;
		n->k = wxdl_new_str(_key);
		_hash->size += 1;
		return n;
	}
	else
	{
		wxdl_set_node_null(*pn);
		return *pn;
	}
}

WXDLhash_node* wxdl_hash_add_bool(WXDLhash* _hash, const WXDLchar* _key, WXDLbool _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_bool(n, _v);

	return n;
}

WXDLhash_node* wxdl_hash_add_int(WXDLhash* _hash, const WXDLchar* _key, WXDLint _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_int(n, _v);

	return n;
}

WXDLhash_node* wxdl_hash_add_u64(WXDLhash* _hash, const WXDLchar* _key, WXDLu64 _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_u64(n, _v);

	return n;
}

WXDLhash_node* wxdl_hash_add_float(WXDLhash* _hash, const WXDLchar* _key, WXDLfloat _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_float(n, _v);

	return n;
}

WXDLhash_node* wxdl_hash_add_str(WXDLhash* _hash, const WXDLchar* _key, const WXDLchar* _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_str(n, _v);

	return n;
}

WXDLhash_node* wxdl_hash_add_str_ref(WXDLhash* _hash, const WXDLchar* _key, WXDLchar* _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_str_ref(n, _v);

	return n;
}

WXDLhash_node* wxdl_hash_add_hash(WXDLhash* _hash, const WXDLchar* _key, struct WXDLhash* _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_hash(n, _v);

	return n;
}

WXDLhash_node* wxdl_hash_add_arr(WXDLhash* _hash, const WXDLchar* _key, struct WXDLarr* _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_arr(n, _v);

	return n;
}

WXDLhash_node* wxdl_hash_add_v(WXDLhash* _hash, const WXDLchar* _key, struct WXDLvalue* _v)
{
	if (_hash == NULL || _key == NULL)
		return NULL;
	WXDLhash_node* n = wxdl_hash_add_null(_hash, _key);
	wxdl_set_node_v(n, _v);

	return n;
}

void wxdl_hash_set_has_data(WXDLhash* _h1, WXDLhash* _h2)
{
	if (_h1 == NULL || _h2 == NULL)
		return;

	WXDLu64 size = _h2->size;
	for (WXDLu64 i = 0; size > 0; i++)
	{
		WXDLhash_node* n = _h2->table[i], *n2;
		if (n != NULL)
		{
			n2 = wxdl_hash_find(_h1, n->k);
			if (n2 != NULL)
			{
				// 只会设置非表数据
				// 如果为表往下迭代
				if (WXDL_NODE_TYPE(n) == WXDL_TYPE_DIC && WXDL_NODE_TYPE(n2) == WXDL_TYPE_DIC)
				{
					wxdl_hash_set_has_data(WXDL_NODE_DIC(n2), WXDL_NODE_DIC(n));
				}
				else
				{
					// 必须类型可转
					if (wxdl_is_type_convert(WXDL_NODE_TYPE(n2), WXDL_NODE_TYPE(n)))
						wxdl_set_node_v(n2, &n->v);
				}
			}

			size -= 1;
		}

	}
}

WXDLhash_node* wxdl_hash_remove(WXDLhash* _hash, const WXDLchar* _key)
{
	if (_hash == NULL || _key == NULL)
		return NULL;

	
	WXDLu64 code = wxdl_str_hashcode(_key) % _hash->table_size;

	WXDLhash_node* n = _hash->table[code], *ln = NULL;
	while (n != NULL)
	{
		if (wxdl_str_cmp(n->k, _key) == 0)
		{
			if (ln != NULL)
				WXDL_NODE_NEXT(ln) = WXDL_NODE_NEXT(n);
			_hash->size -= 1;
			return n;
		}

		ln = n;
		n = WXDL_NODE_NEXT(n);
	}
	return NULL;
}

void wxdl_hash_delete(WXDLhash* _hash, const WXDLchar* _key)
{
	WXDLhash_node* n = wxdl_hash_remove(_hash, _key);

	if (n != NULL)
		wxdl_free_node(n);
}


// node==========================================================================
WXDLhash_node* wxdl_new_node()
{
	WXDLhash_node* n = (WXDLhash_node*)wxdl_malloc(sizeof(WXDLhash_node));
	WXDL_NODE_KEY(n) = NULL;
	WXDL_NODE_NEXT(n) = NULL;
	WXDL_NODE_INT(n) = 0;
	WXDL_NODE_FLAG(n) = 0;
	WXDL_NODE_TYPE(n) = WXDL_TYPE_NULL;

	return n;
}

void wxdl_free_node(WXDLhash_node* _n)
{
	if (_n == NULL) 
		return;

	wxdl_free(WXDL_NODE_KEY(_n));
	wxdl_free_value(&_n->v);
	wxdl_free(_n);
}

void wxdl_set_node_null(WXDLhash_node* _n)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
}

void wxdl_set_node_bool(WXDLhash_node* _n, WXDLbool _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	WXDL_NODE_BOOL(_n) = _v;
	WXDL_NODE_TYPE(_n) = WXDL_TYPE_BOOL;
}

void wxdl_set_node_int(WXDLhash_node* _n, WXDLint _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	WXDL_NODE_INT(_n) = _v;
	WXDL_NODE_TYPE(_n) = WXDL_TYPE_INT;
}

void wxdl_set_node_u64(WXDLhash_node* _n, WXDLu64 _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	WXDL_NODE_UINT(_n) = _v;
	WXDL_NODE_TYPE(_n) = WXDL_TYPE_INT;
}

void wxdl_set_node_float(WXDLhash_node* _n, WXDLfloat _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	WXDL_NODE_FLOAT(_n) = _v;
	WXDL_NODE_TYPE(_n) = WXDL_TYPE_FLOAT;
}

void wxdl_set_node_str(WXDLhash_node* _n, const WXDLchar* _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	WXDL_NODE_STR(_n) = wxdl_new_str(_v);
	WXDL_NODE_TYPE(_n) = WXDL_TYPE_STR;
}

void wxdl_set_node_str_ref(WXDLhash_node* _n, WXDLchar* _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	WXDL_NODE_STR(_n) = _v;
	WXDL_NODE_TYPE(_n) = WXDL_TYPE_STR;
}


void wxdl_set_node_hash(WXDLhash_node* _n, WXDLhash* _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	WXDL_NODE_DIC(_n) = _v;
	WXDL_NODE_TYPE(_n) = WXDL_TYPE_DIC;
}

void wxdl_set_node_arr(WXDLhash_node* _n, WXDLarr* _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	WXDL_NODE_ARR(_n) = _v;
	WXDL_NODE_TYPE(_n) = WXDL_TYPE_ARR;
}

void wxdl_set_node_v(WXDLhash_node* _n, WXDLvalue* _v)
{
	if (_n == NULL)
		return;

	wxdl_free_value(&_n->v);
	wxdl_value_copy(&_n->v, _v);
}

// ite============================================================

WXDLbool _wxdl_hash_ite_next(WXDLiterator* _ite)
{
	WXDLhash* hash = (WXDLhash*)_ite->data;
	if ((WXDLu64)_ite->user1 >= hash->size) return WXDL_FALSE;
	else
	{
		
		WXDLhash_node* n = (WXDLhash_node*)_ite->user3;

		// 先尝试查找节点next
		if (n != NULL && n->next != NULL) 
			n = WXDL_NODE_NEXT(n);
		else n = NULL;

		if (n == NULL)
		{
			for (WXDLu64 i = (WXDLu64)_ite->user2; i < hash->table_size; i++)
			{
				n = hash->table[i];

				if (n != NULL)
				{
					// 记录迭代到的表索引后一位
					_ite->user2 = (WXDLchar*)i + 1;
					break;
				}
			}
		}

		_ite->user1 = (WXDLchar*)_ite->user1 + 1;
		// 记录当前指向节点
		_ite->user3 = n;
		
		if (n != NULL)
		{
			_ite->_v0 = &n->v;
			_ite->_v1 = n->k;
			_ite->v_count = 2;
		}

		return WXDL_TRUE;
	}
}

WXDLbool _wxdl_hash_ite_last(WXDLiterator* _ite)
{
	return WXDL_FALSE;
}

WXDLvalue* _wxdl_hash_ite_get(WXDLiterator* _ite)
{
	WXDLhash* hash = (WXDLhash*)_ite->data;
	if (hash->size == 0) return NULL;
	else return (WXDLvalue*)_ite->_v0;
}

WXDLiterator* wxdl_hash_ite(WXDLhash* _hash)
{
	if (_hash == NULL)
		return NULL;

	WXDLiterator* ite = (WXDLiterator*)wxdl_malloc(sizeof(WXDLiterator));

	wxdl_iterator_init(ite);

	ite->data = _hash;
	ite->next_func = _wxdl_hash_ite_next;
	ite->last_func = _wxdl_hash_ite_last;
	ite->get_func = _wxdl_hash_ite_get;

	_wxdl_hash_ite_next(ite);

	return ite;
}

const WXDLchar* wxdl_hash_ite_key(WXDLiterator* _ite)
{
	if (_ite != NULL)
	{
		return (const WXDLchar*)_ite->_v1;
	}
	else return NULL;
}