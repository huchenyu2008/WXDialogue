# WXDialogue 库文档

## 概述

WXDialogue（简称 WXDL）是用于在文本中**内嵌数据**的工具库，通过特定语法实现文本与数据的结合，支持数据类型校验、全局/独立签名表、全局变量等核心能力。

---

## 基础语法示例

核心语法：`文本$签名{属性: 值}`，支持多种格式写法：

```lua
"abc"                    -- 一个普通字符串
"abc$$"                  -- 转义后仍为字符串，结果："abc$"
"abc$label{}"            -- 包含 label 签名表的内嵌数据块
"abc$'label'{}"          -- 签名表的另一种写法
"abc$label{a : 123, b.a : 321}"  -- 定义签名表属性
"abc$label{'a' : 123, 'b'.'a' : 321}"  -- 我们可以使用字符串ID！！！
```

---

## 支持的数据类型

支持基础数据类型及**自动类型转换**


| 类型    | 说明 & 示例                     | 可转换类型      |
| ------- | ------------------------------- | --------------- |
| null    | 空值                            | -               |
| boolean | 布尔值（true、false）           | number          |
| int     | 整数（123、0x123、0b1010）      | number、boolean |
| float   | 浮点数（123.、123.0、123.0123） | number、boolean |
| string  | 字符串                          | number、boolean |
| arr     | 数组（使用`[]` 包裹）           | -               |
| dic     | 字典/表（使用`{}` 包裹）        | -               |

> ⚠️ 警告：**非通行状态**下，无法获取或修改表数据

---

## 全局签名表

WXDL 维护全局状态，内置**全局签名表**（本质是字典），用于**校验数据属性和类型**，避免非法赋值。

### 示例

```lua
-- 假设 label 定义：font_size 属性，类型为 number
"abc$label{font_size : 2.0}"        -- 类型匹配，正常运行
"abc$label{font_size : true}"       -- 布尔值 ↔ 数值可转换，正常运行
"abc$label{font_size : '2.0'}"      -- 字符串无法赋值给数值，报错
"abc$label{_font_size_ : 2.0}"      -- 属性不存在，报错
```

### 通行符 `!`（跳过校验）

在签名/属性前加 `!`，**禁用校验机制**，无视属性是否存在、类型是否匹配：

```lua
-- label 嵌套结构：font.size (number)
"abc$label{font.size : 2.0}"                -- 正常校验通过
"abc$label{!font.max_size : 2.0}"           -- 单个属性跳过校验
"abc$!label{font_brother : 'hi'}"          -- 整个块跳过所有校验
```

---

## 独立签名表

相比全局签名表**更灵活**，支持**多重继承**（一个数据块绑定多个签名表）。

### 示例

```lua
-- 沿用全局 label 定义
-- 新增 control 表（visible：布尔值）
-- 新增 text_edit 表（text：布尔值）

"abc$label:control{visible : false}"       -- 单个独立签名表
"abc$label:control:text_edit{text : 'wow'}" -- 多重继承（两个签名表）
```

---

## 全局变量表

用于**复用常量数据**（如颜色、通用配置），避免重复手写硬编码。

### 示例

```lua
-- 变量表预定义：color.white = 0xFFFFFF，color.black = 0x000000
-- label 新增 color（number 类型）属性

"abc$label{color : color.white}"  -- 自动解析为 0xFFFFFF，简化赋值
"abc$label{color : .'color'.white}"  -- 为了避免歧义，所以字符串ID要在前面加上'.'
```

---

## 文本块结构

文本解析后会返回**文本结构**，由两种基础块组成：

1. **普通文本块（text）**
2. **数据块（data）**

### 示例

```lua
"abc$label:control{font.size:2.0}hahaha"
```

解析结果：**3 个文本块** → `text` + `data` + `text`

---

## Hash 表警告

1. 未实现**单一父节点**机制：多个父节点引用同一子节点时，若一个父节点释放，子节点会被销毁，其他父节点会变成**空引用**且无通知
2. 迭代 Hash 表前，必须检查解析结果是否为 `NULL`

---

## 错误提示

解析时开启 `logbuff` 且缓存充足，可获取**精准错误信息**，包含位置、错误代码、可视化提示。

### 示例

错误代码：

```lua
"$font{size : 1color.black}"  -- id 开头不能为数字
```

控制台输出：

```
WXDL Text Format Error, pos (1, 14):
|    $font{size : 1color.black}
|              ~~~^
|error info : Missing separator or incorrect ID format.
```

> 补充：单行文本超过 **160 字节**时，会自动裁剪远离错误位置的内容，保留核心报错信息。

---

## 作者信息

作者：luguoE
身份：一个石山的创造者
