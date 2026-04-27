
#include "parse.h"
#include "define.h"
#include "lib.h"
#include "type_define.h"
#include "hash.h"
#include "state.h"
#include "std.h"
#include "arr.h"
#include "call.h"
#include "string_builder.h"
#include "./log.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>


// function============================================================================

// log======================================================

#define _WXDL_OUT_LOG(bl, str, ...) bl->bufflen += (bl->bufflen >= bl->buffsize) ? 0 : (WXDLu64)snprintf(bl->logbuff + bl->bufflen, bl->buffsize - bl->bufflen, str, ##__VA_ARGS__)

// 日志输出当前解析位置
void _wxdl_out_log_position(WXDLloader* _loader)
{
    WXDLstate* state = wxdl_loader_state(_loader);
    WXDLlogbuff* bl = wxdl_state_logbuff(state);
    if (bl->bufflen >= bl->buffsize) return;

    _WXDL_OUT_LOG(bl, "(%lld, %lld)", _loader->line, _loader->ptr - _loader->line_start);
}

//输出错误头
void _wxdl_out_log_error_head(WXDLloader* _loader)
{
    WXDLstate* state = wxdl_loader_state(_loader);
    WXDLlogbuff* bl = wxdl_state_logbuff(state);
    if (bl->bufflen >= bl->buffsize) return;

    _WXDL_OUT_LOG(bl, "wxdl error at (%lld, %lld) : ", _loader->line, _loader->ptr - _loader->line_start);
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
    if (loader->ptr > loader->text_size || loader->text[loader->ptr] == '\0') return WXDL_TRUE;
    else return WXDL_FALSE;
}

WXDLu64 _wxdl_text_next(const WXDLchar* _text, WXDLu64 _len, WXDLu64* _off, WXDLu64* _nextline, WXDLu64* _line_start)
{
    if (*_off >= _len) return (WXDLu64)-1;

    WXDLu64 len = _wxdl_get_utf8_len(_text + *_off);

    if (len == 1)
    {
        // 用于兼容Windows的换行\r\n
        WXDLu64 m = _wxdl_is_enter(_text + *_off);
        if (m > 0)
        {
            if (_nextline != NULL)
                *_nextline += 1;
            if (_line_start != NULL)
                *_line_start = *_off + 1;
            *_off += m;
            return m;
        }

        *_off += 1;
        return 1;
    }
    else
    {
        *_off += len;
        return len;
    }
}

// 移动到下一个符号
// 返回移动字节
WXDLu64 _wxdl_loader_next(WXDLloader* loader)
{
    return _wxdl_text_next(loader->text, loader->text_size, &loader->ptr, &loader->line, &loader->line_start);
}

// 跳过空格
void _wxdl_text_jump_space(const WXDLchar* _text, WXDLu64 _len, WXDLu64* _off, WXDLu64* _nextline, WXDLu64* _line_start)
{
    for (; _text[*_off] != 0;)
    {
        WXDLchar c = _text[*_off];
        if (c == ' ' || c == '\n' || c == '\t')
        {
            _wxdl_text_next(_text, _len, _off, _nextline, _line_start);
        }
        else break;
    }
}

// 跳过空格
void _wxdl_jump_space(WXDLloader* _loader)
{
    _wxdl_text_jump_space(_loader->text, _loader->text_size, &_loader->ptr, &_loader->line, &_loader->line_start);
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

char _wxdl_get_esc_char(const char *str, size_t index)
{
    switch (str[index + 1])
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
    }
    return  0;
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

WXDLerror _wxdl_text_parse_string(const WXDLchar* _text, WXDLu64 _len, WXDLu64* _off, WXDLu64* _nextline, WXDLu64* _line_start, WXDLchar endchar, WXDLchar** _pstr, WXDLu64* _hashcode)
{
    WXDLu64 str_start = *_off;
    WXDLu64 len = 0;

    if (_hashcode) *_hashcode = 0;

    for (; _text[*_off] != 0;)
    {
        if (_text[*_off] == endchar)
        {
            if (_pstr != NULL)
            {
                *_pstr = wxdl_malloc(sizeof(WXDLchar) * len + 1);
                WXDLchar* ps = (*_pstr);
                for (size_t i = str_start, i2 = 0; i < str_start + len; i++, i2++)
                {
                    if (_text[i] == '\\')

                        ps[i2] = _wxdl_parse_esc_char(_text, &i);
                    else
                        ps[i2] = _text[i];
                }
                ps[len] = '\0';
            }
            _wxdl_text_next(_text, _len, _off, _nextline, _line_start);
            return 0;
        }

        if (_text[*_off] == '\\')
        {
            if (_wxdl_check_esc_char(_text + *_off))
            {
                if (_hashcode) *_hashcode = *_hashcode * 131 + _wxdl_get_esc_char(_text, *_off);
                _wxdl_text_next(_text, _len, _off, _nextline, _line_start);
            }
            else
                if (_hashcode) *_hashcode = *_hashcode * 131 + _text[*_off];
        }
        else
            if (_hashcode) *_hashcode = *_hashcode * 131 + _text[*_off];
        len += _wxdl_text_next(_text, _len, _off, _nextline, _line_start);
    }


    return 1;
}

// 解析字符串
// 注 要跳过开头的endchar再调用
WXDLerror _wxdl_parse_string(WXDLloader* _loader, WXDLchar endchar, WXDLchar** _pstr, WXDLu64* _hashcode)
{
    return _wxdl_text_parse_string(_loader->text, _loader->text_size, &_loader->ptr, &_loader->line, &_loader->line_start, endchar, _pstr, _hashcode);
}

// 解析id
WXDLerror _wxdl_text_parse_id(const WXDLchar* _text, WXDLu64 _len, WXDLu64* _off, WXDLu64* _nextline, WXDLu64* _line_start, WXDLchar** _pstr, WXDLu64* _hashcode)
{
    WXDLu64 str_start = *_off;

    // id开头不能为数字
    if (_wxdl_is_num(_text + *_off))
    {
        return 1;
    }

    if (_hashcode) *_hashcode = 0;

    for (; _text[*_off] != 0;)
    {
        const WXDLchar* s = _text + *_off;
        if (!(*s == '_' || _wxdl_is_little(s) || _wxdl_is_num(s) || _wxdl_get_utf8_len(s) > 1))
        {
_WXDL_PARSE_ID_END_MAKE_GOTO:
            // 没有id的话, 我直接报错
            if (str_start == *_off)
            {
                return 1;
            }
            WXDLu64 len = *_off - str_start;
            if (_pstr != NULL)
            {
                *_pstr = wxdl_malloc(sizeof(WXDLchar) * len + 1);
                (*_pstr)[len] = '\0';
                wxdl_copy(*_pstr, (const WXDLptr)(_text + str_start), len);
            }
            return 0;
        }

        if (_hashcode) *_hashcode = *_hashcode * 131 + _text[*_off];
        _wxdl_text_next(_text, _len, _off, _nextline, _line_start);
    }

    goto _WXDL_PARSE_ID_END_MAKE_GOTO;
}

// 解析id
WXDLerror _wxdl_parse_id(WXDLloader* _loader, WXDLchar** _pstr, WXDLu64* _hashcode)
{
    return _wxdl_text_parse_id(_loader->text, _loader->text_size, &_loader->ptr, &_loader->line, &_loader->line_start, _pstr, _hashcode);
}

// 解析id或str, endchar为0为id, 不然为str
WXDLerror _wxdl_text_parse_name_and_id(const WXDLchar* _text, WXDLu64 _len, WXDLu64* _off, WXDLu64* _nextline, WXDLu64* _line_start, WXDLchar endchar, WXDLchar** _pstr, WXDLu64* _hashcode)
{
    if (endchar != '\'' && endchar != '"')
    {
        return _wxdl_text_parse_id(_text, _len, _off, _nextline, _line_start, _pstr, _hashcode);
    }
    else
    {
        _wxdl_text_next(_text, _len, _off, _nextline, _line_start);
        return _wxdl_text_parse_string(_text, _len, _off, _nextline, _line_start, endchar, _pstr, _hashcode);
    }
}

// 解析id或str, endchar为0为id, 不然为str
WXDLerror _wxdl_parse_name_and_id(WXDLloader* _loader, WXDLchar endchar, WXDLchar** _pstr, WXDLu64* _hashcode)
{
    return _wxdl_text_parse_name_and_id(_loader->text, _loader->text_size, &_loader->ptr, &_loader->line, &_loader->line_start, endchar, _pstr, _hashcode);
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
            err = _wxdl_parse_data(_loader, wxdl_arr_add_null(_arr), NULL);
            if (err)
                return err;
            is_split = WXDL_TRUE;
        }
    }

    return 1;
}

// 迭代文本路径
// 返回是否结束
WXDLerror _wxdl_text_parse_path(const WXDLchar* _text, WXDLu64 _len, WXDLu64* _off, WXDLu64* _nextline, WXDLu64* _line_start, WXDLbool* _wait_spilt, WXDLchar** _path, WXDLu64* _hashcode, WXDLbool* _is_end)
{
    _wxdl_text_jump_space(_text, _len, _off, _nextline, _line_start);
    WXDLchar c = _text[*_off];
    if (c == '.')
    {
        if (_path) *_path = NULL;
        *_off += 1;
        *_wait_spilt = WXDL_FALSE;
    }
    else
    {
        if ((_wxdl_is_little(&c) || _wxdl_get_utf8_len(&c) > 1 || c == '_' || c == '\'' || c == '"'))
        {
            if (*_wait_spilt)
            {
                return 1;
            }
            *_wait_spilt = WXDL_TRUE;
            return _wxdl_text_parse_name_and_id(_text, _len, _off, _nextline, _line_start, c, _path, _hashcode);
        }
        else
        {
            if (_path) *_path = NULL;
            *_is_end = WXDL_TRUE;
            return 0;
        }
    }

    return 0;
}

WXDLerror _wxdl_parse_path(WXDLloader* _loader, WXDLbool* wait_spilt, WXDLchar** _path, WXDLu64* _hashcode, WXDLbool* _is_end)
{
    return _wxdl_text_parse_path(_loader->text, _loader->text_size, &_loader->ptr, &_loader->line, &_loader->line_start, wait_spilt, _path, _hashcode, _is_end);
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
            WXDLu64 hc = 0;
            err = _wxdl_parse_name_and_id(_loader, *pc, &str, &hc);
            if (err)
            {
                WXDL_LOG_WRITE(_loader, _loader->where->str, "Invalid path node name.");
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
                    WXDL_LOG_WRITE(_loader, _loader->where->str, "The node type in the path is not a table.");
                    wxdl_free(str);
                    return 1;
                }

                if (find_dic != NULL)
                {
                    n = _wxdl_hash_find_with_hashcode(find_dic, str, hc);
                }
                else
                {
                    n = _wxdl_hash_find_with_hashcode(_check_hash, str, hc);
                    // 没找到到其它检查表找
                    if (_ext_check_hashs != NULL)
                    {
                        for (WXDLu64 i = 0; i < wxdl_arr_size(_ext_check_hashs) && n == NULL; i++)
                        {
                            WXDLhash* h = wxdl_arr_at(_ext_check_hashs, i)->data.d;

                            n = _wxdl_hash_find_with_hashcode(h, str, hc);
                        }
                    }
                }

                // 判断标记
                if (n == NULL)
                {
                    WXDL_LOG_WRITE(_loader, _loader->where->str, "Invalid path cannot locate the target node. Please check if the path is correct.");
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
                node = _wxdl_hash_find_with_hashcode(_hash, str, hc);
                if (node == NULL)
                {
                    // 判断是否支持补全
                    if (_complete_data)
                        node = wxdl_hash_add_null(_hash, str);
                    else
                    {
                        WXDL_LOG_WRITE(_loader, _loader->where->str, "Failed to fetch data correctly.");
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
                    WXDL_V_SET_DIC(*v, wxdl_new_hash(16, _loader->builder));
                }
                node = _wxdl_hash_find_with_hashcode(v->data.d, str, hc);
                // 没找到就自己加
                // 为什么呢? 因为前面有类型检查emmmm, 所以能保证有这个字典或数据
                if (node == NULL)
                {
                    // 判断是否支持补全
                    if (_complete_data)
                        node = wxdl_hash_add_null(v->data.d, str);
                    else
                    {
                        WXDL_LOG_WRITE(_loader, _loader->where->str, "Failed to fetch data correctly.");
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
    //if (_check)
    //    if ((WXDLu64)find_dic != 1 && !(_hash == _check_hash && _ext_check_hashs == NULL))
    //    {
    //        WXDL_LOG_WRITE(_loader, _loader->where->str, "In check mode, table get/set is not allowed. Try adding '!' to the path.");
    //        return 1;
    //    }
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

// 调用函数
WXDLerror _wxdl_parse_call(WXDLloader* _loader, WXDLvalue* pv)
{
    _wxdl_jump_space(_loader);
    char head = _loader->text[_loader->ptr];
    char endchar = 0;

    WXDLu64 xpos = _loader->ptr;

    // 确认是否为字符串ID
    if (head == '\'' || head == '"')
        endchar = head;

    WXDLchar* id;
    WXDLu64 hc = 0;
    WXDLerror err = _wxdl_parse_name_and_id(_loader, endchar, &id, &hc);

    if (err)
    {
        WXDL_LOG_WRITE(_loader, _loader->where->str, "Invalid function name format");
        return 1;
    }

    WXDLfunction_info func = wxdl_state_get_func(_loader->state, id);
    if (func.func == NULL)
    {
        wxdl_free(id);
        WXDL_LOG_WRITE(_loader, _loader->where->str, "Unable to find a function called '%s'", id);
        return 1;
    }

    _wxdl_jump_space(_loader);
    if (_loader->text[_loader->ptr] != '(')
    {
        wxdl_free(id);
        WXDL_LOG_WRITE(_loader, _loader->where->str, "Invalid function call format, parameters must be wrapped with '()'");
        return 1;
    }
    _wxdl_loader_next(_loader);


    WXDLcall* call = wxdl_new_call(id, &func, NULL, 0, _loader->builder);
    call->line = (WXDLu32)_loader->line;
    call->xpos = (WXDLu32)(_loader->ptr - xpos);
    call->xpos_st = (WXDLu32)(xpos);
    call->where = wxdl_string_ref(_loader->where);
    WXDLbool wait = WXDL_FALSE;
    WXDLchar c = 0;
    for (; !_wxdl_loader_eof(_loader);)
    {
        _wxdl_jump_space(_loader);
        c = _loader->text[_loader->ptr];
        if (c == ')')
        {
            _wxdl_loader_next(_loader);
            // 如果未设置解析时游戏, 则把它转为call类型
            if (_loader->is_running_call && _loader->call_layer_count == 0)
            {
                // 这里是为了让报错指针指向函数名称位置
                WXDLu64 lp = _loader->ptr;
                _loader->ptr = xpos;
                err = wxdl_call_ext(call, _loader->state, pv, _loader->pid, WXDL_TRUE);
                _loader->ptr = lp;
                wxdl_free_call(call);
            }
            else
            {
                WXDL_V_SET_CALL(*pv, call);
            }
            wxdl_free(id);
            return err;
        }
        else if (c == ',')
        {
            wait = WXDL_FALSE;
            _wxdl_loader_next(_loader);
        }
        else
        {
            if (call->argc >= WXDL_FUNC_MAX_PARAM_COUNT)
            {
                wxdl_free_call(call);
                wxdl_free(id);
                WXDL_LOG_WRITE(_loader, _loader->where->str, "Parameter count exceeds supported limit (argc > WXDL_FUNC_MAX_PARAM_COUNT)");
                return 1;
            }
            else if (wait == WXDL_TRUE)
            {
                wxdl_free_call(call);
                wxdl_free(id);
                WXDL_LOG_WRITE(_loader, _loader->where->str, "Missing separator ',' between function call parameters");
                return 1;
            }

            _loader->call_layer_count += 1;
            err = _wxdl_parse_data(_loader, wxdl_call_add_null(call), NULL);
            _loader->call_layer_count -= 1;
            if (err)
            {
                wxdl_free_call(call);
                wxdl_free(id);
                return 1;
            }


            wait = WXDL_TRUE;
        }
    }

    wxdl_free_call(call);
    WXDL_LOG_WRITE(_loader, _loader->where->str, "Parse function error, you seem to be missing a terminator");

    wxdl_free(id);
    return 1;
}

// 解析数据
// _check_node检查表中对应变量节点
WXDLerror _wxdl_parse_data(WXDLloader* _loader, WXDLvalue* _v, WXDLhash_node* _check_node)
{
    WXDLerror _wxdl_parse_block(WXDLloader* loader, WXDLtext* text, WXDLhash* dic, WXDLchar* sign_name, WXDLbool sign_check, WXDLarr* use_local_name, WXDLhash*);
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
                WXDL_LOG_WRITE(_loader, _loader->where->str, "Unable to convert from type '%s' to type '%s'", "number", wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        WXDLint i = 0;
        WXDLfloat f = 0;
        WXDLflag t = 0;
        err = _wxdl_parse_number(_loader, &i, &f, &t);
        if (err)
        {
            WXDL_LOG_WRITE(_loader, _loader->where->str, "Failed to parse number");
            return err;
        }

        if (t == 0)
        {
            WXDL_V_SET_INT(*_v, i);
        }
        else
        {
            WXDL_V_SET_FLOAT(*_v, f);
        }
    }
    // 变量
    else if (_wxdl_is_little(ps) || _wxdl_get_utf8_len(ps) > 1 || head == '_' || head == '.')
    {
        if (head == '.') _wxdl_loader_next(_loader);
        WXDLvalue* pv;
        // is_running_call == false使用
        WXDLu64 path_st;
        WXDLu64 path_ed;
        WXDLbool wait = WXDL_FALSE;
        WXDLbool end = WXDL_FALSE;
        WXDLchar* path = NULL;

        // 如果设为运行延时, 则将变量变为函数调用
        if (_loader->is_running_call)
        {
            err = _wxdl_parse_hash_path(_loader, wxdl_state_get_global(_loader->state), &pv, WXDL_FALSE, WXDL_FALSE, NULL, NULL, NULL);



            if (err)
            {
                return err;
            }

            if (_check_node != NULL)
                if (!wxdl_is_type_convert(_check_node->v.type, pv->type))
                {
                    WXDL_LOG_WRITE(_loader, _loader->where->str, "Unable to convert from type '%s' to type '%s'", wxdl_get_type_str(pv->type), wxdl_get_type_str(_check_node->v.type));
                    return 1;
                }

             wxdl_value_shallow_copy(_v, pv);
        }
        else
        {
            _wxdl_jump_space(_loader);
            path_st = _loader->ptr;
            while (!end)
            {
                err = _wxdl_parse_path(_loader, &wait, NULL, NULL, &end);
                if (err)
                {
                    return err;
                }
                if (!end)
                    path_ed = _loader->ptr;
            }

            WXDLu64 l = path_ed - path_st;

            // 特殊状况如布尔值, null
            if (l == 4)
            {
                WXDLint32* i1 = (WXDLint32*)(_loader->text + path_st);
                if (*i1 == *(WXDLint32*)"null") {WXDL_V_SET_NULL(*_v); return 0;}
                else if (*i1 == *(WXDLint32*)"true") {WXDL_V_SET_BOOL(*_v, WXDL_TRUE); return 0;}
            }
            else if (l == 5)
            {
                WXDLint32* i1 = (WXDLint32*)(_loader->text + path_st);
                if (*i1 == *(WXDLint32*)"fals" && _loader->text[path_st + 4] == 'e') {WXDL_V_SET_BOOL(*_v, WXDL_FALSE); return 0;}
            }

            path = wxdl_malloc(sizeof(WXDLchar) * (l + 1));
            wxdl_copy(path, _loader->text + path_st, l); path[l] = 0;

            WXDL_V_SET_STR(*_v, wxdl_try_gen_build_string(_loader->builder, path));

            WXDLfunction_info fi;
            fi.func = _wxdl_lib_getvar;
            fi.is_change_param = WXDL_TRUE;
            WXDLcall* c = wxdl_new_call(WXDL_FUNC_NAME_GET_GLOBAL_VAR, &fi, _v, 1, _loader->builder);
            WXDL_V_SET_CALL(*_v, c);
        }
    }
    else if (head == '\'' || head == '\"')
    {
        if (_check_node != NULL)
            if (!wxdl_is_type_convert(_check_node->v.type, WXDL_TYPE_STR))
            {
                WXDL_LOG_WRITE(_loader, _loader->where->str, "Unable to convert from type '%s' to type '%s'", "string", wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        WXDLchar* str = NULL;
        WXDLu64 hc = 0;
        _wxdl_loader_next(_loader);
        err = _wxdl_parse_string(_loader, head, &str, &hc);
        if (err)
        {
            wxdl_free(str);
            return err;
        }

        WXDL_V_SET_STR(*_v, wxdl_try_gen_build_string_with_hashcode(_loader->builder, str, hc));
    }
    else if (head == '[')
    {
        if (_check_node != NULL)
            if (!wxdl_is_type_convert(_check_node->v.type, WXDL_TYPE_ARR))
            {
                WXDL_LOG_WRITE(_loader, _loader->where->str, "Unable to convert from type '%s' to type '%s'", "array", wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        WXDLarr* arr = wxdl_new_arr(16, _loader->builder);

        _wxdl_loader_next(_loader);
        _wxdl_jump_space(_loader);
        err = _wxdl_parse_arr(_loader, arr);
        if (err)
        {
            wxdl_free_arr(arr);
            return err;
        }


        WXDL_V_SET_ARR(*_v, arr);
    }
    else if (head == '{')
    {
        if (_check_node != NULL)
            if (!wxdl_is_type_convert(_check_node->v.type, WXDL_TYPE_DIC))
            {
                WXDL_LOG_WRITE(_loader, _loader->where->str, "Unable to convert from type '%s' to type '%s'", "table", wxdl_get_type_str(_check_node->v.type));
                return 1;
            }

        WXDLhash* hash = wxdl_new_hash(16, _loader->builder);

        _wxdl_loader_next(_loader);
        _wxdl_jump_space(_loader);

        // 用于支持下面的操作
        // 有 a : {b : 1}
        // 在未通行状态下
        // a.b : 1 可以通过
        // 如果没下面的代码
        // a : {b : 1} 就不行...
        if (_check_node != NULL)
        {
            err = _wxdl_parse_block(_loader, NULL, hash, NULL, WXDL_TRUE, NULL, WXDL_NODE_DIC(_check_node));
        }
        else
        {
            err = _wxdl_parse_dic(_loader, hash);
        }

        if (err)
        {
            wxdl_free_hash(hash);
            return err;
        }

        WXDL_V_SET_DIC(*_v, hash);
    }
    else if (head == '@')
    {
        _wxdl_loader_next(_loader);
        _wxdl_jump_space(_loader);
        WXDL_V_SET_NULL(*_v);

        err = _wxdl_parse_call(_loader, _v);
        if (err)
        {
            return 1;
        }
    }

    return 0;
}

// 解析标签块
// use_local_name是使用独立表的名称数组
// check_table是为了支持在未通行时 a : {b : 1}, 这种a.b的类型检查的
WXDLerror _wxdl_parse_block(WXDLloader* loader, WXDLtext* text, WXDLhash* dic, WXDLchar* sign_name, WXDLbool sign_check, WXDLarr* use_local_name, WXDLhash* check_table)
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
                WXDL_LOG_WRITE(loader, loader->where->str, "Missing separator or incorrect ID format.");
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
            if (check_table != NULL)
                err = _wxdl_parse_hash_path(loader, dic, &set_v, WXDL_TRUE, sign_check, check_table, NULL, &fnode);
            else
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
        WXDL_LOG_WRITE(loader, loader->where->str, "Abnormal scope exit. Did you forget the scope terminator?");
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

            err = _wxdl_parse_name_and_id(loader, loader->text[loader->ptr], &sign, NULL);
            if (err)
            {
                WXDL_LOG_WRITE(loader, loader->where->str, "Invalid global signature table name");
                wxdl_free_text(text);
                return NULL;
            }

            // 检查格式
            // $sign : local...{....}
            _wxdl_jump_space(loader);


            // 判断是否使用独立签名表
            WXDLarr* use_local = wxdl_new_arr(4, loader->builder);

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
                        WXDL_LOG_WRITE(loader, loader->where->str, "Invalid local signature table setting");
                        wxdl_free_text(text);
                        wxdl_free(sign);
                        wxdl_free_arr(use_local);
                        return NULL;
                    }
                    else
                    {
                        // 添加使用的独立签名表
                        wxdl_arr_add_hash(loader->use_local_signs, v->data.d);

                        // 添加使用表的名称
                        WXDLu64 l = loader->ptr - st;
                        WXDLchar* name = wxdl_malloc(l + 1);
                        name[l] = '\0';
                        wxdl_copy(name, loader->text + st, l);
                        wxdl_arr_add_str_ref(use_local, wxdl_try_gen_build_string(loader->builder, name));
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
                WXDLhash* dic = wxdl_new_hash(16, loader->builder);

                err = _wxdl_parse_block(loader, text, dic, sign, sign_check, use_local, NULL);
                if (err)
                {
                    wxdl_free_text(text);
                    wxdl_free(sign);
                    wxdl_free_hash(dic);
                    wxdl_free_arr(use_local);
                    return NULL;
                }

                wxdl_free(sign);
                // 在清空使用的签名
                loader->psign = NULL;
                _wxdl_arr_clear_not_free(loader->use_local_signs);
            }
            else
            {
                WXDL_LOG_WRITE(loader, loader->where->str, "A block wrapped in '{}' is expected after the signature. Alternatively, did you mean to use '$$'?");
                wxdl_free(sign);
                wxdl_free_arr(use_local);
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


WXDLtext* wxdl_parse(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, const WXDLchar* _where, WXDLu32 _pid, WXDLlogbuff* _logbuff)
{
    if (_state == NULL || _text == NULL)
        return NULL;

    WXDLloader loader = {0};

    // 用于恢复pid内容
    WXDLchar* ltext = NULL;
    WXDLu64 ltext_size = 0;

    loader.state = _state;
    loader.is_running_call = WXDL_TRUE;
    loader.where = wxdl_build_string(loader.builder, _where);
    if (loader.where == NULL)
        loader.where = wxdl_build_string(loader.builder, "unknown");

    loader.text = _text;
    if (_text_size != 0)
        loader.text_size = _text_size;
    else
        loader.text_size = wxdl_str_len(_text);

    loader.ptr = 0;
    loader.line = 1;
    loader.line_start = 0;

    loader.logbuff = _logbuff;

    loader.call_layer_count = 0;
    loader.use_local_signs = wxdl_new_arr(8, loader.builder);

    if (!wxdl_state_pid_vaild(_state, _pid)) _pid = WXDL_INVAILD_PID;
    loader.pid = _pid;

    WXDLthread_resoucre* rs = wxdl_state_pid(loader.state, loader.pid);
    if (rs != NULL)
    {
        ltext = rs->text;
        ltext_size = rs->text_size;
        rs->text = _text;
        rs->text_size = _text_size;
    }

    WXDLtext* t = _wxdl_parse(&loader);

    if (rs != NULL)
    {
        rs->text = ltext;
        rs->text_size = ltext_size;
    }
    wxdl_free_arr(loader.use_local_signs);
    return t;
}

WXDLblock* wxdl_parse_block(WXDLstate* _state, WXDLchar* _text, WXDLu64 _text_size, WXDLbool func_running, const WXDLchar* _where, WXDLu32 _pid, WXDLlogbuff* _logbuff)
{
    if (_state == NULL || _text == NULL)
        return NULL;

    WXDLloader loader = {0};

    // 用于恢复pid内容
    WXDLchar* ltext = NULL;
    WXDLu64 ltext_size = 0;

    loader.state = _state;
    loader.builder = wxdl_state_get_string_builder(_state);
    loader.is_running_call = func_running;
    loader.where = wxdl_build_string(loader.builder, _where);
    if (loader.where == NULL)
        loader.where = wxdl_build_string(loader.builder, "unknown");

    loader.text = _text;
    if (_text_size != 0)
        loader.text_size = _text_size;
    else
        loader.text_size = wxdl_str_len(_text);

    loader.ptr = 0;
    loader.line = 1;
    loader.line_start = 0;

    loader.logbuff = _logbuff;

    loader.call_layer_count = 0;

    if (!wxdl_state_pid_vaild(_state, _pid)) _pid = WXDL_INVAILD_PID;
    loader.pid = _pid;

    WXDLthread_resoucre* rs = wxdl_state_pid(loader.state, loader.pid);
    if (rs != NULL)
    {
        ltext = rs->text;
        ltext_size = rs->text_size;
        rs->text = _text;
        rs->text_size = _text_size;
    }

    WXDLblock* bl = NULL;
    for (; !_wxdl_loader_eof(&loader);)
    {
        if (_wxdl_is_scope(loader.text + loader.ptr))
        {
            _wxdl_loader_next(&loader);
        }
        else if (loader.text[loader.ptr] == '{')
        {
            _wxdl_loader_next(&loader);
            bl = wxdl_new_block(NULL, loader.builder);
            WXDLerror err = _wxdl_parse_block(&loader, NULL, bl->data, NULL, WXDL_FALSE, NULL, NULL);
            if (err)
            {
                wxdl_free_block(bl);
                goto _WXDL_BLOCK_PARSE_ERROR_GOTO;
            }

            if (rs != NULL)
            {
                rs->text = ltext;
                rs->text_size = ltext_size;
            }

            wxdl_free_string(loader.where);
            return bl;
        }
        else
        {
             WXDL_LOG_WRITE(&loader, loader.where->str, "A block wrapped in '{}' is expected after the signature.");
             goto _WXDL_BLOCK_PARSE_ERROR_GOTO;
        }
    }
_WXDL_BLOCK_PARSE_ERROR_GOTO:

    if (rs != NULL)
    {
        rs->text = ltext;
        rs->text_size = ltext_size;
    }
    wxdl_free_string(loader.where);
    return NULL;
}

WXDLhash* wxdl_block_running(WXDLstate* _state, WXDLblock* _block, WXDLu32 _pid)
{
    if (!wxdl_state_pid_vaild(_state, _pid)) _pid = WXDL_INVAILD_PID;

    return wxdl_hash_copy_running(_block->data, _state, _pid);
}

WXDLblock* wxdl_new_block(WXDLhash* _refhash, WXDLstring_builder* _builder)
{
    WXDLblock* b = (WXDLblock*)wxdl_malloc(sizeof(WXDLblock));
    if (_refhash == NULL)
    {
        b->data = wxdl_new_hash(32, _builder);
        b->is_ref = WXDL_FALSE;
    }
    else
    {
        b->data = _refhash;
        b->is_ref = WXDL_TRUE;
    }
    return b;
}

WXDLhash* wxdl_block_set_data(WXDLblock* _block, WXDLhash* _refhash)
{
    if (_block == NULL || _refhash == NULL) return NULL;
    wxdl_free_hash(_block->data);
    _block->data = wxdl_hash_ref(_refhash);
    return _block->data;
}

WXDLhash* wxdl_block_data(WXDLblock* _block)
{
    if (_block == NULL) return NULL;
    return _block->data;
}

void wxdl_free_block(WXDLblock* _block)
{
    if (_block != NULL)
    {
        if (_block->data != NULL && _block->is_ref == WXDL_FALSE) { wxdl_free_hash(_block->data);}
        wxdl_free(_block);
    }
}

WXDLhash_node* wxdl_hash_path(WXDLhash* _hash, const WXDLchar* _path, WXDLu64 _len)
{
    WXDLu64 off = 0;
    WXDLbool wait = WXDL_FALSE;
    WXDLchar* path;
    WXDLbool is_end = WXDL_FALSE;
    WXDLhash_node* n = NULL;
    WXDLerror err = 0;
    WXDLu64 hc = 0;
    if (_len == 0) _len = wxdl_str_len(_path);

    while (!is_end)
    {
        err = _wxdl_text_parse_path(_path, _len, &off, NULL, NULL, &wait, &path, &hc, &is_end);
        if (err)
        {
            return NULL;
        }
        else if (path != NULL)
        {
            if (n == NULL)
            {
                n = wxdl_hash_find(_hash, path);
            }
            else
            {
                if (WXDL_NODE_TYPE(n) != WXDL_TYPE_DIC)
                {
                    wxdl_free(path);
                    return NULL;
                }
                n = wxdl_hash_find(WXDL_NODE_DIC(n), path);
            }

            wxdl_free(path);
            if (n == NULL) return NULL;
        }
    }

    return n;
}

// loader===============================================================================================

WXDLstate* wxdl_loader_state(struct WXDLloader* _loader)
{
    if (_loader == NULL) return NULL;
    return _loader->state;
}

WXDLptr wxdl_loader_userdata(struct WXDLloader* _loader)
{
    if (_loader == NULL) return NULL;
    return _loader->userdata;
}

WXDLptr wxdl_set_loader_userdata(struct WXDLloader* _loader, WXDLptr _ptr)
{
    if (_loader == NULL) return NULL;
    _loader->userdata = _ptr;
    return _loader->userdata;
}

WXDLstring_builder* wxdl_set_loader_builder(struct WXDLloader* _loader)
{
    if (_loader == NULL) return NULL;
    return _loader->builder;
}
