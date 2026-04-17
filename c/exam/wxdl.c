
#include "../arr.h"
#include "../hash.h"
#include "../state.h"
#include "../parse.h"
#include "../std.h"
#include "../log.h"
#include "../buff.h"
#include "../string_builder.h"
#include "../call.h"
#include "../lib.h"
#include <stdio.h>
#include <string.h>

WXDLerror print_1(struct WXDLloader* loader, WXDLvalue* args, WXDLu64 arg_count, WXDLvalue* ret)
{
	WXDLint p1 = wxdl_param_int(loader, &args[0]);
	WXDL_ERR_LOG(loader, "Get Number %lld", p1);
	printf("hello!!!\n");
	WXDL_V_SET_INT(*ret, p1);
	printf("I'm call!!!\n");
	return 0;
}

WXDLerror getvar(struct WXDLloader* loader, WXDLvalue* args, WXDLu64 arg_count, WXDLvalue* ret)
{
    WXDLstate* s = wxdl_loader_state(loader);
    WXDLhash* n = wxdl_state_get_global(s);
	WXDL_V_SET_INT(*ret, 1);
	return 0;
}

int main()
{
	printf("start running\n");
	WXDLstring_builder* sb = wxdl_new_builder();
	printf("end create\n");
	WXDLarr* a1 = wxdl_new_arr(20, sb);
	printf("end create arr\n");
	for (int i = 0; i < 99; i++)
		wxdl_arr_add_int(a1, i);

	printf("arr size : %d - %d\n", (int)wxdl_arr_size(a1), (int)wxdl_arr_at(a1, 98)->data.i);

	WXDLhash* h1 = wxdl_new_hash(4, sb);
	WXDLhash* h2 = wxdl_new_hash(16, sb);
	wxdl_hash_add_int(h1, "size", 1);
	wxdl_hash_add_hash(h1, "data", h2);
	wxdl_hash_add_float(h2, "scale", 0);

	WXDLhash* global = wxdl_new_hash(16, sb);
	wxdl_hash_add_int(global, "black", 0);
	wxdl_hash_add_int(global, "white", 0xFFFFFF);

	WXDLhash* ext1 = wxdl_new_hash(12, sb);
	wxdl_hash_add_int(ext1, "size", 12);

	WXDLstate* state = wxdl_new_state(sb);
	wxdl_state_add_global(state, "color", global);
	wxdl_state_add_sign(state, "font", h1);
	wxdl_state_add_local_sign(state, "view", ext1);
	wxdl_state_add_local_sign(state, "view2", ext1);

	wxdl_state_add_func(state, "p1", print_1);
	wxdl_init_std_lib(state);
	char log_buff[4096];
	memset(log_buff, 0, sizeof(log_buff));

	printf("find %lld\n", wxdl_hash_path(wxdl_state_get_global(state), "color.black", 0)->v.data.i);

	char text[] =
		u8"{size : @p1(@getvar('color.white')), data : color.black}";
	printf("%s\n", text);



	WXDLblock* data = wxdl_parse_block(state, text, 0, WXDL_TRUE, log_buff, sizeof(log_buff) - 1, "current");


	//printf("running\n");
	//WXDLhash* ret = wxdl_block_running(state, data, log_buff, sizeof(log_buff) - 1);
	//printf("parse err : %d\n", data == NULL);
	//printf("%lld\n", wxdl_text_size(textdata));
	// 输出所以节点
	//WXDLtext_node* tnode = wxdl_text_head(data);

	printf("%s\n", log_buff);

	/*
	//while (tnode != NULL)
	{
		//printf("type[%d] : %s\n", tnode->type, tnode->text);
		//if (tnode->type == 1)
		{
			//WXDLiterator* ite = wxdl_hash_ite(tnode->data);
			WXDLiterator* ite = wxdl_hash_ite(ret);
			do
			{
				WXDLvalue* v = wxdl_iterator_get(ite);
				if (v == NULL) break;

				printf("\tname[%s] type[%d]\n", wxdl_hash_ite_key(ite)->str, v->type);
			} while (wxdl_iterator_next(ite));
		}
		//tnode = tnode->next;
	}*/

	WXDLbuff_set* bs = wxdl_new_buff_set();
	WXDLsaver* saver = wxdl_new_saver(data, NULL);

	wxdl_saver_output(saver, bs);

	printf("%s\n", wxdl_buff_get(wxdl_buff_set_at(bs, 0)));

	wxdl_free_block(data);
	wxdl_free_state(state);
	return 0;
}
