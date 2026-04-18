
#include "arr.h"
#include "define.h"
#include "std.h"
#include "parse.h"
#include "call.h"
#include "string_builder.h"
#include <stdlib.h>

// 扩容因子
#define _WXDL_ARR_EXT_SIZE_FATOR 1.5


// function================================================================================

WXDLarr* wxdl_new_arr(WXDLu64 _size, WXDLstring_builder* _builder)
{
	if (_size == 0)
		_size = 20;

	WXDLarr* a = wxdl_malloc(sizeof(WXDLarr));
	a->data = wxdl_malloc(sizeof(WXDLvalue) * _size);
	wxdl_set(a->data, 0, sizeof(WXDLvalue) * _size);
	a->size = 0;
	a->max_size = _size;
	if (_builder == NULL) _builder = wxdl_get_global_builder();
	a->builder = _builder;
	a->refcount = 1;

	return a;
}

WXDLarr* wxdl_arr_ref(WXDLarr* _arr)
{
	if (_arr == NULL)
		return NULL;

	_arr->refcount += 1;

	return _arr;
}

WXDLarr* wxdl_arr_copy_running(WXDLarr* _arr, struct WXDLloader* _loader)
{
	if (_arr == NULL)
		return NULL;

	WXDLarr* a = wxdl_new_arr(_arr->max_size, _arr->builder);
	a->size = _arr->size;
	WXDLvalue* v = NULL;
	for (WXDLu64 i = 0; i < _arr->size; i++)
	{
		v = &_arr->data[i];
		if (_loader != NULL && v->type == WXDL_TYPE_CALL)
		{
			WXDLptr lp = wxdl_loader_userdata(_loader);
			wxdl_set_loader_userdata(_loader, WXDL_V_CALL(*v));
			wxdl_call(WXDL_V_CALL(*v), _loader, v);
			wxdl_set_loader_userdata(_loader, lp);
		}
		wxdl_value_shallow_copy(&a->data[i], v);
	}

	return a;
}

WXDLarr* wxdl_arr_copy(WXDLarr* _arr)
{
	return wxdl_arr_copy_running(_arr, NULL);
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

	if (_arr->refcount == 0)
	{
		wxdl_arr_clear(_arr);
		wxdl_free(_arr);
	}
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
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_NULL(*v);
	_arr->size += 1;
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_bool(WXDLarr* _arr, WXDLbool _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_BOOL(*v, _v);
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_int(WXDLarr* _arr, WXDLint _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_INT(*v, _v);
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_u64(WXDLarr* _arr, WXDLu64 _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_UINT(*v, _v);
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_float(WXDLarr* _arr, WXDLfloat _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_FLOAT(*v, _v);
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_str(WXDLarr* _arr, const WXDLchar* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_STR(*v, wxdl_build_string(_arr->builder, _v));
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_str_ref(WXDLarr* _arr, WXDLstring* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_STR(*v, _v);
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_hash(WXDLarr* _arr, WXDLhash* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_DIC(*v, wxdl_hash_ref(_v));
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_arr(WXDLarr* _arr,  WXDLarr* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_ARR(*v, wxdl_arr_ref(_v));
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_call(WXDLarr* _arr,  WXDLcall* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_CALL(*v, _v);
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_call_ref(WXDLarr* _arr,  WXDLcall* _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_CALL_REF(*v, _v);
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_ptr(WXDLarr* _arr,  WXDLptr _v, WXDLu64 _index)
{
	wxdl_arr_insert_null(_arr, _index);
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_PTR(*v, _v);
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_v(WXDLarr* _arr, WXDLvalue* _v, WXDLu64 _index)
{
	wxdl_value_shallow_copy(wxdl_arr_insert_null(_arr, _index), _v);
	return &_arr->data[_index];
}
// add==========================================================

WXDLvalue* wxdl_arr_add_null(WXDLarr* _arr)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];

	WXDL_V_SET_NULL(*v);
	return v;
}

WXDLvalue* wxdl_arr_add_bool(WXDLarr* _arr, WXDLbool _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_BOOL(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_add_int(WXDLarr* _arr, WXDLint _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_INT(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_add_u64(WXDLarr* _arr, WXDLu64 _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_UINT(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_add_float(WXDLarr* _arr, WXDLfloat _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_FLOAT(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_add_str(WXDLarr* _arr, const WXDLchar* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_STR(*v, wxdl_build_string(_arr->builder, _v));
	return v;
}

WXDLvalue* wxdl_arr_add_str_ref(WXDLarr* _arr, WXDLstring* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_STR(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_add_hash(WXDLarr* _arr, WXDLhash* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_DIC(*v, wxdl_hash_ref(_v));
	return v;
}

WXDLvalue* wxdl_arr_add_arr(WXDLarr* _arr, WXDLarr* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_ARR(*v, wxdl_arr_ref(_v));
	return v;
}

WXDLvalue* wxdl_arr_add_call(WXDLarr* _arr, WXDLcall* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_CALL(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_add_call_ref(WXDLarr* _arr, WXDLcall* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_CALL_REF(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_add_ptr(WXDLarr* _arr, WXDLptr _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_PTR(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_add_value(WXDLarr* _arr, WXDLvalue* _v)
{
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];

	wxdl_value_shallow_copy(v, _v);
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
	if ((WXDLu64)_ite->user1 >= arr->size) return NULL;
	else
	{
		return wxdl_arr_at(_ite->data, (WXDLu64)_ite->user1);
	}

}

WXDLiterator* wxdl_arr_ite(WXDLarr* _arr)
{
	if (_arr == NULL)
		return NULL;

	WXDLiterator* ite = (WXDLiterator*)wxdl_malloc(sizeof(WXDLiterator));

	wxdl_iterator_init(ite);

	ite->data = _arr;

	ite->builder = _arr->builder;
	ite->user1 = 0;
	ite->_v0 = wxdl_arr_at(_arr, 0);

	ite->next_func = _wxdl_arr_ite_next;
	ite->last_func = _wxdl_arr_ite_last;
	ite->get_func = _wxdl_arr_ite_get;

	return ite;
}
