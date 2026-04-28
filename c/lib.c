
#include "lib.h"
#include "define.h"
#include "parse.h"
#include "state.h"
#include "std.h"
#include "log.h"
#include "call.h"
#include "string_builder.h"
#include <stdio.h>

WXDLerror _wxdl_lib_getvar(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    const WXDLflag _param[] = {WXDL_TYPE_STR};
    if (!wxdl_param_check(args, arg_count, _param, 1))
    {
	    WXDL_ERR_LOG(state, call, pres, "func 'VAR' need a string param");
        return 1;
    }

    WXDLhash* n = wxdl_state_get_global(state);
	WXDLhash_node* n2 = wxdl_hash_path(n, WXDL_V_STR(args[0])->str, WXDL_V_STR(args[0])->len);

	if (n2 == NULL)
	{
        WXDL_ERR_LOG(state, call, pres, "variable named '%s' not found", WXDL_V_STR(args[0])->str);
        return 1;
	}
    wxdl_value_shallow_copy(ret, &(n2->v));
	return 0;
}

WXDLerror _wxdl_lib_if(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    const WXDLflag _param[] = {WXDL_TYPE_BOOL};
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'IF' need two params");
        return 1;
    }
    WXDLbool b;
    wxdl_param_bool(state, &args[0], &b, pres);

    if (b)
    {
        wxdl_param_value(state, &args[1], ret, pres);
    }
    else
    {
        if (arg_count > 2)
        {
            wxdl_param_value(state, &args[2], ret, pres);
        }
    }
	return 0;
}

WXDLerror _wxdl_lib_block(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    for (WXDLu32 i = 0; i < arg_count; i++)
    {
        WXDLvalue* pv = &args[i];
        if (WXDL_V_TYPE(*pv) == WXDL_TYPE_CALL)
        {
            wxdl_free_value(ret);
            wxdl_call(WXDL_V_CALL(*pv), state, ret, pres);
        }
    }

	return 0;
}



WXDLerror _wxdl_lib_print(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    WXDLvalue v = {0};
    for (WXDLu32 i = 0; i < arg_count; i++)
    {
        wxdl_free_value(&v);
        WXDLvalue* pv = &args[i];
_WXDL_LIB_PRINT_GOTO:
        switch (WXDL_V_TYPE(*pv))
        {
        case WXDL_TYPE_NULL:
            printf("(null)");
            break;

        case WXDL_TYPE_BOOL:
            if (WXDL_V_BOOL(*pv))
                printf("true");
            else
                printf("false");
            break;

        case WXDL_TYPE_INT:
            printf("%lld", WXDL_V_INT(*pv));
            break;

        case WXDL_TYPE_FLOAT:
            printf("%lf", WXDL_V_FLOAT(*pv));
            break;

        case WXDL_TYPE_STR:
            printf("%s", WXDL_V_STR(*pv)->str);
            break;

        case WXDL_TYPE_DIC:
            printf("[hash : %p]", WXDL_V_PTR(*pv));
            break;

        case WXDL_TYPE_ARR:
            printf("[arr : %p]", WXDL_V_PTR(*pv));
            break;

        case WXDL_TYPE_CALL:
            {
                wxdl_param_value(state, pv, &v, pres);
                pv = &v;
                goto _WXDL_LIB_PRINT_GOTO;
            }
            break;


        default:
            printf("[adress : %p]", WXDL_V_PTR(*pv));
            break;
        }
    }
    wxdl_free_value(&v);
    printf("\n");

	return 0;
}

// string-----------------------------------------------------------------------------------------------------------------------------------------------------------
WXDLerror _wxdl_lib_strcmp(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'STRCMP' need two params");
        return 1;
    }

    WXDLstring* s1 = NULL;
    wxdl_param_str_ref(state, &args[0], &s1, pres);

    WXDLstring* s2 = NULL;
    wxdl_param_str_ref(state, &args[1], &s2, pres);

    if (s1 == NULL || s2 == NULL)
    {
	    WXDL_ERR_LOG(state, call, pres, "func 'STRCMP' first param need two string value");
        return 1;
    }

    WXDL_V_SET_BOOL(*ret, (wxdl_string_ref_cmp(s1, s2) == 0));
    return 0;
}

WXDLerror _wxdl_lib_strcat(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'STRCAT' need two params");
        return 1;
    }


    WXDLstring* s1 = NULL;
    wxdl_param_str_ref(state, &args[0], &s1, pres);

    WXDLstring* s2 = NULL;
    wxdl_param_str_ref(state, &args[1], &s2, pres);

    if (s1 == NULL || s2 == NULL)
    {
	    WXDL_ERR_LOG(state, call, pres, "func 'STRCAT' first param need two string value");
        return 1;
    }


    WXDLu64 l = s1->len + s2->len + 1;
    WXDLchar* s = wxdl_malloc(l);
    s[l] = 0;
    wxdl_copy(s, s1->str, s1->len);
    wxdl_copy(s + s1->len, s2->str, s2->len);
    WXDL_V_SET_STR(*ret, wxdl_try_gen_build_string(wxdl_state_get_string_builder(state), s));

    return 0;
}

WXDLerror _wxdl_lib_strlen(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 1)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'STRCAT' need one params");
        return 1;
    }

    WXDLstring* s = NULL;
    wxdl_param_str_ref(state, &args[0], &s, pres);

    if (s == NULL)
    {
	    WXDL_ERR_LOG(state, call, pres, "func 'STRCAT' first param need a string value");
        return 1;
    }

    WXDL_V_SET_INT(*ret, WXDL_V_STR(args[0])->len);

    return 0;
}

// number-----------------------------------------------------------------------------------------------------------------------------------------------------------

// 兼容数字运算的过程(除了条件判断)
#define WXDL_LIB_NUM_OP_PROCESS(args, ret, op) \
    WXDLvalue p1 = {0};\
    wxdl_param_value(state, &args[0], &p1, pres);\
    WXDLvalue p2 = {0};\
    wxdl_param_value(state, &args[1], &p2, pres);\
    union {\
        WXDLint i;\
        WXDLfloat f;\
    }v1, v2;\
    if (WXDL_V_TYPE(p2) == WXDL_TYPE_FLOAT)\
    {\
        v1.f = WXDL_V_FLOAT(p1);\
        if (WXDL_V_TYPE(p2) != WXDL_TYPE_FLOAT)\
            v2.f = (WXDLfloat)WXDL_V_INT(p2);\
        WXDL_V_SET_FLOAT(*ret, (v1.f op v2.f));\
    }\
    else\
    {\
        v1.i = WXDL_V_INT(p1);\
        if (WXDL_V_TYPE(p2) == WXDL_TYPE_FLOAT)\
            v2.i = (WXDLint)WXDL_V_FLOAT(p2);\
        WXDL_V_SET_INT(*ret, (v1.i op v2.i));\
    }

#define WXDL_LIB_NUM_INT_OP_PROCESS(args, ret, op) \
    WXDLvalue p1 = {0};\
    wxdl_param_value(state, &args[0], &p1, pres);\
    WXDLvalue p2 = {0};\
    wxdl_param_value(state, &args[1], &p2, pres);\
    union {\
        WXDLint i;\
        WXDLfloat f;\
    }v1, v2;\
    {\
        v1.i = WXDL_V_INT(p1);\
        if (WXDL_V_TYPE(p2) == WXDL_TYPE_FLOAT)\
            v2.i = (WXDLint)WXDL_V_FLOAT(p2);\
        WXDL_V_SET_INT(*ret, (v1.i op v2.i));\
    }

WXDLerror _wxdl_lib_add(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'ADD' need two params");
        return 1;
    }

    WXDL_LIB_NUM_OP_PROCESS(args, ret, +);
    return 0;
}

WXDLerror _wxdl_lib_sub(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'SUB' need two params");
        return 1;
    }

    WXDL_LIB_NUM_OP_PROCESS(args, ret, -);
    return 0;
}

WXDLerror _wxdl_lib_mul(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'MUL' need two params");
        return 1;
    }

    WXDL_LIB_NUM_OP_PROCESS(args, ret, *);
    return 0;
}


WXDLerror _wxdl_lib_div(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'DIV' need two params");
        return 1;
    }

    WXDL_LIB_NUM_OP_PROCESS(args, ret, /);
    return 0;
}

WXDLerror _wxdl_lib_mod(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'MOD' need two params");
        return 1;
    }

    WXDL_LIB_NUM_INT_OP_PROCESS(args, ret, %);
    return 0;
}

WXDLerror _wxdl_lib_and(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'AND' need two params");
        return 1;
    }

    WXDLbool p1;
    wxdl_param_bool(state, &args[0], &p1, pres);
    WXDLbool p2;
    wxdl_param_bool(state, &args[1], &p2, pres);
    WXDL_V_SET_BOOL(*ret, (p1 && p2));
    return 0;
}

WXDLerror _wxdl_lib_or(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'OR' need two params");
        return 1;
    }

    WXDLbool p1;
    wxdl_param_bool(state, &args[0], &p1, pres);
    WXDLbool p2;
    wxdl_param_bool(state, &args[1], &p2, pres);
    WXDL_V_SET_BOOL(*ret, (p1 || p2));
    return 0;
}

WXDLerror _wxdl_lib_band(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'BAND' need two params");
        return 1;
    }

    WXDLint p1;
    wxdl_param_int(state, &args[0], &p1, pres);
    WXDLint p2;
    wxdl_param_int(state, &args[1], &p2, pres);
    WXDL_V_SET_INT(*ret, (p1 & p2));
    return 0;
}

WXDLerror _wxdl_lib_bor(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'BOR' need two params");
        return 1;
    }

    WXDLint p1;
    wxdl_param_int(state, &args[0], &p1, pres);
    WXDLint p2;
    wxdl_param_int(state, &args[1], &p2, pres);
    WXDL_V_SET_INT(*ret, (p1 | p2));
    return 0;
}

WXDLerror _wxdl_lib_not(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLthread_resoucre* pres)
{
    if (arg_count < 1)
    {
        WXDL_ERR_LOG(state, call, pres, "func 'NOT' need one params");
        return 1;
    }

    WXDLint p1;
    wxdl_param_int(state, &args[0], &p1, pres);
    WXDL_V_SET_INT(*ret, (!p1));
    return 0;
}

void wxdl_init_std_lib(WXDLstate* _state)
{
    if (_state == NULL) return;
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_GET_GLOBAL_VAR, _wxdl_lib_getvar, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_IF, _wxdl_lib_if, WXDL_TRUE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_BLOCK, _wxdl_lib_block, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_PRINT, _wxdl_lib_print, WXDL_FALSE);

    // string
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_STR_CMP, _wxdl_lib_strcmp, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_STR_CAT, _wxdl_lib_strcat, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_STR_LEN, _wxdl_lib_strlen, WXDL_FALSE);

    // number
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_ADD, _wxdl_lib_add, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_SUB, _wxdl_lib_sub, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_MUL, _wxdl_lib_mul, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_DIV, _wxdl_lib_div, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_MOD, _wxdl_lib_mod, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_AND, _wxdl_lib_and, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_OR, _wxdl_lib_or, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_BAND, _wxdl_lib_band, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_BOR, _wxdl_lib_bor, WXDL_FALSE);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_NOT, _wxdl_lib_not, WXDL_FALSE);
    return;
}
