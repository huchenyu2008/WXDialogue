
#include "iterator.h"
#include "std.h"
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
	v->data.i = 0;
	v->type = WXDL_TYPE_NULL;

	return v;
}

WXDLvalue* wxdl_set_iterator_data_bool(WXDLiterator* _ite, WXDLbool _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	v->data.b = _v;
	v->type = WXDL_TYPE_BOOL;

	return v;
}

WXDLvalue* wxdl_set_iterator_data_int(WXDLiterator* _ite, WXDLint _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	v->data.i = _v;
	v->type = WXDL_TYPE_INT;

	return v;
}

WXDLvalue* wxdl_set_iterator_data_u64(WXDLiterator* _ite, WXDLu64 _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	v->data.u = _v;
	v->type = WXDL_TYPE_INT;

	return v;
}

WXDLvalue* wxdl_set_iterator_data_float(WXDLiterator* _ite, WXDLfloat _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	v->data.f = _v;
	v->type = WXDL_TYPE_FLOAT;

	return v;
}

WXDLvalue* wxdl_set_iterator_data_str(WXDLiterator* _ite, const WXDLchar* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	v->data.s = wxdl_new_str(_v);
	v->type = WXDL_TYPE_STR;

	return v;
}

WXDLvalue* wxdl_set_iterator_data_str_ref(WXDLiterator* _ite, WXDLchar* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	v->data.s = _v;
	v->type = WXDL_TYPE_STR;

	return v;
}

WXDLvalue* wxdl_set_iterator_data_hash(WXDLiterator* _ite, struct WXDLhash* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	v->data.d = _v;
	v->type = WXDL_TYPE_DIC;

	return v;
}

WXDLvalue* wxdl_set_iterator_data_arr(WXDLiterator* _ite, struct WXDLarr* _v)
{
	WXDLvalue* v = _ite->_v0;

	wxdl_free_value(v);
	v->data.a = _v;
	v->type = WXDL_TYPE_ARR;

	return v;
}