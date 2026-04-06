
#include "parse.h"
#include "std.h"
#include "arr.h"
#include "./log.h"
#include <ctype.h>
#include <stdio.h>

typedef struct WXDLloader
{
	// 使用的状态机
	WXDLstate* state;

	// 解析文本
	WXDLchar* text;
	WXDLu64 text_size;

	// 当前解析位置
	WXDLu64 ptr;

    // 当前行数
    WXDLu64 line;

    // 当前行数开始字节
    WXDLu64 line_start;

	// 日志输出缓存
	WXDLchar* log_buff;
	WXDLu64 log_buff_size;

    // 当前日志长度
    WXDLu64 log_len;

    // 当前使用标签
    WXDLhash* psign;

    // 使用的本地签名表
    WXDLarr* use_local_signs;

    // 文件名称
    const WXDLchar* where;
}WXDLloader;

// function============================================================================

// log======================================================

#define _WXDL_OUT_LOG(ploader, str, ...) _loader->log_len += (_loader->log_len >= _loader->log_buff_size) ? 0 : (WXDLu64)snprintf(_loader->log_buff + _loader->log_len, _loader->log_buff_size - _loader->log_len, str, ##__VA_ARGS__)

// 日志输出当前解析位置
void _wxdl_out_log_position(WXDLloader* _loader)
{
    if (_loader->log_len >= _loader->log_buff_size) return;

    _WXDL_OUT_LOG(_loader, "(%lld, %lld)", _loader->line, _loader->ptr - _loader->line_start);
}

//输出错误头
void _wxdl_out_log_error_head(WXDLloader* _loader)
{
    if (_loader->log_len >= _loader->log_buff_size) return;

    _WXDL_OUT_LOG(_loader, "wxdl error at (%lld, %lld) : ", _loader->line, _loader->ptr - _loader->line_start);
}


// char======================================================
WXDLu64 _wxdl_get_utf8_len(const WXDLchar* _text)
{
    unsigned char c = _text[0];

    if (c <= (unsigned char)0b01111111)
    {
        return 1;
    }
    else if (c <= (unsigned char)0b11011111)
    {
        return 2;
    }
    else if (c <= (unsigned char)0b11101111)
    {
        return 3;
    }
    else if (c <= (unsigned char)0b11110111)
    {
        return 4;
    }
    else if (c <= (unsigned char)0b11111011)
    {
        return 5;
    }
    else if (c <= (unsigned char)0b11111101)
    {
        return 6;
    }

    return -1;
}

WXDLbool _wxdl_is_little(const WXDLchar* _text)
{
    unsigned char c = _text[0];
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
        return WXDL_TRUE;
    else
        return WXDL_FALSE;
}

WXDLbool _wxdl_is_scope(const WXDLchar* _text)
{
    unsigned char c = _text[0];
    if (c == ' ' || c == '\n' || c == '\t')
        return WXDL_TRUE;
    else
        return WXDL_FALSE;
}

WXDLbool _wxdl_is_num(const WXDLchar* _text)
{
    unsigned char c = _text[0];
    if ('0' <= c && c <= '9')
        return WXDL_TRUE;
    else
        return WXDL_FALSE;
}

WXDLbool _wxdl_is_symbol(const WXDLchar* _text)
{
    if (ispunct((int)_text[0])) return WXDL_TRUE;
    else return WXDL_FALSE;
}

WXDLbool _wxdl_is_ascll(const WXDLchar* _text)
{
    char c = _text[0];
    if (_wxdl_get_utf8_len(&c) == 1 && c <= 127)
        return WXDL_TRUE;
    return WXDL_FALSE;
}

WXDLu64 _wxdl_is_enter(const WXDLchar* _text)
{
    if (_text[0] == '\n')
    {
        return 1;
    }
    else if (_text[0] == '\r' && _text[1] == '\n')
    {
        return 2;
    }
    else return 0;

}

// 是否结束
WXDLbool _wxdl_loader_eof(WXDLloader* loader)
{
    if (loader->ptr >= loader->text_size || loader->text[loader->ptr] == '\0') return WXDL_TRUE;
    else return WXDL_FALSE;
}

// 移动到下一个符号
// 返回移动字节
WXDLu64 _wxdl_loader_next(WXDLloader* loader)
{
    if (loader->ptr >= loader->text_size) return (WXDLu64)-1;

    WXDLu64 len = _wxdl_get_utf8_len(loader->text + loader->ptr);

    if (len == 1)
    {
        // 用于兼容Windows的换行\r\n
        WXDLu64 m = _wxdl_is_enter(&loader->text[loader->ptr]);
        if (m > 0)
        {
            loader->line += 1;
            loader->ptr += m;
            loader->line_start = loader->ptr;
            return m;
        }

        loader->ptr += 1;
        return 1;
    }
    else
    {
        loader->ptr += len;
        return len;
    }
}

// 跳过空格
void _wxdl_jump_space(WXDLloader* _loader)
{
    for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
    {
        WXDLchar c = _loader->text[_loader->ptr];
        if (c == ' ' || c == '\n' || c == '\t')
        {
            _wxdl_loader_next(_loader);
        }
        else break;
    }
}

// parse===================================================================================

WXDLerror _wxdl_parse_data(WXDLloader* _loader, WXDLvalue* _v, WXDLhash_node* _check_node);

// 解析数字
// _ptype : 1为浮点数, 0为整数
WXDLerror _wxdl_parse_number(WXDLloader* _loader, WXDLint* _pint, WXDLfloat* pfloat, WXDLflag* _ptype)
{
    // 第一个为0时有几个可能
    // 0 十进制 0
    // 0x 十六进制 1
    // 0b 二进制 2
    WXDLflag type = 0;

    WXDLu64 i = 0;
    WXDLint f = 0;

    // 浮点数位数(1, 10, 100, ...)
    WXDLint f_bit = 0;

    WXDLchar* pc = NULL;

    WXDLint ft = 1;

    // 判断是否为其它进制解析
    if (_loader->text[_loader->ptr] == '0')
    {
        if (_loader->text[_loader->ptr + 1] == 'x') type = 1, _loader->ptr += 2;
        else if (_loader->text[_loader->ptr + 1] == 'b') type = 2, _loader->ptr += 2;
    }
    else if (_loader->text[_loader->ptr] == '-')
    {
        ft = -1;
        _wxdl_loader_next(_loader);
    }

    switch (type)
    {
    // 十进制解析
    case 0:

        for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
        {
            pc = _loader->text + _loader->ptr;
            if (*pc == '.')
            {
                // 不能重复复小数点
                if (f_bit) return 1;
                else
                {
                    f_bit = 1;
                }
            }
            else if (_wxdl_is_num(pc))
            {
                if (f_bit)
                {
                    f *= 10;
                    f += (WXDLu64)(*pc - '0');
                    f_bit *= 10;
                }
                else
                {
                    i *= 10;
                    i += (WXDLu64)(*pc - '0');
                }
            }
            else
            {
                break;
            }

            _wxdl_loader_next(_loader);
        }

        if (f_bit) *_ptype = 1;
        else *_ptype = 0;

        break;
    // 十六进制解析
    case 1:
        for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
        {
            pc = _loader->text + _loader->ptr;
            if (_wxdl_is_num(pc))
            {
                i *= 16;
                i += (WXDLu64)(*pc - '0');
            }
            else if ('a' <= *pc && *pc <= 'z')
            {
                i *= 16;
                i += 11 + (WXDLu64)(*pc - 'a');
            }
            else if ('A' <= *pc && *pc <= 'Z')
            {
                i *= 16;
                i += 11 + (WXDLu64)(*pc - 'A');
            }
            else
            {
                break;
            }

            _wxdl_loader_next(_loader);
        }

        *_ptype = 0;

        break;
    // 二进制解析
    case 2:
        for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
        {
            pc = _loader->text + _loader->ptr;
            if (*pc == '0' || *pc == '1')
            {
                i *= 2;
                i += (WXDLu64)(*pc - '0');
            }
            else
            {
                break;
            }

            _wxdl_loader_next(_loader);
        }

        *_ptype = 0;

        break;
    default:
        return 1;
    }

    // 返回对应类型值
    if (f_bit)
    {
        *pfloat = ((WXDLfloat)(i)+(WXDLfloat)(f / f_bit)) * ft;
    }
    else
    {
        *_pint = i * ft;
    }

    return 0;
}

// 转移符号
char _wxdl_parse_esc_char(const char *str, size_t* index)
{
    *index += 1;
    switch (str[*index])
    {
    case 'n':
        return '\n';

    case 't':
        return '\t';

    case 'b':
        return '\b';

    case '\\':
        return '\\';

    case '"':
        return '"';

    case '\'':
        return '\'';

    case 'r':
        return '\r';
    default:
        *index -= 1;
        return str[*index];
    }
}

int _wxdl_check_esc_char(const char *str)
{
    switch (str[1])
    {
    case 'n':
    case 't':
    case 'b':
    case '\\':
    case '"':
    case '\'':
    case 'r':
        return 1;
    default:
        return 0;
    }
}

// 解析字符串
// 注 要跳过开头的endchar再调用
WXDLerror _wxdl_parse_string(WXDLloader* _loader, WXDLchar endchar, WXDLchar** _pstr)
{
    WXDLu64 str_start = _loader->ptr;
    WXDLu64 len = 0;
    for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
    {
        if (_loader->text[_loader->ptr] == endchar)
        {
            *_pstr = wxdl_malloc(sizeof(WXDLchar) * len + 1);
            WXDLchar* ps = (*_pstr);
            for (size_t i = str_start, i2 = 0; i < str_start + len; i++, i2++)
            {
                if (_loader->text[i] == '\\')

                    ps[i2] = _wxdl_parse_esc_char(_loader->text, &i);
                else
                    ps[i2] = _loader->text[i];
            }
            ps[len] = '\0';
            _wxdl_loader_next(_loader);
            return 0;
        }

        if (_loader->text[_loader->ptr] == '\\')
        {
            if (_wxdl_check_esc_char(_loader->text + _loader->ptr))
                _wxdl_loader_next(_loader);
        }
        len += _wxdl_loader_next(_loader);
    }


    return 1;
}

// 解析id
WXDLerror _wxdl_parse_id(WXDLloader* _loader, WXDLchar** _pstr)
{
    WXDLu64 str_start = _loader->ptr;

    // id开头不能为数字
    if (_wxdl_is_num(_loader->text + _loader->ptr))
    {
        return 1;
    }

    for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
    {
        const WXDLchar* s = _loader->text + _loader->ptr;
        if (!(*s == '_' || _wxdl_is_little(s) || _wxdl_is_num(s) || _wxdl_get_utf8_len(s) > 1))
        {
            // 没有id的话, 我直接报错
            if (str_start == _loader->ptr)
            {
                return 1;
            }
            WXDLu64 len = _loader->ptr - str_start;
            *_pstr = wxdl_malloc(sizeof(WXDLchar) * len + 1);
            (*_pstr)[len] = '\0';
            wxdl_copy(*_pstr, _loader->text + str_start, len);
            return 0;
        }
        _wxdl_loader_next(_loader);
    }


    return 1;
}


// 解析id或str, endchar为0为id, 不然为str
WXDLerror _wxdl_parse_name_and_id(WXDLloader* loader, WXDLchar endchar, WXDLchar** _pstr)
{
    if (endchar != '\'' && endchar != '"')
    {
        return _wxdl_parse_id(loader, _pstr);
    }
    else
    {
        _wxdl_loader_next(loader);
        return _wxdl_parse_string(loader, endchar, _pstr);
    }
}

// 解析数组
// 注 要跳过开头的'['再调用
WXDLerror _wxdl_parse_arr(WXDLloader* _loader, WXDLarr* _arr)
{
    // 是否需要逗号分隔
    WXDLbool is_split = WXDL_FALSE;
    WXDLerror err = 0;

    for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
    {
        _wxdl_jump_space(_loader);

        // 是否结束
        if (_loader->text[_loader->ptr] == ']')
        {
            _wxdl_loader_next(_loader);
            return 0;
        }

        // 每个元素间要有逗号分隔
        if (is_split == WXDL_TRUE)
        {
            if (_loader->text[_loader->ptr] == ',')
                is_split = WXDL_FALSE;
            else
                return 1;
            _wxdl_loader_next(_loader);
        }
        else
        {
            wxdl_arr_add_null(_arr);
            err = _wxdl_parse_data(_loader, wxdl_arr_at(_arr, wxdl_arr_size(_arr) - 1), NULL);
            if (err)
                return err;
            is_split = WXDL_TRUE;
        }
    }

    return 1;
}

// 加载hash路径
// 并获取指定变量
// _hash是获取的地方
// _complete_data 是否补全数据, 如果为false, 不存在键会报错
// _check_hash为检查表
// _check_hash -> (特殊实现: 当为_check_hash与_hash相同时，不会检查, 因为这种情况, 是通过取巧, 让这个函数可以用于获取检查表的数据)
// _check_find为检查表中对应pv的数据节点
// _ext_check_hashs扩展使用的检查表
WXDLerror _wxdl_parse_hash_path(WXDLloader* _loader, WXDLhash* _hash, WXDLvalue** pv, WXDLbool _complete_data, WXDLbool _check, WXDLhash* _check_hash, WXDLarr* _ext_check_hashs, WXDLhash_node** _check_find)
{
    WXDLerror err = 0;
    WXDLchar* pc = NULL;
    // 其被设为1时, 代表当前找到的地方不是hash类型
    WXDLhash* find_dic = NULL;
    WXDLvalue* v = NULL;
    // 是否需要间隔
    WXDLbool is_spilt = WXDL_FALSE;

    WXDLhash_node* node = NULL;

    if (_check_find != NULL)
        *_check_find = NULL;

    for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
    {
        _wxdl_jump_space(_loader);
        pc = _loader->text + _loader->ptr;

        if ((_wxdl_is_little(pc) || _wxdl_get_utf8_len(pc) > 1 || *pc == '_' || *pc == '\'' || *pc == '"'))
        {
            // 判断是否要分隔
            if (is_spilt)
            {
                return 1;
            }

            WXDLchar* str = NULL;
            err = _wxdl_parse_name_and_id(_loader, *pc, &str);
            if (err)
            {
                WXDL_LOG_WRITE(_loader, _loader->where, "Invalid path node name.");
                return err;
            }
            // 检查路径, 如果启用路径检查
            //
            if (_check && !(_hash == _check_hash && _ext_check_hashs == NULL))
            {
                WXDLhash_node* n = NULL;

                // 难不成还是想在数字里找东西
                if ((WXDLu64)find_dic == 1)
                {
                    WXDL_LOG_WRITE(_loader, _loader->where, "The node type in the path is not a table.");
                    wxdl_free(str);
                    return 1;
                }

                if (find_dic != NULL)
                {
                    n = wxdl_hash_find(find_dic, str);
                }
                else
                {
                    n = wxdl_hash_find(_check_hash, str);
                    // 没找到到其它检查表找
                    if (_ext_check_hashs != NULL)
                    {
                        for (WXDLu64 i = 0; i < wxdl_arr_size(_ext_check_hashs) && n == NULL; i++)
                        {
                            WXDLhash* h = wxdl_arr_at(_ext_check_hashs, i)->data.d;

                            n = wxdl_hash_find(h, str);
                        }
                    }
                }

                // 判断标记
                if (n == NULL)
                {
                    WXDL_LOG_WRITE(_loader, _loader->where, "Invalid path cannot locate the target node. Please check if the path is correct.");
                    wxdl_free(str);
                    return 1;
                }
                else
                {
                    if (n->v.type != WXDL_TYPE_DIC)
                        find_dic = (WXDLhash*)1;
                    else
                        find_dic = n->v.data.d;

                    if (_check_find != NULL)
                        *_check_find = n;
                }

            }

            // 获取数据
            if (v == NULL)
            {
                node = wxdl_hash_find(_hash, str);
                if (node == NULL)
                {
                    // 判断是否支持补全
                    if (_complete_data)
                        node = wxdl_hash_add_null(_hash, str);
                    else
                    {
                        WXDL_LOG_WRITE(_loader, _loader->where, "Failed to fetch data correctly.");
                        wxdl_free(str);
                        return 1;
                    }
                }
                v = &node->v;
            }
            else
            {
                // 代码强制转(笑
                if (v->type != WXDL_TYPE_DIC)
                {
                    wxdl_free_value(v);
                    v->data.d = wxdl_new_hash(16);
                    v->type = WXDL_TYPE_DIC;
                }
                node = wxdl_hash_find(v->data.d, str);
                // 没找到就自己加
                // 为什么呢? 因为前面有类型检查emmmm, 所以能保证有这个字典或数据
                if (node == NULL)
                {
                    // 判断是否支持补全
                    if (_complete_data)
                        node = wxdl_hash_add_null(v->data.d, str);
                    else
                    {
                        WXDL_LOG_WRITE(_loader, _loader->where, "Failed to fetch data correctly.");
                        wxdl_free(str);
                        return 1;
                    }
                }
                v = &node->v;
            }

            is_spilt = WXDL_TRUE;
            wxdl_free(str);
        }
        else if (*pc == '.')
        {
            is_spilt = WXDL_FALSE;
            _wxdl_loader_next(_loader);
        }
        else
        {
            break;
        }

    }
    
    // 如果有检查, 则判断找到的变量是否为表, 表不让过
    // 因为表在检查中不算可设置属性
    // 如果查找到的和检查表相同, 且没而外检查表, 则不用生效
    if (_check)
        if ((WXDLu64)find_dic != 1 && !(_hash == _check_hash && _ext_check_hashs == NULL))
        {
            WXDL_LOG_WRITE(_loader, _loader->where, "In check mode, table get/set is not allowed. Try adding '!' to the path.");
            return 1;
        }
    if (v == NULL) return 1;
    *pv = v;

    return 0;
}

WXDLerror _wxdl_parse_dic(WXDLloader* _loader, WXDLhash* _hash)
{
    // 是否需要逗号分隔
    WXDLbool is_split = WXDL_FALSE;
    WXDLerror err = 0;

    for (; _wxdl_loader_eof(_loader) != WXDL_TRUE;)
    {
        _wxdl_jump_space(_loader);

        // 是否结束
        if (_loader->text[_loader->ptr] == '}')
        {
            _wxdl_loader_next(_loader);
            return 0;
        }

        // 每个元素间要有逗号分隔
        if (is_split == WXDL_TRUE)
        {
            if (_loader->text[_loader->ptr] == ',')
                is_split = WXDL_FALSE;
            else
                return 1;
            _wxdl_loader_next(_loader);
        }
        else
        {
            WXDLvalue* v = NULL;
            // id : name
            err = _wxdl_parse_hash_path(_loader, _hash, &v, WXDL_TRUE, WXDL_FALSE, NULL, NULL, NULL);

            if (_loader->text[_loader->ptr] == ':' && err == 0)
            {
                _wxdl_loader_next(_loader);
                _wxdl_jump_space(_loader);
            }
            else
            {
                return 0;
            }

            err = _wxdl_parse_data(_loader, v, NULL);
            if (err)
                return err;
            is_split = WXDL_TRUE;
        }
    }

    return 1;
}


// 解析数据
// _check_node检查表中对应变量节点
WXDLerror _wxdl_parse_data(WXDLloader* _loader, WXDLvalue* _v, WXDLhash_node* _check_node)
{
    WXDLerror err = 0;
    // 根据开头字符判断
    WXDLchar head = _loader->text[_loader->ptr];
    WXDLchar* ps = _loader->text + _loader->ptr;

    // 将其中数据清空
    wxdl_free_value(_v);

    // 数字
    if (_wxdl_is_num(ps) || head == '-')
    {
        if (_check_node != NULL)
            if (!wxdl_is_type_convert(_check_node->v.type, WXDL_TYPE_FLOAT))
            {
                WXDL_LOG_WRITE(_loader, _loader->where, "Unable to convert from type '%s' to type '%s'", "number", wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        WXDLint i = 0;
        WXDLfloat f = 0;
        WXDLflag t = 0;
        err = _wxdl_parse_number(_loader, &i, &f, &t);
        if (err)
        {
            WXDL_LOG_WRITE(_loader, _loader->where, "Failed to parse number");
            return err;
        }

        if (t == 0)
        {
            _v->type = WXDL_TYPE_INT;
            _v->data.i = i;
        }
        else
        {
            _v->type = WXDL_TYPE_FLOAT;
            _v->data.f = f;
        }
    }
    // 变量
    else if (_wxdl_is_little(ps) || _wxdl_get_utf8_len(ps) > 1 || head == '_' || head == '.')
    {
        if (head == '.') _wxdl_loader_next(_loader);
        WXDLvalue* pv;

        err = _wxdl_parse_hash_path(_loader, wxdl_state_get_global(_loader->state), &pv, WXDL_FALSE, WXDL_FALSE, NULL, NULL, NULL);

        if (err)
        {
            return err;
        }

        if (_check_node != NULL)
            if (!wxdl_is_type_convert(_check_node->v.type, pv->type))
            {
                WXDL_LOG_WRITE(_loader, _loader->where, "Unable to convert from type '%s' to type '%s'", wxdl_get_type_str(pv->type), wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        wxdl_value_copy(_v, pv);
    }
    else if (head == '\'' || head == '\"')
    {
        if (_check_node != NULL)
            if (!wxdl_is_type_convert(_check_node->v.type, WXDL_TYPE_STR))
            {
                WXDL_LOG_WRITE(_loader, _loader->where, "Unable to convert from type '%s' to type '%s'", "string", wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        WXDLchar* str = NULL;

        _wxdl_loader_next(_loader);
        err = _wxdl_parse_string(_loader, head, &str);
        if (err)
        {
            wxdl_free(str);
            return err;
        }

        _v->type = WXDL_TYPE_STR;
        _v->data.s = str;
    }
    else if (head == '[')
    {
        if (_check_node != NULL)
            if (!wxdl_is_type_convert(_check_node->v.type, WXDL_TYPE_ARR))
            {
                WXDL_LOG_WRITE(_loader, _loader->where, "Unable to convert from type '%s' to type '%s'", "array", wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        WXDLarr* arr = wxdl_new_arr(16);

        _wxdl_loader_next(_loader);
        _wxdl_jump_space(_loader);
        err = _wxdl_parse_arr(_loader, arr);
        if (err)
        {
            wxdl_free_arr(arr);
            return err;
        }

        _v->type = WXDL_TYPE_ARR;
        _v->data.a = arr;
    }
    else if (head == '{')
    {
        if (_check_node != NULL)
            if (!wxdl_is_type_convert(_check_node->v.type, WXDL_TYPE_DIC))
            {
                WXDL_LOG_WRITE(_loader, _loader->where, "Unable to convert from type '%s' to type '%s'", "table", wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        WXDLhash* hash = wxdl_new_hash(16);

        _wxdl_loader_next(_loader);
        _wxdl_jump_space(_loader);
        err = _wxdl_parse_dic(_loader, hash);
        if (err)
        {
            wxdl_free_hash(hash);
            return err;
        }

        _v->type = WXDL_TYPE_DIC;
        _v->data.d = hash;
    }

    return 0;
}

// 解析标签块
// use_local_name是使用独立表的名称数组
WXDLerror _wxdl_parse_block(WXDLloader* loader, WXDLtext* text, WXDLhash* dic, WXDLchar* sign_name, WXDLbool sign_check, WXDLarr* use_local_name)
{
    WXDLerror err = 0;

    WXDLbool is_spilt = WXDL_FALSE;

    WXDLbool is_break = WXDL_FALSE;

    WXDLchar* spc = NULL;

    WXDLvalue* set_v = NULL;

    // 这里是变量解析
    for (; _wxdl_loader_eof(loader) != WXDL_TRUE;)
    {
        _wxdl_jump_space(loader);

        spc = loader->text + loader->ptr;
        if ((_wxdl_is_little(spc) || _wxdl_get_utf8_len(spc) > 1 || *spc == '_' || *spc == '!' || *spc == '\'' || *spc == '"'))
        {

            if (is_spilt)
            {
                WXDL_LOG_WRITE(loader, loader->where, "Missing separator or incorrect ID format.");
                return 1;
            }


            // [id / !id]  : data

            // 判断是或是额外数据
            if (loader->text[loader->ptr] == '!')
            {
                sign_check = WXDL_FALSE;
                _wxdl_loader_next(loader);
            }

            WXDLhash_node* fnode = NULL;
            err = _wxdl_parse_hash_path(loader, dic, &set_v, WXDL_TRUE, sign_check, loader->psign, loader->use_local_signs, &fnode);
            if (err)
            {
                return 1;
            }

            _wxdl_jump_space(loader);
            if (loader->text[loader->ptr] == ':')
            {
                _wxdl_loader_next(loader);
                _wxdl_jump_space(loader);

                err = _wxdl_parse_data(loader, set_v, fnode);
                if (err)
                {
                    return 1;
                }
            }
            else
            {

                return 1;
            }

            is_spilt = WXDL_TRUE;
        }
        else if (*spc == ',')
        {
            is_spilt = WXDL_FALSE;
            _wxdl_loader_next(loader);
            _wxdl_jump_space(loader);
        }
        else if (*spc == '}')
        {
            is_break = WXDL_TRUE;
            _wxdl_loader_next(loader);
            break;
        }
        else if (_wxdl_is_scope(loader->text + loader->ptr))
        {
            _wxdl_loader_next(loader);
        }
        else
        {
            // 报错!!!
            // 不正常退出
            break;
        }
    }

    // 判断是否是正常退出
    if (!is_break)
    {
        WXDL_LOG_WRITE(loader, loader->where, "Abnormal scope exit. Did you forget the scope terminator?");
        return 1;
    }
    else
    {
        if (text != NULL)
        {
            // 生成节点
            WXDLtext_node* tnode = (WXDLtext_node*)wxdl_malloc(sizeof(WXDLtext_node));
            tnode->type = WXDL_TEXT_NODE_SIGN;
            tnode->text = wxdl_new_str(sign_name);
            tnode->data = dic;
            tnode->use_local_tables = use_local_name;

            wxdl_text_add(text, tnode);
        }
    }
    return 0;
}

// 第一层解析
WXDLtext* _wxdl_parse(WXDLloader* loader)
{
    WXDLerror err = 0;
    // 文本开始位置
    WXDLu64 text_start = (WXDLu64)-1;
    WXDLu64 textsize = 0;

    WXDLtext* text = wxdl_new_text();

    for (; _wxdl_loader_eof(loader) != WXDL_TRUE;)
    {
        WXDLchar* pc = (loader->text + loader->ptr);

        // 文本忽视, 查找标签
        if (*pc == '$' && *(pc + 1) != '$')
        {
            // 将之前确认的文本创建节点(如果有的话)
            if (text_start != (WXDLu64)-1)
            {
                WXDLu64 text_len = textsize;
                WXDLtext_node* n = (WXDLtext_node*)wxdl_malloc(sizeof(WXDLtext_node));
                n->type = WXDL_TEXT_NODE_TEXT;
                n->data = NULL;
                n->text = wxdl_malloc(sizeof(WXDLchar) * text_len + 1);
                n->text[text_len] = 0;
                WXDLchar* pw = loader->text + text_start;
                WXDLu64 j = 0;
                for (WXDLu64 i = 0; i < loader->ptr - text_start; i++)
                {
                    n->text[j++] = pw[i];
                    if (pw[i] == '$')
                    {
                        i++;
                    }
                }

                wxdl_text_add(text, n);
                text_start = (WXDLu64)-1;
            }

            // 处理标签
            _wxdl_loader_next(loader);

            // 先获取标签名
            WXDLchar* sign = NULL;
            // 这个签名组是否是非格式话结构(不用检查)
            WXDLbool sign_check = WXDL_TRUE;
            _wxdl_jump_space(loader);

            // 为什么这里要一个判断呢, 因为 !sign 代表是不用检查的
            if (loader->text[loader->ptr] == '!')
            {
                sign_check = WXDL_FALSE;
                _wxdl_loader_next(loader);
                _wxdl_jump_space(loader);
            }

            err = _wxdl_parse_name_and_id(loader, loader->text[loader->ptr], &sign);
            if (err)
            {
                WXDL_LOG_WRITE(loader, loader->where, "Invalid global signature table name");
                wxdl_free_text(text);
                return NULL;
            }

            // 检查格式
            // $sign : local...{....}
            _wxdl_jump_space(loader);


            // 判断是否使用独立签名表
            WXDLarr* use_local = wxdl_new_arr(4);

            if (loader->text[loader->ptr] == ':')
            {
                while (loader->text[loader->ptr] == ':' && _wxdl_loader_eof(loader) == WXDL_FALSE)
                {
                    WXDLvalue* v;
                    _wxdl_loader_next(loader);
                    _wxdl_jump_space(loader);

                    WXDLu64 st = loader->ptr;
                    err = _wxdl_parse_hash_path(loader, wxdl_state_get_local_signs_table(loader->state), &v, WXDL_FALSE, WXDL_FALSE, NULL, NULL, NULL);

                    
                    // 找到对象必须为表
                    if (err || v->type != WXDL_TYPE_DIC)
                    {
                        WXDL_LOG_WRITE(loader, loader->where, "Invalid local signature table setting");
                        wxdl_free_text(text);
                        wxdl_free(sign);
                        wxdl_free_arr(use_local);
                        return NULL;
                    }
                    else
                    {
                        // 添加使用的独立签名表
                        wxdl_arr_add_hash_ref(loader->use_local_signs, v->data.d);

                        // 添加使用表的名称
                        WXDLu64 l = loader->ptr - st;
                        WXDLchar* name = wxdl_malloc(l + 1);
                        name[l] = '\0';
                        wxdl_copy(name, loader->text + st, l);
                        wxdl_arr_add_str_ref(use_local ,name);
                    }
                    // 找到':'
                    _wxdl_jump_space(loader);
                }
            }

            if (loader->text[loader->ptr] == '{')
            {
                _wxdl_loader_next(loader);
                loader->psign = wxdl_state_get_sign(loader->state ,sign);


                // 加载变量
                WXDLhash* dic = wxdl_new_hash(16);

                err = _wxdl_parse_block(loader, text, dic, sign, sign_check, use_local);
                if (err)
                {
                    wxdl_free_text(text);
                    wxdl_free(sign);
                    wxdl_free_hash(dic);
                    wxdl_free(use_local);
                    return NULL;
                }

                wxdl_free(sign);
                // 在清空使用的签名
                loader->psign = NULL;
                _wxdl_arr_clear_not_free(loader->use_local_signs);
            }
            else
            {
                WXDL_LOG_WRITE(loader, loader->where, "A block wrapped in '{}' is expected after the signature. Alternatively, did you mean to use '$$'?");
                wxdl_free(sign);
                wxdl_free(use_local);
                wxdl_free_text(text);
                return NULL;
            }
        }
        else
        {
            // 正常的文本

            if (text_start == (WXDLu64)-1)
                text_start = loader->ptr,
                textsize = 0;
            textsize++;

            if (*pc == '$')
            {
                _wxdl_loader_next(loader);
            }
            _wxdl_loader_next(loader);
        }
    }

    // 结束的时候再检查一遍, 将之前确认的文本创建节点(如果有的话)
    if (text_start != (WXDLu64)-1)
    {
        WXDLu64 text_len = loader->ptr - text_start;
        WXDLtext_node* n = (WXDLtext_node*)wxdl_malloc(sizeof(WXDLtext_node));
        n->type = WXDL_TEXT_NODE_TEXT;
        n->data = NULL;
        n->text = wxdl_malloc(sizeof(WXDLchar) * text_len + 1);
        n->text[text_len] = 0;
        n->use_local_tables = NULL;
        wxdl_copy(n->text, loader->text + text_start, text_len);

        wxdl_text_add(text, n);
        text_start = (WXDLu64)-1;
    }

    return text;
}


WXDLtext* wxdl_parse(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, WXDLchar* _log_buff, WXDLu64 _log_max_size, const WXDLchar* _where)
{
    if (_state == NULL || _text == NULL)
        return NULL;

    WXDLloader loader;

    loader.state = _state;
    loader.where = _where;
    if (loader.where == NULL)
        loader.where = "unknown";

    loader.text = _text;
    if (_text_size != 0)
        loader.text_size = _text_size;
    else
        loader.text_size = wxdl_str_len(_text);

    loader.ptr = 0;
    loader.line = 1;
    loader.line_start = 0;

    loader.log_buff = _log_buff;
    loader.log_buff_size = _log_max_size;
    loader.log_len = 0;

    loader.use_local_signs = wxdl_new_arr(8);

    WXDLtext* t = _wxdl_parse(&loader);

    wxdl_free_arr(loader.use_local_signs);
    return t;
}

WXDLhash* wxdl_parse_block(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, WXDLchar* _log_buff, WXDLu64 _log_max_size, const WXDLchar* _where)
{
    if (_state == NULL || _text == NULL)
        return NULL;

    WXDLloader loader;

    loader.state = _state;
    loader.where = _where;
    if (loader.where == NULL)
        loader.where = "unknown";

    loader.text = _text;
    if (_text_size != 0)
        loader.text_size = _text_size;
    else
        loader.text_size = wxdl_str_len(_text);

    loader.ptr = 0;
    loader.line = 1;
    loader.line_start = 0;

    loader.log_buff = _log_buff;
    loader.log_buff_size = _log_max_size;
    loader.log_len = 0;

    loader.use_local_signs = wxdl_new_arr(8);

    WXDLhash* dic = NULL;
    
    for (; !_wxdl_loader_eof(&loader);)
    {
        if (_wxdl_is_scope(loader.text + loader.ptr))
        {
            _wxdl_loader_next(&loader);
        }
        else if (loader.text[loader.ptr] == '{')
        {
            _wxdl_loader_next(&loader);
            dic = wxdl_new_hash(16);
            WXDLerror err = _wxdl_parse_block(&loader, NULL, dic, NULL, WXDL_FALSE, NULL);
            if (err)
            {
                wxdl_free_hash(dic);
                return NULL;
            }
            return dic;
        }
        else
        {
             WXDL_LOG_WRITE(&loader, loader.where, "A block wrapped in '{}' is expected after the signature.");
             return NULL;
        }
    }

    return NULL;
}