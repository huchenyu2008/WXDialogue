
#include "lib.h"
#include "define.h"
#include "parse.h"
#include "state.h"
#include "std.h"
#include "log.h"
#include "call.h"
#include "string_builder.h"
#include <stdio.h>

WXDLerror _wxdl_lib_getvar(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLu32 pid)
{
    const WXDLflag _param[] = {WXDL_TYPE_STR};
    if (!wxdl_param_check(args, arg_count, _param, 1))
    {
	    WXDL_ERR_LOG(state, call, pid, "func 'VAR' need a string param");
        return 1;
    }

    WXDLhash* n = wxdl_state_get_global(state);
	WXDLhash_node* n2 = wxdl_hash_path(n, WXDL_V_STR(args[0])->str, WXDL_V_STR(args[0])->len);

	if (n2 == NULL)
	{
        WXDL_ERR_LOG(state, call, pid, "variable named '%s' not found", WXDL_V_STR(args[0])->str);
        return 1;
	}
    wxdl_value_shallow_copy(ret, &(n2->v));
	return 0;
}

WXDLerror _wxdl_lib_if(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLu32 pid)
{
    const WXDLflag _param[] = {WXDL_TYPE_BOOL};
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pid, "func 'IF' need two params");
        return 1;
    }
    WXDLbool b;
    wxdl_param_bool(state, &args[0], &b, pid);

    if (b)
    {
        wxdl_param_value(state, &args[1], ret, pid);
    }
    else
    {
        if (arg_count > 2)
        {
            wxdl_param_value(state, &args[2], ret, pid);
        }
    }
	return 0;
}

WXDLerror _wxdl_lib_block(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLu32 pid)
{
    for (WXDLu32 i = 0; i < arg_count; i++)
    {
        WXDLvalue* pv = &args[i];
        if (WXDL_V_TYPE(*pv) == WXDL_TYPE_CALL)
        {
            wxdl_free_value(ret);
            wxdl_call(WXDL_V_CALL(*pv), state, ret, pid);
        }
    }

	return 0;
}

WXDLerror _wxdl_lib_print(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLu32 pid)
{
    for (WXDLu32 i = 0; i < arg_count; i++)
    {
        WXDLvalue* pv = &args[i];
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
            printf("[call : %p]", WXDL_V_PTR(*pv));
            break;


        default:
            printf("[adress : %p]", WXDL_V_PTR(*pv));
            break;
        }
    }
    printf("\n");

	return 0;
}

// string-----------------------------------------------------------------------------------------------------------------------------------------------------------
WXDLerror _wxdl_lib_strcmp(struct WXDLstate* state, struct WXDLcall* call, struct WXDLvalue* args, WXDLu32 arg_count, struct WXDLvalue* ret, WXDLu32 pid)
{
    const WXDLflag _param[] = {WXDL_TYPE_STR, WXDL_TYPE_STR};
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(state, call, pid, "func 'STRCMP' need two params");
        return 1;
    }
    else if (!wxdl_param_check(args, arg_count, _param, 2))
    {
	    WXDL_ERR_LOG(state, call, pid, "func 'STRCMP' first param need two string value");
        return 1;
    }

    WXDL_V_SET_BOOL(*ret, (wxdl_string_ref_cmp(WXDL_V_STR(args[0]), WXDL_V_STR(args[1])) == 0));
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
    return;
}
