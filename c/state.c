
#include "state.h"
#include "hash.h"
#include "arr.h"
#include "std.h"
#include "string_builder.h"

typedef struct WXDLstate
{
	// 全局数据
	// 用于代替数据别名
	// 比如 : color.black 数值为 0x000000
	WXDLhash* global;

	// 签名表(用于核对签名是否有误)
	WXDLhash* signs;

	// 非全局签名表
	// 使用要声明(用于支持一些非全局标签的类型检查使用)
	WXDLhash* local_signs;

	// 全部函数
	WXDLhash* funcs;

	// 文本的节点
	// 注意, 每次解析都会将其释放掉
	// 如果需要长期使用, 请调用函数获得托管权
	WXDLtext_node* texts;

	WXDLtext_node* end_text;

	WXDLstring_builder* builder;
}WXDLstate;

typedef struct WXDLtext
{
	WXDLtext_node* texts;
	WXDLtext_node* end_text;


	WXDLu64 size;
}WXDLtext;

// function===========================================================
WXDLstate* wxdl_new_state(WXDLstring_builder* _builder)
{
	WXDLstate* s = wxdl_malloc(sizeof(WXDLstate));
	if (_builder == NULL) _builder = wxdl_get_global_builder();
	s->builder = _builder;
	s->global = wxdl_new_hash(32, _builder);
	s->signs = wxdl_new_hash(32, _builder);
	s->local_signs = wxdl_new_hash(32, _builder);
	s->funcs = wxdl_new_hash(32, _builder);

	// 添加全局变量
	wxdl_hash_add_null(s->global, "null");
	wxdl_hash_add_bool(s->global, "true", WXDL_TRUE);
	wxdl_hash_add_bool(s->global, "false", WXDL_FALSE);

	s->texts = NULL;
	s->end_text = NULL;
	return s;
}

void wxdl_free_state(WXDLstate* _state)
{
	if (_state == NULL)
		return;

	wxdl_free_hash(_state->global);
	wxdl_free_hash(_state->signs);
	wxdl_free_hash(_state->local_signs);
	wxdl_free_hash(_state->funcs);

	wxdl_free(_state);
}

WXDLhash* wxdl_state_gen_sign_table(WXDLstate* _state)
{
	if (_state == NULL)
		return NULL;

	return wxdl_hash_copy(_state->signs);
}

WXDLtext_node* wxdl_state_add_node(WXDLstate* _state, WXDLtext_node* _node)
{
	if (_state == NULL || _node == NULL)
		return NULL;


	if (_state->texts == NULL)
	{
		_state->texts = _node;
		_state->end_text = _node;
		_node->next = NULL;
	}
	else
	{
		_state->end_text->next = _node;
		_state->end_text = _node;
		_node->next = NULL;
	}

	return _node;
}

WXDLhash_node* wxdl_state_add_global(WXDLstate* _state, const WXDLchar* _name, WXDLhash* _hash)
{
	if (_state == NULL || _name == NULL || _hash == NULL)
		return NULL;

	WXDLhash_node* n = wxdl_hash_find(_state->global, _name);
	if (n != NULL)
	{
		// 避免关键词被修改
		if (n->v.type != WXDL_TYPE_DIC)
			return NULL;
		wxdl_set_node_hash(n, _hash);
		return n;
	}
	else
	{
		return wxdl_hash_add_hash(_state->global, _name, _hash);
	}
}

WXDLtext_node* wxdl_state_get_head(WXDLstate* _state)
{
	if (_state == NULL)
		return NULL;

	return _state->texts;
}

WXDLhash* wxdl_state_add_sign(WXDLstate* _state, const WXDLchar* _sign, WXDLhash* _table)
{
	if (_state == NULL || _sign == NULL || _table == NULL)
		return NULL;

	WXDLhash_node* h = wxdl_hash_find(_state->signs, _sign);
	if (h == NULL)
	{
		wxdl_hash_add_hash(_state->signs, _sign, _table);
	}
	else
	{
		wxdl_set_node_hash(h, _table);
	}

	return _table;
}

WXDLhash* wxdl_state_get_local_signs_table(WXDLstate* _state)
{
	if (_state == NULL) return NULL;
	else return _state->local_signs;
}

WXDLhash* wxdl_state_get_signs_table(WXDLstate* _state)
{
	if (_state == NULL) return NULL;
	else return _state->signs;
}

WXDLhash* wxdl_state_add_local_sign(WXDLstate* _state, const WXDLchar* _sign, WXDLhash* _table)
{
	if (_state == NULL || _sign == NULL || _table == NULL)
		return NULL;

	WXDLhash_node* h = wxdl_hash_find(_state->local_signs, _sign);
	if (h == NULL)
	{
		wxdl_hash_add_hash(_state->local_signs, _sign, _table);
	}
	else
	{
		wxdl_set_node_hash(h, _table);
	}

	return _table;
}

WXDLhash* wxdl_state_get_sign(WXDLstate* _state, const WXDLchar* _sign)
{
	if (_state == NULL || _sign == NULL)
		return NULL;

	WXDLhash_node* h = wxdl_hash_find(_state->signs, _sign);
	if (h == NULL) return NULL;

	return h->v.data.d;
}

WXDLhash* wxdl_state_get_local_sign(WXDLstate* _state, const WXDLchar* _sign)
{
	if (_state == NULL || _sign == NULL)
		return NULL;

	WXDLhash_node* h = wxdl_hash_find(_state->local_signs, _sign);
	if (h == NULL) return NULL;

	return h->v.data.d;
}

WXDLhash* wxdl_state_get_global(WXDLstate* _state)
{
	if (_state == NULL)
		return NULL;

	return _state->global;
}

WXDLhash_node* wxdl_state_add_func(WXDLstate* _state, const WXDLchar* _name, WXDLfunction func)
{
	if (_state == NULL)
		return NULL;

	return wxdl_hash_add_ptr(_state->funcs, _name, func);
}

WXDLfunction wxdl_state_get_func(WXDLstate* _state, const WXDLchar* _name)
{
	if (_state == NULL)
		return NULL;

	WXDLhash_node* n = wxdl_hash_find(_state->funcs, _name);
	if (n == NULL)
        return NULL;
	return (WXDLfunction)n->v.data.p;
}

WXDLhash* wxdl_state_get_func_table(WXDLstate* _state)
{
	if (_state == NULL)
		return NULL;

	return _state->funcs;
}

WXDLstring_builder* wxdl_state_get_string_builder(WXDLstate* _state)
{
	if (_state == NULL)
		return NULL;

	return _state->builder;
}

// WXDLtext===============================================================================
WXDLtext* wxdl_new_text()
{
	WXDLtext* t = wxdl_malloc(sizeof(WXDLtext));
	t->texts = NULL;
	t->end_text = NULL;
	t->size = 0;

	return t;
}

void wxdl_free_text(WXDLtext* _text)
{
	WXDLtext_node* n = _text->texts, * n2;
	while (n != NULL)
	{
		n2 = n->next;
		if (n->text != NULL)
			wxdl_free(n->text);
		if (n->data != NULL)
			wxdl_free_hash(n->data);
		if (n->use_local_tables != NULL)
			wxdl_free_arr(n->use_local_tables);
		wxdl_free(n);
		n = n2;
	}

	wxdl_free(_text);
}

WXDLtext_node* wxdl_text_add(WXDLtext* _text, WXDLtext_node* _node)
{
	if (_text == NULL || _node == NULL)
		return NULL;


	if (_text->texts == NULL)
	{
		_text->texts = _node;
		_text->end_text = _node;
		_node->next = NULL;
	}
	else
	{
		_text->end_text->next = _node;
		_text->end_text = _node;
		_node->next = NULL;
	}

	_text->size += 1;

	return _node;
}

WXDLtext_node* wxdl_text_head(WXDLtext* _text)
{
	if (_text == NULL)
		return NULL;
	else
		return _text->texts;
}

WXDLtext_node* wxdl_text_at(WXDLtext* _text, WXDLu64 _index)
{
	if (_text == NULL)
		return NULL;
	else if (_index > _text->size)
		return _text->end_text;


	WXDLtext_node* n = _text->texts;
	while (_index != 0 && n->next != NULL)
	{
		n = n->next;
		_index -= 1;
	}

	return n;
}

void wxdl_text_runing_at(WXDLtext* _text, WXDLu64 _index, WXDLhash* _vartable)
{
	if (_text == NULL)
		return;

	WXDLtext_node* n = wxdl_text_at(_text, _index);
	if (n == NULL || n->type != WXDL_TEXT_NODE_SIGN)
		return;

	WXDLhash_node* sign = wxdl_hash_find(_vartable, n->text);
	if (sign == NULL || sign->v.type != WXDL_TYPE_DIC)
		return;

	wxdl_hash_set_has_data(WXDL_NODE_DIC(sign), n->data);
}

WXDLu64 wxdl_text_size(WXDLtext* _text)
{
	if (_text == NULL)
		return 0;
	else
		return _text->size;
}
