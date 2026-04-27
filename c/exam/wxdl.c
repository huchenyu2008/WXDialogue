
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

	wxdl_init_std_lib(state);
	char log_buff[4096] = {0};
	memset(log_buff, 0, sizeof(log_buff));

	printf("find %lld\n", wxdl_hash_path(wxdl_state_get_global(state), "color.black", 0)->v.data.i);

	char text[] =
		u8"{\nsize : @IF(@STRCMP('1', '12') ,@PRINT('你好如果输出我就是true', 634, ' ', true), @PRINT('你好如果输出我就是false', 777, ' ', false)),\n data : color.black,\n a.b : 1\n}\n";
	printf("%s\n", text);


	wxdl_state_set_logbuff(state, log_buff, sizeof(log_buff));
	WXDLu32 pid = wxdl_state_new_pid(state);

	WXDLblock* data = wxdl_parse_block(NULL, text, 0, WXDL_FALSE, "current", WXDL_INVAILD_PID, wxdl_state_logbuff(state));


	printf("err %d\n", data == NULL);
	//printf("running\n");
	//printf("parse err : %d\n", data == NULL);
	//printf("%lld\n", wxdl_text_size(textdata));
	// 输出所以节点
	//WXDLtext_node* tnode = wxdl_text_head(data);

	printf("%s\n", log_buff);

	if (data == NULL) return 1;
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

	printf("start saver\n");
	WXDLbuff_set* bs = wxdl_new_buff_set();
	WXDLsaver* saver = wxdl_new_saver(data, NULL);

	wxdl_saver_output(saver, bs);
	printf("saver  yes\n");

	printf("%s\n", wxdl_buff_get(wxdl_buff_set_at(bs, 0)));
	printf("free block\n");
	wxdl_free_block(data);
	printf("free state\n");
	wxdl_free_state(state);


	printf("program end\n");
	return 0;
}
