
#include "call.h"
#include "string_builder.h"
#include "define.h"
#include "arr.h"
#include "std.h"
#include "parse.h"
#include <string.h>

WXDLcall* wxdl_new_call(const WXDLchar* _name, WXDLfunction _func, WXDLvalue* _argv, WXDLu32 _argc, WXDLstring_builder* _builder)
{
	WXDLcall* c = (WXDLcall*)wxdl_malloc(sizeof(WXDLcall));
	c->func = _func;
	if (_builder == NULL) _builder = wxdl_get_global_builder();
	c->builder = _builder;
	if (_argv != NULL)
	{
		wxdl_copy(c->argv, _argv, sizeof(WXDLvalue) * _argc);
	}
	c->argc = _argc;
	if (_name != NULL)
	    c->name = wxdl_build_string(_builder, _name);
	c->where = NULL;
	c->line = 0;
	c->xpos = 0;
	return c;
}

void wxdl_free_call(WXDLcall* _call, WXDLbool _is_free_param)
{
	if (_call == NULL)
		return;

	if (_is_free_param)
	    wxdl_free_value_arr(_call->argv, _call->argc);

	if (_call->where != NULL)
		wxdl_free_string(_call->where);
	if (_call->name != NULL)
		wxdl_free_string(_call->name);
}

WXDLerror wxdl_call_ext(WXDLcall* _v, struct WXDLloader* _loader, WXDLvalue* _ret, WXDLbool _after_destroy)
{
	if (_v == NULL || _v->func == NULL)
	{
	    return 1;
	}
	//WXDLvalue v[WXDL_FUNC_MAX_PARAM_COUNT];
	WXDLcall* c = NULL;
	WXDLerror err = 0;

	WXDLptr lp = wxdl_loader_userdata(_loader);
	wxdl_set_loader_userdata(_loader, _v);
	if (_after_destroy)
	{
	    err = _v->func(_loader, _v->argv, _v->argc, _ret);
        wxdl_free_call(_v, WXDL_TRUE);
	}
	else
	{
	    c = wxdl_call_copy(_v);
		err = c->func(_loader, c->argv, c->argc, _ret);
		wxdl_free_call(c, WXDL_TRUE);
	}
	wxdl_set_loader_userdata(_loader, lp);
	return err;
}

WXDLerror wxdl_call(WXDLcall* _v, struct WXDLloader* _loader, WXDLvalue* _ret)
{
    return wxdl_call_ext(_v, _loader, _ret, WXDL_FALSE);
}

WXDLcall* wxdl_call_copy(WXDLcall* _v2)
{
	if (_v2 == NULL) return NULL;
	WXDLcall* _v1 = wxdl_new_call(NULL, _v2->func, NULL, 0, _v2->builder);
	_v1->name = wxdl_string_ref(_v2->name);

	for (WXDLu64 i = 0; i < _v2->argc; i++)
	{
		wxdl_value_copy(&_v1->argv[i], &_v2->argv[i]);
	}
	_v1->argc = _v2->argc;
	return _v1;
}

// add===========================================================================
WXDLvalue* wxdl_call_add_null(WXDLcall* _c)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_null(_c, _c->argc);
}

WXDLvalue* wxdl_call_add_bool(WXDLcall* _c, WXDLbool _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_bool(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_int(WXDLcall* _c, WXDLint _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_int(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_u64(WXDLcall* _c, WXDLu64 _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_u64(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_float(WXDLcall* _c, WXDLfloat _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_float(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_str(WXDLcall* _c, const WXDLchar* _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_str(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_str_ref(WXDLcall* _c, WXDLstring* _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_str_ref(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_hash(WXDLcall* _c, WXDLhash* _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_hash(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_arr(WXDLcall* _c, WXDLarr* _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_arr(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_call(WXDLcall* _c, WXDLcall* _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_call(_c, _c->argc, _v);
}

WXDLvalue* wxdl_call_add_call_ref(WXDLcall* _c, WXDLcall* _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_call_ref(_c, _c->argc, _v);
}

// set===========================================================================
WXDLvalue* wxdl_call_set_null(WXDLcall* _c, WXDLu64 _index)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_NULL(_c->argv[_index]);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_bool(WXDLcall* _c, WXDLu64 _index, WXDLbool _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_BOOL(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_int(WXDLcall* _c, WXDLu64 _index, WXDLint _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_INT(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_u64(WXDLcall* _c, WXDLu64 _index, WXDLu64 _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_UINT(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_float(WXDLcall* _c, WXDLu64 _index, WXDLfloat _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_FLOAT(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_str(WXDLcall* _c, WXDLu64 _index, const WXDLchar* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_STR(_c->argv[_index], wxdl_build_string(_c->builder, _v));
    return &_c->argv[_index];
}


WXDLvalue* wxdl_call_set_str_ref(WXDLcall* _c, WXDLu64 _index, WXDLstring* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_STR(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_hash(WXDLcall* _c, WXDLu64 _index, WXDLhash* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_DIC(_c->argv[_index], wxdl_hash_ref(_v));
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_arr(WXDLcall* _c, WXDLu64 _index, WXDLarr* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_ARR(_c->argv[_index], wxdl_arr_ref(_v));
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_call(WXDLcall* _c, WXDLu64 _index, WXDLcall* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_CALL(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_call_ref(WXDLcall* _c, WXDLu64 _index, WXDLcall* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc) _c->argc = (WXDLu32)_index + 1;
    WXDL_V_SET_CALL_REF(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLu64 wxdl_call_param_size(WXDLcall* _c)
{
    if (_c == NULL) return 0;
    return _c->argc;
}

WXDLfunction wxdl_call_param_func(WXDLcall* _c)
{
    if (_c == NULL) return NULL;
    return _c->func;
}

// param===================================================================================================

WXDLvalue* wxdl_param_running(struct WXDLloader* _loader, WXDLvalue* _v)
{
    WXDLvalue* param = _v;
    if (WXDL_V_TYPE(*param) == WXDL_TYPE_CALL)
    {
        WXDLcall* c = WXDL_V_CALL(*param);
        WXDL_V_TYPE(*param) = WXDL_TYPE_NULL;
        wxdl_call_ext(c, _loader, param, WXDL_TRUE);
    }
    return param;
}

WXDLbool wxdl_param_bool(struct WXDLloader* _loader, WXDLvalue* _v)
{
    WXDLvalue* param = wxdl_param_running(_loader, _v);

    switch (WXDL_V_TYPE(*param))
    {
    case WXDL_TYPE_NULL:
        return 0;

    case WXDL_TYPE_BOOL:
    case WXDL_TYPE_INT:
        return (WXDLbool)WXDL_V_INT(*param);

    case WXDL_TYPE_FLOAT:
        return (WXDLbool)WXDL_V_FLOAT((*param));

    case WXDL_TYPE_DIC:
    case WXDL_TYPE_ARR:
    case WXDL_TYPE_STR:
    case WXDL_TYPE_PTR:
    case WXDL_TYPE_CALL:
        return (WXDLbool)(WXDLint)WXDL_V_PTR((*param));


    default:
        return 0;
    }

    return 0;
}

WXDLint wxdl_param_int(struct WXDLloader* _loader, WXDLvalue* _v)
{
    WXDLvalue* param = wxdl_param_running(_loader, _v);

    switch (WXDL_V_TYPE(*param))
    {
    case WXDL_TYPE_NULL:
        return 0;

    case WXDL_TYPE_BOOL:
    case WXDL_TYPE_INT:

        return WXDL_V_INT(*param);

    case WXDL_TYPE_FLOAT:
        return (WXDLint)WXDL_V_FLOAT((*param));

    case WXDL_TYPE_DIC:
    case WXDL_TYPE_ARR:
    case WXDL_TYPE_STR:
    case WXDL_TYPE_PTR:
    case WXDL_TYPE_CALL:
        return (WXDLint)WXDL_V_PTR((*param));


    default:
        return 0;
    }

    return 0;
}

WXDLfloat wxdl_param_float(struct WXDLloader* _loader, WXDLvalue* _v)
{
    WXDLvalue* param = wxdl_param_running(_loader, _v);

    switch (WXDL_V_TYPE(*param))
    {
    case WXDL_TYPE_NULL:
        return 0.;

    case WXDL_TYPE_BOOL:
    case WXDL_TYPE_INT:
        return (WXDLfloat)WXDL_V_INT(*param);

    case WXDL_TYPE_FLOAT:
        return WXDL_V_FLOAT((*param));

    case WXDL_TYPE_DIC:
    case WXDL_TYPE_ARR:
    case WXDL_TYPE_STR:
    case WXDL_TYPE_PTR:
    case WXDL_TYPE_CALL:
        return 0.;


    default:
        return 0.;
    }

    return 0.;
}

const WXDLchar* wxdl_param_str(struct WXDLloader* _loader, WXDLvalue* _v)
{
    WXDLstring* str = wxdl_param_str_ref(_loader, _v);

    if (str != NULL)
    {
        return str->str;
    }
    else return NULL;
}

WXDLstring* wxdl_param_str_ref(struct WXDLloader* _loader, WXDLvalue* _v)
{
    if (_loader == NULL || _v == NULL) return NULL;
    WXDLvalue* param = wxdl_param_running(_loader, _v);

    switch (WXDL_V_TYPE(*param))
    {
    case WXDL_TYPE_STR:
        return WXDL_V_STR(*param);

    default:
        return NULL;
    }

    return NULL;
}

WXDLhash* wxdl_param_hash(struct WXDLloader* _loader, WXDLvalue* _v)
{
    if (_loader == NULL || _v == NULL) return NULL;
    WXDLvalue* param = wxdl_param_running(_loader, _v);

    switch (WXDL_V_TYPE(*param))
    {
    case WXDL_TYPE_DIC:
        return WXDL_V_DIC(*param);

    default:
        return NULL;
    }

    return NULL;
}

WXDLarr* wxdl_param_arr(struct WXDLloader* _loader, WXDLvalue* _v)
{
    if (_loader == NULL || _v == NULL) return NULL;
    WXDLvalue* param = wxdl_param_running(_loader, _v);

    switch (WXDL_V_TYPE(*param))
    {
    case WXDL_TYPE_ARR:
        return WXDL_V_ARR(*param);

    default:
        return NULL;
    }

    return NULL;
}

WXDLcall* wxdl_param_call(struct WXDLloader* _loader, WXDLvalue* _v)
{
    if (_loader == NULL || _v == NULL) return NULL;
    WXDLvalue* param = wxdl_param_running(_loader, _v);

    switch (WXDL_V_TYPE(*param))
    {
    case WXDL_TYPE_CALL:
        return WXDL_V_CALL(*param);

    default:
        return NULL;
    }

    return NULL;
}
