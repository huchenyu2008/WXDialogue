
#include "std.h"
#include "arr.h"
#include "define.h"
#include "hash.h"
#include "call.h"
#include "string_builder.h"
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#elif defined(__linux__) || defined(__ANDROID__) || defined(__FreeBSD__) || defined(__OpenBSD__)  || (defined(__APPLE__) && defined(__MACH__))
    #include <sys/syscall.h>
    #include <unistd.h>
#elif defined(__NetBSD__)
    #include <lwp.h>
#else
    #include <unistd.h>
#endif

WXDLu32 wxdl_get_thread_id()
{
    #if defined(_WIN32) || defined(_WIN64)
        return (WXDLu32)GetCurrentThreadId();
    #elif defined(__linux__) || defined(__ANDROID__)
        #if defined(SYS_gettid)
            return (WXDLu32)syscall(SYS_gettid);
        #elif defined(__NR_gettid)
            return (WXDLu32)syscall(__NR_gettid);
        #else
            return (WXDLu32)getpid();
        #endif
    #elif defined(__APPLE__) && defined(__MACH__)
        #if defined(SYS_thread_selfid)
            return (WXDLu32)syscall(SYS_thread_selfid);
         #else
            extern uint64_t __thread_selfid(void);
            return (WXDLu32)__thread_selfid();
        #endif
    #elif defined(__FreeBSD__)
        long tid;
        syscall(SYS_thr_self, &tid);
        return (WXDLu32)tid;
    #elif defined(__OpenBSD__)
        return (WXDLu32)syscall(SYS_getthrid);
    #elif defined(__NetBSD__)
        return (WXDLu32)_lwp_self();
    #else
        return (WXDLu32)getpid();
    #endif
}

WXDLptr wxdl_malloc(WXDLint _malloc_size)
{
	return (WXDLptr)malloc(_malloc_size);
}

void wxdl_value_copy_running(WXDLvalue* _v1, WXDLvalue* _v2, struct WXDLstate* _state, WXDLu32 _pid)
{
	if (_v1 == NULL || _v2 == NULL)
		return;

	wxdl_free_value(_v1);

	_v1->type = _v2->type;
	switch (_v2->type)
	{
	case WXDL_TYPE_STR:
		WXDL_V_SET_STR(*_v1, wxdl_string_ref(_v2->data.s));
		break;
	case WXDL_TYPE_DIC:
		WXDL_V_SET_DIC(*_v1, wxdl_hash_copy(_v2->data.d));
		break;
	case WXDL_TYPE_ARR:
		WXDL_V_SET_ARR(*_v1, wxdl_arr_copy(_v2->data.a));
		break;
	case WXDL_TYPE_CALL:
		if (_state == NULL)
		    WXDL_V_SET_CALL(*_v1, wxdl_call_copy(_v2->data.c));
		else
			wxdl_call_ext(_v2->data.c, _state, _v1, _pid, WXDL_TRUE);
		break;
	default:
		_v1->data.p = _v2->data.p;
		break;
	}

	_v1->flag = _v2->flag;
}

void wxdl_value_copy(WXDLvalue* _v1, WXDLvalue* _v2)
{
	wxdl_value_copy_running(_v1, _v2, NULL, WXDL_INVAILD_PID);
}

void wxdl_value_shallow_copy_running(WXDLvalue* _v1, WXDLvalue* _v2, struct WXDLstate* _state, WXDLu32 _pid)
{
	if (_v1 == NULL || _v2 == NULL)
		return;

	wxdl_free_value(_v1);

	_v1->type = _v2->type;
	switch (_v2->type)
	{
	case WXDL_TYPE_STR:
		WXDL_V_SET_STR(*_v1, wxdl_string_ref(_v2->data.s));
		break;
	case WXDL_TYPE_DIC:
		WXDL_V_SET_DIC(*_v1, wxdl_hash_ref(_v2->data.d));
		break;
	case WXDL_TYPE_ARR:
		WXDL_V_SET_ARR(*_v1, wxdl_arr_ref(_v2->data.a));
		break;
	case WXDL_TYPE_CALL:
		if (_state == NULL)
			WXDL_V_SET_CALL(*_v1, wxdl_call_ref(_v2->data.c));
		else
			wxdl_call(_v2->data.c, _state, _v1, _pid);
	default:
		_v1->data.p = _v2->data.p;
		break;
	}

	_v1->flag = _v2->flag;
}

void wxdl_value_shallow_copy(WXDLvalue* _v1, WXDLvalue* _v2)
{
	wxdl_value_shallow_copy_running(_v1, _v2, NULL, WXDL_INVAILD_PID);
}

void wxdl_free(WXDLptr _p)
{
	free(_p);
}

void wxdl_free_value(WXDLvalue* _pv)
{
	if (_pv == NULL)
		return;

	switch (_pv->type)
	{
	case WXDL_TYPE_DIC:
		wxdl_free_hash(_pv->data.d);
		break;
	case WXDL_TYPE_ARR:
		wxdl_free_arr(_pv->data.a);
		break;
	case WXDL_TYPE_STR:
		wxdl_free_string(_pv->data.s);
		break;
	case WXDL_TYPE_CALL:
		wxdl_free_call(_pv->data.c);
		break;
	}

	_pv->type = WXDL_TYPE_NULL;

	_pv->data.p = NULL;
}

void wxdl_free_value_arr(WXDLvalue* _pv, WXDLu64 count)
{
	for (WXDLu64 i = 0; i < count; i++)
		wxdl_free_value(&_pv[i]);
}

void wxdl_set(WXDLptr _buff, WXDLint32 _v, WXDLu64 _size)
{
	memset(_buff, _v, _size);
}

void wxdl_copy(WXDLptr _buff, const WXDLptr _copy, WXDLu64 _size)
{
	memcpy(_buff, _copy, _size);
}

WXDLchar* wxdl_new_str(const WXDLchar* _str)
{
	WXDLu64 l = wxdl_str_len(_str);
	WXDLchar* s = wxdl_malloc(l + 1);
	s[l] = 0;
	wxdl_copy(s, (const WXDLptr)_str, l);

	return s;
}

WXDLu64 wxdl_str_len(const WXDLchar* _str)
{
	return (WXDLu64)strlen(_str);
}

WXDLbool wxdl_str_cmp(const WXDLchar* _s1, const WXDLchar* _s2)
{
	return (WXDLbool)strcmp(_s1, _s2);
}

WXDLu64 wxdl_str_hashcode(const WXDLchar* _str)
{
	WXDLu64 hash = 0;
	WXDLu64 ptr = 0;
	char chr;
	const WXDLu64 f = 131;
	while ((chr = _str[ptr++]) && ptr < 64)
	{
		hash = hash * f + chr;
	}
	return hash;
}

WXDLbool wxdl_is_type_convert(WXDLflag _t1, WXDLflag _t2)
{
	switch (_t1)
	{
	case WXDL_TYPE_NULL:
		return WXDL_TRUE;

	case WXDL_TYPE_ARR:
	case WXDL_TYPE_DIC:
	case WXDL_TYPE_STR:
		if (_t1 == _t2) return WXDL_TRUE;
		else return WXDL_FALSE;

	case WXDL_TYPE_BOOL:
	case WXDL_TYPE_INT:
	case WXDL_TYPE_FLOAT:
		if (_t2 == WXDL_TYPE_BOOL || _t2 == WXDL_TYPE_INT || _t2 == WXDL_TYPE_FLOAT)
			return WXDL_TRUE;
		else
			return WXDL_FALSE;
	}

	return WXDL_FALSE;
}

WXDLbool wxdl_value_bool(WXDLvalue* _v)
{
    if (_v == NULL) return WXDL_FALSE;
    return (WXDLbool)_v->data.u;
}

WXDLint wxdl_value_int(WXDLvalue* _v)
{
    if (_v == NULL) return 0;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_FLOAT) return (WXDLint)_v->data.f;

    return _v->data.i;
}

WXDLu64 wxdl_value_u64(WXDLvalue* _v)
{
    if (_v == NULL) return 0;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_FLOAT) return (WXDLu64)_v->data.f;

    return _v->data.u;
}

WXDLfloat wxdl_value_float(WXDLvalue* _v)
{
    if (_v == NULL) return 0;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_INT) return (WXDLfloat)_v->data.i;

    return _v->data.f;
}

const WXDLchar* wxdl_value_str(WXDLvalue* _v)
{
    if (_v == NULL) return NULL;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_STR) return _v->data.s->str;
    return NULL;
}

WXDLstring* wxdl_value_str_ref(WXDLvalue* _v)
{
    if (_v == NULL) return NULL;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_STR) return _v->data.s;
    return NULL;
}

WXDLhash* wxdl_value_hash(WXDLvalue* _v)
{
    if (_v == NULL) return NULL;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_DIC) return _v->data.d;
    return NULL;
}

WXDLarr* wxdl_value_arr(WXDLvalue* _v)
{
    if (_v == NULL) return NULL;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_ARR) return _v->data.a;
    return NULL;
}

WXDLcall* wxdl_value_call(WXDLvalue* _v)
{
    if (_v == NULL) return NULL;
    if (WXDL_V_TYPE(*_v) == WXDL_TYPE_CALL) return _v->data.c;
    return NULL;
}
