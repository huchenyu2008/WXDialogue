
#include "iterator.h"
#include "std.h"
#include "string_builder.h"
#include <stdlib.h>

WXDLbool wxdl_iterator_init(WXDLiterator* _ite)
{
	if (_ite == NULL)
		return WXDL_FALSE;

	_ite->magic = "";
	_ite->_v0 = NULL;
	_ite->_v1 = NULL;

	_ite->v_count = 0;

	_ite->next_func = NULL;
	_ite->last_func = NULL;
	_ite->get_func = NULL;
	_ite->free_func = NULL;

	_ite->data = NULL;
	_ite->user1 = NULL;
	_ite->user2 = NULL;
	_ite->user3 = NULL;

	return WXDL_TRUE;
}

void wxdl_iterator_free(WXDLiterator* _ite)
{
	if (_ite->free_func != NULL)
	{
		_ite->free_func(_ite);
	}

	wxdl_free(_ite);
}

// move==================================================================================

WXDLbool wxdl_iterator_next(WXDLiterator* _ite)
{
	if (_ite != NULL && _ite->next_func != NULL)
	{
		return _ite->next_func(_ite);
	}
	else return WXDL_FALSE;
}


WXDLbool wxdl_iterator_last(WXDLiterator* _ite)
{
	if (_ite != NULL && _ite->last_func != NULL)
	{
		return _ite->last_func(_ite);
	}
	else return WXDL_FALSE;
}


// get and set==================================================================================

WXDLvalue* wxdl_iterator_get(WXDLiterator* _ite)
{
	if (_ite != NULL && _ite->get_func != NULL)
	{
		return _ite->get_func(_ite);
	}
	else return NULL;
}


WXDLvalue* wxdl_set_iterator_data_null(WXDLiterator* _ite)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_NULL(*v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_bool(WXDLiterator* _ite, WXDLbool _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_BOOL(*v, _v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_int(WXDLiterator* _ite, WXDLint _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_INT(*v, _v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_u64(WXDLiterator* _ite, WXDLu64 _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_UINT(*v, _v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_float(WXDLiterator* _ite, WXDLfloat _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_FLOAT(*v, _v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_str(WXDLiterator* _ite, const WXDLchar* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_STR(*v, wxdl_build_string(_ite->builder, _v));

	return v;
}

WXDLvalue* wxdl_set_iterator_data_str_ref(WXDLiterator* _ite, WXDLstring* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_STR(*v, _v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_hash(WXDLiterator* _ite, struct WXDLhash* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_DIC(*v, _v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_hash_ref(WXDLiterator* _ite, struct WXDLhash* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_DIC_REF(*v, _v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_arr(WXDLiterator* _ite, struct WXDLarr* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_ARR(*v, _v);

	return v;
}

WXDLvalue* wxdl_set_iterator_data_arr_ref(WXDLiterator* _ite, struct WXDLarr* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	WXDL_V_SET_ARR_REF(*v, _v);

	return v;
}