
#include "../arr.h"
#include "../hash.h"
#include "../state.h"
#include "../parse.h"
#include <stdio.h>
#include <string.h>

int main()
{
	WXDLarr* a1 = wxdl_new_arr(20);

	for (int i = 0; i < 99; i++)
		wxdl_arr_add_int(a1, i);

	printf("arr size : %d - %d\n", (int)wxdl_arr_size(a1), (int)wxdl_arr_at(a1, 98)->data.i);

	WXDLhash* h1 = wxdl_new_hash(4);

	wxdl_hash_add_int(h1, u8"一", 8);
	wxdl_hash_add_int(h1, "尔", 6);
	wxdl_hash_add_int(h1, "散", 4);
	wxdl_hash_add_float(h1, "市", 12);
	wxdl_hash_add_int(h1, "雾", 32);
	wxdl_hash_add_int(h1, "哈啊啊啊啊", 1);
	wxdl_hash_add_int(h1, "size", 1);

	WXDLhash* global = wxdl_new_hash(16);
	wxdl_hash_add_int(global, "black", 1);

	WXDLhash* ext1 = wxdl_new_hash(12);
	wxdl_hash_add_int(ext1, "size", 12);

	WXDLstate* state = wxdl_new_state();
	wxdl_state_add_global(state, "color", global);
	wxdl_state_add_sign(state, "font", h1);
	wxdl_state_add_local_sign(state, "view", ext1);
	wxdl_state_add_local_sign(state, "view2", ext1);

	char log_buff[4096];
	memset(log_buff, 0, sizeof(log_buff));

	char text[] =
		u8"{a : 1}";
	printf("%s\n", text);
	WXDLhash* data = wxdl_parse_block(state, text, 0, log_buff, sizeof(log_buff) - 1, "current");
	printf("parse err : %d\n", data == NULL);
	//printf("%lld\n", wxdl_text_size(textdata));
	// 输出所以节点
	//WXDLtext_node* tnode = wxdl_text_head(textdata);

	printf("%s\n", log_buff);

	//while (tnode != NULL)
	{
		//printf("type[%d] : %s\n", tnode->type, tnode->text);
		//if (tnode->type == 1)
		{
			//WXDLiterator* ite = wxdl_hash_ite(tnode->data);
			WXDLiterator* ite = wxdl_hash_ite(data);
			do
			{
				WXDLvalue* v = wxdl_iterator_get(ite);
				if (v == NULL) break;

				printf("\tname[%s] type[%d]\n", wxdl_hash_ite_key(ite), v->type);
			} while (wxdl_iterator_next(ite));
		}
		//tnode = tnode->next;
	}

	wxdl_free_state(state);

	return 0;
}
