
#include "arr.h"
#include "std.h"
#include <stdlib.h>

// 扩容因子
#define _WXDL_ARR_EXT_SIZE_FATOR 1.5

typedef struct WXDLarr
{
	WXDLvalue* data;
	WXDLu64 size;
	WXDLu64 max_size;
}WXDLarr;

// function================================================================================

WXDLarr* wxdl_new_arr(WXDLu64 _size)
{
	if (_size == 0)
		_size = 20;

	WXDLarr* a = wxdl_malloc(sizeof(WXDLarr));
	a->data = wxdl_malloc(sizeof(WXDLvalue) * _size);
	wxdl_set(a->data, 0, sizeof(WXDLvalue) * _size);
	a->size = 0;
	a->max_size = _size;

	return a;
}

WXDLarr* wxdl_arr_copy(WXDLarr* _arr)
{
	if (_arr == NULL)
		return NULL;

	WXDLarr* a = wxdl_new_arr(_arr->max_size);
	a->size = _arr->size;
	for (WXDLu64 i = 0; i < _arr->size; i++)
	{
		wxdl_value_copy(&a->data[i], &_arr->data[i]);
	}

	return a;
}

void wxdl_arr_clear(WXDLarr* _arr)
{
	if (_arr == NULL)
		return;

	for (int i = 0; i < _arr->size; i++)
	{
		wxdl_free_value(&_arr->data[i]);
	}
	_arr->size = 0;
}

void _wxdl_arr_clear_not_free(WXDLarr* _arr)
{
	if (_arr == NULL)
		return;
	_arr->size = 0;
	wxdl_set(_arr->data, 0, sizeof(WXDLvalue) * _arr->size);
}

void wxdl_free_arr(WXDLarr* _arr)
{
	if (_arr == NULL)
		return;

	wxdl_arr_clear(_arr);
	wxdl_free(_arr);
}

WXDLvalue* wxdl_arr_at(WXDLarr* _arr, WXDLu64 _index)
{
	if (_arr == NULL || _index >= _arr->size)
		return NULL;
	return &_arr->data[_index];
}

WXDLu64 wxdl_arr_size(WXDLarr* _arr)
{
	if (_arr == NULL)
		return 0;
	return _arr->size;
}

// 扩充数组
void _wxdl_arr_ext(WXDLarr* _arr, WXDLu64 _new_size)
{
	if (_new_size <= _arr->max_size)
		return;
	WXDLvalue* la = _arr->data;

	_arr->max_size = _new_size;
	_arr->data = wxdl_malloc(sizeof(WXDLvalue) * _arr->max_size);
	wxdl_copy(_arr->data, la, sizeof(WXDLvalue) * _arr->size);

	wxdl_free(la);
}

// 检查数组大小, 判断是否扩容, 如果要则扩容
void _wxdl_arr_check_size(WXDLarr* _arr, WXDLu64 _add_size)
{
	WXDLu64 ns = _arr->size + _add_size;
	if (ns > _arr->max_size)
	{
		// 计算扩大容量
		WXDLu64 size = (WXDLu64)(_arr->max_size * _WXDL_ARR_EXT_SIZE_FATOR);
		while (size < ns) 
			size = (WXDLu64)(size * _WXDL_ARR_EXT_SIZE_FATOR);

		_wxdl_arr_ext(_arr, size);
	}
}

// insert=======================================================

WXDLvalue* wxdl_arr_insert_null(WXDLarr* _arr, WXDLu64 _index)
{
	_wxdl_arr_check_size(_arr, 1);

	for (WXDLu64 i = _index; i < _arr->size; i++)
	{
		_arr->data[i + 1] = _arr->data[i];
	}
	_arr->data[_index].data.i = 0;
	_arr->data[_index].type = WXDL_TYPE_NULL;
	_arr->data[_index].flag = 0;
	_arr->size += 1;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_bool(WXDLarr* _arr, WXDLbool _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	_arr->data[_index].data.b = _v;
	_arr->data[_index].type = WXDL_TYPE_BOOL;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_int(WXDLarr* _arr, WXDLint _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	_arr->data[_index].data.i = _v;
	_arr->data[_index].type = WXDL_TYPE_INT;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_u64(WXDLarr* _arr, WXDLu64 _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	_arr->data[_index].data.u = _v;
	_arr->data[_index].type = WXDL_TYPE_INT;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_float(WXDLarr* _arr, WXDLfloat _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	_arr->data[_index].data.f = _v;
	_arr->data[_index].type = WXDL_TYPE_FLOAT;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_str(WXDLarr* _arr, const WXDLchar* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	_arr->data[_index].data.s = wxdl_new_str(_v);
	_arr->data[_index].type = WXDL_TYPE_STR;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_str_ref(WXDLarr* _arr, WXDLchar* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	_arr->data[_index].data.s = _v;
	_arr->data[_index].type = WXDL_TYPE_STR;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_hash(WXDLarr* _arr, WXDLhash* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	_arr->data[_index].data.d = _v;
	_arr->data[_index].type = WXDL_TYPE_DIC;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_arr(WXDLarr* _arr,  WXDLarr* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	_arr->data[_index].data.a = _v;
	_arr->data[_index].type = WXDL_TYPE_ARR;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_v(WXDLarr* _arr, WXDLvalue* _v, WXDLu64 _index)
{
	wxdl_value_copy(wxdl_arr_insert_null(_arr, _index), _v);
	return &_arr->data[_index];
}
// add==========================================================

WXDLvalue* wxdl_arr_add_null(WXDLarr* _arr)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.i = 0;
	v->type = WXDL_TYPE_NULL;
	v->flag = 0;
	return v;
}

WXDLvalue* wxdl_arr_add_bool(WXDLarr* _arr, WXDLbool _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.b = _v;
	v->type = WXDL_TYPE_BOOL;
	v->flag = 0;
	return v;
}

WXDLvalue* wxdl_arr_add_int(WXDLarr* _arr, WXDLint _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.i = _v;
	v->type = WXDL_TYPE_INT;
	v->flag = 0;
	return v;
}

WXDLvalue* wxdl_arr_add_u64(WXDLarr* _arr, WXDLu64 _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.u = _v;
	v->type = WXDL_TYPE_INT;
	v->flag = 0;
	return v;
}

WXDLvalue* wxdl_arr_add_float(WXDLarr* _arr, WXDLfloat _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.f = _v;
	v->type = WXDL_TYPE_FLOAT;
	v->flag = 0;
	return v;
}

WXDLvalue* wxdl_arr_add_str(WXDLarr* _arr, const WXDLchar* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.s = wxdl_new_str(_v);
	v->type = WXDL_TYPE_STR;
	v->flag = 0;
	return v;
}

WXDLvalue* wxdl_arr_add_str_ref(WXDLarr* _arr, WXDLchar* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.s = _v;
	v->type = WXDL_TYPE_STR;
	v->flag = 1;
	return v;
}

WXDLvalue* wxdl_arr_add_hash(WXDLarr* _arr, WXDLhash* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.d = _v;
	v->type = WXDL_TYPE_DIC;
	v->flag = 0;
	return v;
}

WXDLvalue* wxdl_arr_add_hash_ref(WXDLarr* _arr, WXDLhash* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.d = _v;
	v->type = WXDL_TYPE_DIC;
	v->flag = 1;
	return v;
}

WXDLvalue* wxdl_arr_add_arr(WXDLarr* _arr, WXDLarr* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.a = _v;
	v->type = WXDL_TYPE_ARR;
	v->flag = 0;
	return v;
}

WXDLvalue* wxdl_arr_add_arr_ref(WXDLarr* _arr, WXDLarr* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	v->data.a = _v;
	v->type = WXDL_TYPE_ARR;
	v->flag = 1;
	return v;
}

WXDLvalue* wxdl_arr_add_value(WXDLarr* _arr, WXDLvalue* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];

	wxdl_value_copy(v, _v);
	return v;
}

// remove=========================================

WXDLvalue wxdl_arr_remove(WXDLarr* _arr, WXDLu64 _index)
{
	WXDLvalue lv = {0};
	if (_arr == NULL || _index >= _arr->size)
		return lv;

	lv = _arr->data[_index];
	for (WXDLu64 i = _index; i < (_arr->size - 1); i++)
	{
		_arr[i] = _arr[i + 1];
	}
	
	return lv;
}

void wxdl_arr_delete(WXDLarr* _arr, WXDLu64 _index)
{
	if (_arr == NULL || _index >= _arr->size)
		return;

	wxdl_free_value(&(_arr->data[_index]));
	
	for (WXDLu64 i = _index; i < (_arr->size - 1); i++)
	{
		_arr[i] = _arr[i + 1];
	}

}

// ite============================================

WXDLbool _wxdl_arr_ite_next(WXDLiterator* _ite)
{
	WXDLarr* arr = (WXDLarr*)_ite->data;
	if ((WXDLu64)_ite->user1 + 1 > arr->size) return WXDL_FALSE;
	else
	{
		_ite->user1 = (WXDLchar*)_ite->user1 + 1;
		_ite->_v0 = wxdl_arr_at(arr, (WXDLu64)_ite->user1);
		return WXDL_TRUE;
	}
}

WXDLbool _wxdl_arr_ite_last(WXDLiterator* _ite)
{
	WXDLarr* arr = (WXDLarr*)_ite->data;
	if ((WXDLu64)_ite->user1 == 0) return WXDL_FALSE;
	else
	{
		_ite->user1 = (WXDLchar*)_ite->user1 - 1;
		_ite->_v0 = wxdl_arr_at(arr, (WXDLu64)_ite->user1);
		return WXDL_TRUE;
	}
}

WXDLvalue* _wxdl_arr_ite_get(WXDLiterator* _ite)
{
	WXDLarr* arr = (WXDLarr*)_ite->data;
	if ((WXDLu64)_ite->user1 > arr->size) return NULL;
	else
	{
		return _ite->data;
	}
	
}

WXDLiterator* wxdl_arr_ite(WXDLarr* _arr)
{
	if (_arr == NULL)
		return NULL;

	WXDLiterator* ite = (WXDLiterator*)wxdl_malloc(sizeof(WXDLiterator));

	wxdl_iterator_init(ite);

	ite->data = _arr;

	ite->user1 = 0;
	ite->_v0 = wxdl_arr_at(_arr, 0);

	ite->next_func = _wxdl_arr_ite_next;
	ite->last_func = _wxdl_arr_ite_last;
	ite->get_func = _wxdl_arr_ite_get;

	return ite;
}