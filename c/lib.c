
#include "lib.h"
#include "define.h"
#include "parse.h"
#include "std.h"
#include "log.h"
#include "call.h"

WXDLerror _wxdl_lib_getvar(struct WXDLloader* loader, WXDLvalue* args, WXDLu32 arg_count, WXDLvalue* ret)
{
    const WXDLflag _param[] = {WXDL_TYPE_STR};
    if (!wxdl_param_check(args, arg_count, _param, 1))
    {
	    WXDL_ERR_LOG(loader, "func 'VAR' need a string param");
        return 1;
    }

    WXDLstate* s = wxdl_loader_state(loader);
    WXDLhash* n = wxdl_state_get_global(s);
	WXDLhash_node* n2 = wxdl_hash_path(n, WXDL_V_STR(args[0])->str, WXDL_V_STR(args[0])->len);
    wxdl_value_shallow_copy(ret, &(n2->v));
	return 0;
}

WXDLerror _wxdl_lib_if(struct WXDLloader* loader, WXDLvalue* args, WXDLu32 arg_count, WXDLvalue* ret)
{
    const WXDLflag _param[] = {WXDL_TYPE_BOOL};
    if (arg_count < 2)
    {
        WXDL_ERR_LOG(loader, "func 'IF' need two params");
        return 1;
    }
    else if (!wxdl_param_check(args, arg_count, _param, 1))
    {
	    WXDL_ERR_LOG(loader, "func 'IF' first param need a boolean value");
        return 1;
    }

    if (WXDL_V_BOOL(args[0]))
    {
        wxdl_value_shallow_copy(ret, wxdl_param_value(loader, &args[1]));
    }
    else
    {
        if (arg_count > 2)
        {
            wxdl_value_shallow_copy(ret, wxdl_param_value(loader, &args[2]));
        }
    }
	return 0;
}

void wxdl_init_std_lib(WXDLstate* _state)
{
    if (_state == NULL) return;
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_GET_GLOBAL_VAR, _wxdl_lib_getvar);
    wxdl_state_add_func(_state, WXDL_FUNC_NAME_IF, _wxdl_lib_if);

    return;
}
