
#include "call.h"
#include "define.h"
#include "state.h"
#include "string_builder.h"
#include "type_define.h"
#include "arr.h"
#include "std.h"
#include "parse.h"
#include <stdlib.h>
#include <string.h>

// 扩充数组
void _wxdl_call_ext(WXDLcall* _c, WXDLu32 _new_size)
{
	if (_new_size <= _c->max_argc)
		return;
	WXDLvalue* la = _c->argv;

	_c->max_argc = _new_size;
	_c->argv = wxdl_malloc(sizeof(WXDLvalue) * _c->max_argc);
	if (la != NULL)
	    wxdl_copy(_c->argv, la, sizeof(WXDLvalue) * _c->argc);

	wxdl_free(la);
}

// 检查数组大小, 判断是否扩容, 如果要则扩容
void _wxdl_call_check_size(WXDLcall* _c, WXDLu32 _add_size)
{
	WXDLu32 ns = _c->argc + _add_size;
	if (ns > _c->max_argc)
	{
		// 计算扩大容量
		WXDLu32 size = (WXDLu32)(_c->max_argc * 2);
		if (size == 0) size = 2;
		while (size < ns)
			size = (WXDLu32)(size * 2);
		_wxdl_call_ext(_c, size);
	}
}

WXDLcall* wxdl_new_call(const WXDLchar* _name, WXDLfunction_info* _func, WXDLvalue* _argv, WXDLu32 _argc, WXDLstring_builder* _builder)
{

	WXDLcall* c = (WXDLcall*)wxdl_malloc(sizeof(WXDLcall));
	if (_builder == NULL) _builder = wxdl_get_global_builder();
	c->argc = 0;
	c->max_argc = 0;
	c->argv = NULL;
	if (_argc == 0)
	    _argc = 2;
	_wxdl_call_check_size(c, _argc);
	if (_argv != NULL)
	{
		wxdl_copy(c->argv, _argv, sizeof(WXDLvalue) * _argc);
		c->argc = _argc;
	}
	if (_name != NULL)
	    c->name = wxdl_build_string(_builder, _name);
	else c->name = NULL;
	c->where = NULL;
	c->line = 0;
	c->xpos = 0;
	c->refcount = 1;

	if (_func != NULL)
	{
	    c->func = (WXDLfunction)_func->func;
		c->is_const_param = !_func->is_change_param;
	}
	return c;
}

WXDLcall* wxdl_call_ref(WXDLcall* _call)
{
    if (_call != NULL)
    {
        _call->refcount += 1;
    }
    return _call;
}

void wxdl_call_set_const_param(WXDLcall* _call, WXDLbool _is_const_param)
{
    if (_call == NULL)
		return;

    _call->is_const_param = _is_const_param;
    return;
}

void wxdl_free_call(WXDLcall* _call)
{
	if (_call == NULL)
		return;

	_call->refcount -= 1;
	if (_call->refcount > 0) return;

	wxdl_free_value_arr(_call->argv, _call->argc);

	if (_call->where != NULL)
		wxdl_free_string(_call->where);
	if (_call->name != NULL)
		wxdl_free_string(_call->name);
}

WXDLerror wxdl_call_ext(WXDLcall* _v, struct WXDLstate* _state, WXDLvalue* _ret, WXDLthread_resoucre* _pres, WXDLbool _is_ref_param)
{
	if (_v == NULL || _v->func == NULL)
	{
	    return 1;
	}
	//WXDLvalue v[WXDL_FUNC_MAX_PARAM_COUNT];
	WXDLcall* c = NULL;
	WXDLerror err = 0;

	// 用于优化, 如果不用更改参数的话
	if (_is_ref_param || _v->is_const_param)
	{
	    err = _v->func(_state, _v, _v->argv, _v->argc, _ret, _pres);
	}
	else
	{
	    c = wxdl_call_copy(_v);
		err = c->func(_state, _v, c->argv, c->argc, _ret, _pres);
		wxdl_free_call(c);
	}
	return err;
}

WXDLerror wxdl_call(WXDLcall* _v, struct WXDLstate* _state, WXDLvalue* _ret, WXDLthread_resoucre* _pres)
{
    return wxdl_call_ext(_v, _state, _ret, _pres, WXDL_FALSE);
}

WXDLcall* wxdl_call_copy(WXDLcall* _v2)
{
	if (_v2 == NULL) return NULL;
	WXDLcall* _v1 = wxdl_new_call(NULL, NULL, NULL, _v2->argc, NULL);
	_v1->name = wxdl_string_ref(_v2->name);
	_v1->func = _v2->func;
	_v1->is_const_param = _v2->is_const_param;

	for (WXDLu32 i = 0; i < _v2->argc; i++)
	{
		wxdl_value_copy(&_v1->argv[i], &_v2->argv[i]);
	}
	_v1->argc = _v2->argc;
	return _v1;
}

void wxdl_call_clear(WXDLcall* _c)
{
    if (_c == NULL) return;
    wxdl_free_value_arr(_c->argv, _c->argc);
    _c->argc = 0;
}

WXDLvalue wxdl_call_remove(WXDLcall* _call, WXDLu64 _index)
{
	WXDLvalue lv = {0};
	if (_call == NULL || _index >= _call->argc)
		return lv;

	lv = _call->argv[_index];
	for (WXDLu64 i = _index; i < (_call->argc - 1); i++)
	{
		_call[i] = _call[i + 1];
	}

	return lv;
}

void wxdl_call_delete(WXDLcall* _call, WXDLu64 _index)
{
    WXDLvalue v = wxdl_call_remove(_call, _index);
    wxdl_free_value(&v);
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

WXDLvalue* wxdl_call_add_str(WXDLcall* _c, const WXDLchar* _v, WXDLstring_builder* _builder)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_str(_c, _c->argc, _v, _builder);
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

WXDLvalue* wxdl_call_add_value(WXDLcall* _c, WXDLvalue* _v)
{
    if (_c->argc == WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    return wxdl_call_set_value(_c, _c->argc, _v);
}

// set===========================================================================
WXDLvalue* wxdl_call_set_null(WXDLcall* _c, WXDLu64 _index)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, (WXDLu32)_index + 1 - _c->argc);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_NULL(_c->argv[_index]);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_bool(WXDLcall* _c, WXDLu64 _index, WXDLbool _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, (WXDLu32)_index + 1 - _c->argc);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_BOOL(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_int(WXDLcall* _c, WXDLu64 _index, WXDLint _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, (WXDLu32)_index + 1 - _c->argc);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_INT(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_u64(WXDLcall* _c, WXDLu64 _index, WXDLu64 _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, (WXDLu32)_index + 1 - _c->argc);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_UINT(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_float(WXDLcall* _c, WXDLu64 _index, WXDLfloat _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, 1);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_FLOAT(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_str(WXDLcall* _c, WXDLu64 _index, const WXDLchar* _v, WXDLstring_builder* _builder)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, 1);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_STR(_c->argv[_index], wxdl_build_string(_builder, _v));
    return &_c->argv[_index];
}


WXDLvalue* wxdl_call_set_str_ref(WXDLcall* _c, WXDLu64 _index, WXDLstring* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, 1);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_STR(_c->argv[_index], wxdl_string_ref(_v));
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_hash(WXDLcall* _c, WXDLu64 _index, WXDLhash* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, 1);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_DIC(_c->argv[_index], wxdl_hash_ref(_v));
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_arr(WXDLcall* _c, WXDLu64 _index, WXDLarr* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, 1);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_ARR(_c->argv[_index], wxdl_arr_ref(_v));
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_call(WXDLcall* _c, WXDLu64 _index, WXDLcall* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, 1);
        _c->argc = (WXDLu32)_index + 1;
    }
    WXDL_V_SET_CALL(_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_call_set_value(WXDLcall* _c, WXDLu64 _index, WXDLvalue* _v)
{
    if (_index >= WXDL_FUNC_MAX_PARAM_COUNT) return NULL;
    else if (_index >= _c->argc)
    {
        _wxdl_call_check_size(_c, 1);
        _c->argc = (WXDLu32)_index + 1;
    }
    wxdl_value_shallow_copy(&_c->argv[_index], _v);
    return &_c->argv[_index];
}

WXDLvalue* wxdl_param_at(WXDLcall* _c, WXDLu64 _index)
{
    if (_index >= _c->argc) return NULL;
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

WXDLerror wxdl_param_running(struct WXDLstate* _state, WXDLvalue* _v, WXDLvalue* _pv, WXDLthread_resoucre* _pres)
{
    WXDLvalue* param = _v;
    WXDLerror err = 0;
    if (WXDL_V_TYPE(*param) == WXDL_TYPE_CALL)
    {
        // 如果pid有效的话, 那就在pid里call层数计数加一
        WXDLthread_resoucre* tr = _pres;
        if (tr != NULL) tr->inner_layer += 1;

        WXDLcall* c = WXDL_V_CALL(*param);
        WXDL_V_TYPE(*param) = WXDL_TYPE_NULL;
        err = wxdl_call_ext(c, _state, _pv, _pres, WXDL_TRUE);
        if (tr != NULL) tr->inner_layer -= 1;
    }
    else
    {
        *_pv = *_v;
    }
    return err;
}

WXDLerror wxdl_param_bool(struct WXDLstate* _state, WXDLvalue* _v, WXDLbool* _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    WXDLvalue param;
    WXDLerror err = wxdl_param_running(_state, _v, &param, _pres);

    *_pv = wxdl_value_bool(&param);
    return 0;
}

WXDLerror wxdl_param_int(struct WXDLstate* _state, WXDLvalue* _v, WXDLint* _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    WXDLvalue param;
    WXDLerror err = wxdl_param_running(_state, _v, &param, _pres);

    *_pv = wxdl_value_int(&param);
    return 0;
}

WXDLerror wxdl_param_float(struct WXDLstate* _state, WXDLvalue* _v, WXDLfloat* _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    WXDLvalue param;
    WXDLerror err = wxdl_param_running(_state, _v, &param, _pres);

    *_pv = wxdl_value_float(&param);
    return 0;
}

WXDLerror wxdl_param_str(struct WXDLstate* _state, WXDLvalue* _v, const WXDLchar** _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    WXDLvalue param;
    WXDLerror err = wxdl_param_running(_state, _v, &param, _pres);

    *_pv = wxdl_value_str(&param);
    return 0;
}

WXDLerror wxdl_param_str_ref(struct WXDLstate* _state, WXDLvalue* _v, WXDLstring** _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    WXDLvalue param;
    WXDLerror err = wxdl_param_running(_state, _v, &param, _pres);

    *_pv = wxdl_value_str_ref(&param);
    return 0;
}

WXDLerror wxdl_param_hash(struct WXDLstate* _state, WXDLvalue* _v, WXDLhash** _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    WXDLvalue param;
    WXDLerror err = wxdl_param_running(_state, _v, &param, _pres);

    *_pv = wxdl_value_hash(&param);
    return 0;
}

WXDLerror wxdl_param_arr(struct WXDLstate* _state, WXDLvalue* _v, WXDLarr** _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    WXDLvalue param;
    WXDLerror err = wxdl_param_running(_state, _v, &param, _pres);

    *_pv = wxdl_value_arr(&param);
    return 0;
}

WXDLerror wxdl_param_call(struct WXDLstate* _state, WXDLvalue* _v, WXDLcall** _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    WXDLvalue param;
    WXDLerror err = wxdl_param_running(_state, _v, &param, _pres);

    *_pv = wxdl_value_call(&param);
    return 0;
}

WXDLerror wxdl_param_value(struct WXDLstate* _state, WXDLvalue* _v, WXDLvalue* _pv, WXDLthread_resoucre* _pres)
{
    if (_state == NULL || _v == NULL || _pv == NULL) return 1;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_CALL)
        return wxdl_param_running(_state, _v, _pv, _pres);
    else
    {
        wxdl_value_shallow_copy(_pv, _v);
        return 0;
    }
}

WXDLbool wxdl_param_check(const WXDLvalue* _argv, WXDLu32 _argc, const WXDLflag* flags, WXDLu32 flag_size)
{
    if (_argv == NULL || flags == NULL) return WXDL_FALSE;
    if (_argc < flag_size) return WXDL_FALSE;

    for (WXDLu32 i = 0; i < flag_size; i++)
    {
        if (!wxdl_is_type_convert(WXDL_V_TYPE(_argv[i]), flags[i])) return WXDL_FALSE;
    }

    return WXDL_TRUE;
}
