
#include "state.h"
#include "define.h"
#include "type_define.h"
#include "hash.h"
#include "arr.h"
#include "std.h"
#include "string_builder.h"
#include <string.h>

#define WXDL_STATE_PID_EXT_FATOR 1.5f

// function===========================================================


// 初始化pid资源
void _wxdl_state_pid_init(WXDLthread_resoucre* tr)
{
    memset(tr, 0, sizeof(WXDLthread_resoucre));
}

// 释放pid资源(就是把里面的资源释放, 不是把tr放了)
void _wxdl_state_pid_free(WXDLthread_resoucre* tr)
{
    wxdl_free_value_arr(tr->R, WXDL_REG_SIZE);
    memset(tr, 0, sizeof(WXDLthread_resoucre));
}

// 扩容pid资源
void _wxdl_state_pid_ext(WXDLstate* _state)
{
    WXDLu32 new_size = (WXDLu32)(_state->pid_max_size * WXDL_STATE_PID_EXT_FATOR);
    WXDLu32 msize = _state->pid_max_size;
    if (new_size == 0) new_size = 4;
    _state->pres_fisrt_use = msize;
    wxdl_c_arr_ext((void**)&_state->pres, _state->pid_size, &_state->pid_max_size, new_size, sizeof(WXDLthread_resoucre));
    _state->pid_max_size = msize;
    wxdl_c_arr_ext((void**)&_state->pres_uses, _state->pid_size, &_state->pid_max_size, new_size, sizeof(WXDLu32));

    // 初始化pid
    for (WXDLu32 i = msize; i < new_size; i++)
    {
        _wxdl_state_pid_init(_state->pres + i);
        _state->pres_uses[i] = i + 1;
    }
}

WXDLstate* wxdl_new_state(WXDLstring_builder* _builder)
{
	WXDLstate* s = wxdl_malloc(sizeof(WXDLstate));
	if (_builder == NULL) _builder = wxdl_get_global_builder();
	s->builder = wxdl_builder_ref(_builder);
	s->global = wxdl_new_hash(32, _builder);
	s->signs = wxdl_new_hash(32, _builder);
	s->local_signs = wxdl_new_hash(32, _builder);
	s->funcs = wxdl_new_hash(32, _builder);

	s->logbuff.logbuff = NULL;
	s->logbuff.buffsize = 0;
	s->logbuff.bufflen = 0;

	s->pres = NULL;
	s->pres_uses = NULL;
	s->pres_fisrt_use = 0;
	s->pid_max_size = 0;
	s->pid_size = 0;
	_wxdl_state_pid_ext(s);

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

	for (WXDLu32 i = 0; i < _state->pid_max_size; i++)
        _wxdl_state_pid_free(_state->pres + i);
	wxdl_free(_state->pres);
	wxdl_free(_state->pres_uses);

	wxdl_free_hash(_state->global);
	wxdl_free_hash(_state->signs);
	wxdl_free_hash(_state->local_signs);
	wxdl_free_hash(_state->funcs);

	wxdl_free(_state);
}

WXDLlogbuff* wxdl_state_set_logbuff(WXDLstate* _state, WXDLchar* _logbuff, WXDLu64 _buffsize)
{
    if (_state == NULL) return NULL;
    _state->logbuff.logbuff = _logbuff;
    _state->logbuff.buffsize = _buffsize;
    _state->logbuff.bufflen = 0;
    return  &_state->logbuff;
}

WXDLlogbuff* wxdl_state_logbuff(WXDLstate* _state)
{
    if (_state == NULL) return NULL;
    return &_state->logbuff;
}

WXDLlogbuff* wxdl_state_clear_logbuff(WXDLstate* _state)
{
    if (_state == NULL) return NULL;
    _state->logbuff.bufflen = 0;
    return &_state->logbuff;
}

WXDLu32 wxdl_state_new_pid(WXDLstate* _state)
{
    if (_state == NULL) return WXDL_INVALID_PID;

    // 无可用pid, 扩容pid池
    if (_state->pres_fisrt_use >= _state->pid_max_size)
    {
        _wxdl_state_pid_ext(_state);
    }

    WXDLu32 pid = _state->pres_fisrt_use;
    _state->pres_fisrt_use = _state->pres_uses[pid];
    _state->pres_uses[pid] = WXDL_INVALID_PID;
    _state->pid_size += 1;
    _state->pres[pid].pid = pid;

    return pid;
}

void wxdl_state_free_pid(WXDLstate* _state, WXDLu32 _pid)
{
    if (_state == NULL) return;

    if (_pid < _state->pid_max_size)
    {
        if (_state->pres_uses[_pid] == WXDL_INVALID_PID)
        {
            _wxdl_state_pid_free(_state->pres + _pid);
            _state->pres_uses[_pid] = _state->pres_uses[_state->pres_fisrt_use];
            _state->pres_fisrt_use = _pid;
            _state->pid_size -= 1;
        }
    }
}

WXDLthread_resoucre* wxdl_state_pid(WXDLstate* _state, WXDLu32 _pid)
{
    if (_state == NULL) return NULL;
    if (_pid < _state->pid_max_size && _state->pres_uses[_pid] == WXDL_INVALID_PID)
    {
        return &_state->pres[_pid];
    }
    return NULL;
}

WXDLbool wxdl_state_pid_vaild(WXDLstate* _state, WXDLu32 _pid)
{
    if (_state == NULL) return WXDL_FALSE;
    if (_pid < _state->pid_max_size && _state->pres_uses[_pid] == WXDL_INVALID_PID)
    {
        return WXDL_TRUE;
    }
    return WXDL_FALSE;
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

WXDLhash_node* wxdl_state_add_func(WXDLstate* _state, const WXDLchar* _name, WXDLfunction func, WXDLbool _is_change_param)
{
	if (_state == NULL)
		return NULL;

	WXDLhash_node* n = wxdl_hash_add_ptr(_state->funcs, _name, func);
	n->v.flag |= WXDL_FLAG_FUNC_NOT_SET_PARAM;
	return n;
}

WXDLfunction_info wxdl_state_get_func(WXDLstate* _state, const WXDLchar* _name)
{
	if (_state == NULL)
		return (WXDLfunction_info){.func = NULL};

	WXDLhash_node* n = wxdl_hash_find(_state->funcs, _name);
	if (n == NULL)
        return (WXDLfunction_info){.func = NULL};
	return (WXDLfunction_info){.func = (WXDLfunction)n->v.data.p, .is_change_param = ((WXDL_NODE_FLAG(n) & WXDL_FLAG_FUNC_NOT_SET_PARAM) == WXDL_FLAG_FUNC_NOT_SET_PARAM)};
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
