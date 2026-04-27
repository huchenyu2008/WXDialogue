
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

void wxdl_c_arr_ext(void** data ,WXDLu32 _size ,WXDLu32* _max_size, WXDLu32 _new_size, WXDLu32 _typesize)
{
	if (_new_size <= *_max_size)
		return;
	void* la = *data;

	*_max_size = _new_size;
	*data = wxdl_malloc(_typesize * (*_max_size));
	wxdl_copy(*data, la, _typesize * _size);

	wxdl_free(la);
}

void _wxdl_c_arr_check_size(void** data ,WXDLu32 _size ,WXDLu32* _max_size, WXDLu32 _add_size, WXDLu32 _typesize)
{
	WXDLu32 ns = _size + _add_size;
	if (ns > *_max_size)
	{
		// 计算扩大容量
		WXDLu32 size = (WXDLu32)((*_max_size) * _WXDL_ARR_EXT_SIZE_FATOR);
		while (size < ns)
			size = (WXDLu32)(size * _WXDL_ARR_EXT_SIZE_FATOR);

		wxdl_c_arr_ext(data, _size, _max_size, size, _typesize);
	}
}

WXDLarr* wxdl_new_arr(WXDLu64 _size, WXDLstring_builder* _builder)
{
	if (_size == 0)
		_size = 20;

	WXDLarr* a = wxdl_malloc(sizeof(WXDLarr));
	a->data = wxdl_malloc(sizeof(WXDLvalue) * _size);
	wxdl_set(a->data, 0, sizeof(WXDLvalue) * _size);
	a->size = 0;
	a->max_size = _size;
	if (_builder == NULL) _builder =  wxdl_get_global_builder();
	a->builder = wxdl_builder_ref(_builder);
	a->refcount = 1;
	a->lock = 0;
	a->lockthread = 0;
	return a;
}

WXDLarr* wxdl_arr_ref(WXDLarr* _arr)
{
	if (_arr == NULL)
		return NULL;

	_arr->refcount += 1;

	return _arr;
}

void wxdl_arr_lock(WXDLarr* _arr)
{
    WXDLu32 tid = wxdl_get_thread_id();
    for (;_arr->lock != 0 && _arr->lockthread != tid;)
    {
        CPU_PAUSE();
    };
    _arr->lock += 1;
    _arr->lockthread = tid;
}

void wxdl_arr_unlock(WXDLarr* _arr)
{
    _arr->lock = 0;
}

WXDLarr* wxdl_arr_copy_running(WXDLarr* _arr, struct WXDLstate* _state, WXDLu32 _pid)
{
	if (_arr == NULL)
		return NULL;

	wxdl_arr_lock(_arr);
	WXDLarr* a = wxdl_new_arr(_arr->max_size, _arr->builder);
	a->size = _arr->size;
	WXDLvalue* v = NULL;
	for (WXDLu64 i = 0; i < _arr->size; i++)
	{
		v = &_arr->data[i];
		if (_state != NULL && v->type == WXDL_TYPE_CALL)
		{
			wxdl_call(WXDL_V_CALL(*v), _state, v, _pid);
		}
		else
		    wxdl_value_shallow_copy(v, v);
	}

	wxdl_arr_unlock(_arr);
	return a;
}

WXDLarr* wxdl_arr_copy(WXDLarr* _arr)
{
	return wxdl_arr_copy_running(_arr, NULL, WXDL_INVAILD_PID);
}

void wxdl_arr_clear(WXDLarr* _arr)
{
	if (_arr == NULL)
		return;

	wxdl_arr_lock(_arr);
	for (int i = 0; i < _arr->size; i++)
	{
		wxdl_free_value(&_arr->data[i]);
	}
	_arr->size = 0;
	wxdl_arr_unlock(_arr);
}

void _wxdl_arr_clear_not_free(WXDLarr* _arr)
{
	if (_arr == NULL)
		return;
	wxdl_arr_lock(_arr);
	_arr->size = 0;
	wxdl_set(_arr->data, 0, sizeof(WXDLvalue) * _arr->size);
	wxdl_arr_unlock(_arr);
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
    wxdl_arr_lock(_arr);
    WXDLvalue* v = wxdl_arr_unsafe_at(_arr, _index);
    wxdl_arr_unlock(_arr);
    return v;
}

WXDLvalue* wxdl_arr_unsafe_at(WXDLarr* _arr, WXDLu64 _index)
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
    WXDLu32 ps = (WXDLu32)_arr->max_size;
    wxdl_c_arr_ext((void**)&_arr->data, (WXDLu32)_arr->size, &ps, (WXDLu32)_new_size, sizeof(WXDLvalue));
    _arr->max_size = (WXDLu64)ps;
}

// 检查数组大小, 判断是否扩容, 如果要则扩容
void _wxdl_arr_check_size(WXDLarr* _arr, WXDLu64 _add_size)
{
    WXDLu32 ps = (WXDLu32)_arr->max_size;
    _wxdl_c_arr_check_size((void**)&_arr->data, (WXDLu32)_arr->size, &ps, (WXDLu32)_add_size, sizeof(WXDLvalue));
    _arr->max_size = (WXDLu64)ps;
}

// insert=======================================================

WXDLvalue* wxdl_arr_insert_null(WXDLarr* _arr, WXDLu64 _index)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	for (WXDLu64 i = _index; i < _arr->size; i++)
	{
		_arr->data[i + 1] = _arr->data[i];
	}
	WXDLvalue* v = &_arr->data[_index];
	WXDL_V_SET_NULL(*v);
	_arr->size += 1;
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_insert_bool(WXDLarr* _arr, WXDLbool _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_BOOL(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_insert_int(WXDLarr* _arr, WXDLint _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_INT(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_insert_u64(WXDLarr* _arr, WXDLu64 _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_UINT(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_insert_float(WXDLarr* _arr, WXDLfloat _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_FLOAT(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_insert_str(WXDLarr* _arr, const WXDLchar* _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_STR(*v, wxdl_build_string(_arr->builder, _v));
	return v;
}

WXDLvalue* wxdl_arr_insert_str_ref(WXDLarr* _arr, WXDLstring* _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_STR(*v, wxdl_string_ref(_v));
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_hash(WXDLarr* _arr, WXDLhash* _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_DIC(*v, wxdl_hash_ref(_v));
	return &_arr->data[_index];
}

WXDLvalue* wxdl_arr_insert_arr(WXDLarr* _arr,  WXDLarr* _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_ARR(*v, wxdl_arr_ref(_v));
	return v;
}

WXDLvalue* wxdl_arr_insert_call(WXDLarr* _arr,  WXDLcall* _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_CALL(*v, wxdl_call_ref(_v));
	return v;
}

WXDLvalue* wxdl_arr_insert_ptr(WXDLarr* _arr,  WXDLptr _v, WXDLu64 _index)
{
	WXDLvalue* v = wxdl_arr_insert_null(_arr, _index);
	WXDL_V_SET_PTR(*v, _v);
	return v;
}

WXDLvalue* wxdl_arr_insert_v(WXDLarr* _arr, WXDLvalue* _v, WXDLu64 _index)
{
	wxdl_value_shallow_copy(wxdl_arr_insert_null(_arr, _index), _v);
	return _v;
}
// add==========================================================

WXDLvalue* wxdl_arr_add_null(WXDLarr* _arr)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_NULL(*v);
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_bool(WXDLarr* _arr, WXDLbool _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_BOOL(*v, _v);
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_int(WXDLarr* _arr, WXDLint _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_INT(*v, _v);
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_u64(WXDLarr* _arr, WXDLu64 _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_UINT(*v, _v);
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_float(WXDLarr* _arr, WXDLfloat _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_FLOAT(*v, _v);
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_str(WXDLarr* _arr, const WXDLchar* _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_STR(*v, wxdl_build_string(_arr->builder, _v));
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_str_ref(WXDLarr* _arr, WXDLstring* _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_STR(*v, wxdl_string_ref(_v));
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_hash(WXDLarr* _arr, WXDLhash* _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_DIC(*v, wxdl_hash_ref(_v));
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_arr(WXDLarr* _arr, WXDLarr* _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_ARR(*v, wxdl_arr_ref(_v));
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_call(WXDLarr* _arr, WXDLcall* _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_CALL(*v, wxdl_call_ref(_v));
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_ptr(WXDLarr* _arr, WXDLptr _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];
	WXDL_V_SET_PTR(*v, _v);
	wxdl_arr_unlock(_arr);
	return v;
}

WXDLvalue* wxdl_arr_add_value(WXDLarr* _arr, WXDLvalue* _v)
{
    wxdl_arr_lock(_arr);
	_wxdl_arr_check_size(_arr, 1);

	WXDLvalue* v = &_arr->data[_arr->size++];

	wxdl_value_shallow_copy(v, _v);
	wxdl_arr_unlock(_arr);
	return v;
}

// remove=========================================

WXDLvalue wxdl_arr_remove(WXDLarr* _arr, WXDLu64 _index)
{
    wxdl_arr_lock(_arr);
	WXDLvalue lv = {0};
	if (_arr == NULL || _index >= _arr->size)
		return lv;

	lv = _arr->data[_index];
	for (WXDLu64 i = _index; i < (_arr->size - 1); i++)
	{
		_arr[i] = _arr[i + 1];
	}

	wxdl_arr_unlock(_arr);
	return lv;
}

void wxdl_arr_delete(WXDLarr* _arr, WXDLu64 _index)
{
    WXDLvalue v = wxdl_arr_remove(_arr, _index);
    wxdl_free_value(&v);
}

// ite============================================

WXDLbool _wxdl_arr_ite_next(WXDLiterator* _ite)
{
	WXDLarr* arr = (WXDLarr*)_ite->data;
	if ((WXDLu64)_ite->user1 + 1 > arr->size)
	{
	    _ite->_v0 = NULL;
	    return WXDL_FALSE;
	}
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
