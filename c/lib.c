
#include "lib.h"
#include "parse.h"
#include "std.h"
#include "log.h"

WXDLerror _wxdl_lib_getvar(struct WXDLloader* loader, WXDLvalue* args, WXDLu64 arg_count, WXDLvalue* ret)
{
    if (arg_count < 1 || WXDL_V_TYPE(args[0]) != WXDL_TYPE_STR)
    {
	    WXDL_ERR_LOG(loader, "func 'getvar' need a string param");
        return 1;
    }

    WXDLstate* s = wxdl_loader_state(loader);
    WXDLhash* n = wxdl_state_get_global(s);
	WXDLhash_node* n2 = wxdl_hash_path(n, WXDL_V_STR(args[0])->str, WXDL_V_STR(args[0])->len);
    wxdl_value_copy(ret, &n2->v);
	return 0;
}

void wxdl_init_std_lib(WXDLstate* _state)
{
    if (_state == NULL) return;
    wxdl_state_add_func(_state, "getvar", _wxdl_lib_getvar);

    return;
}