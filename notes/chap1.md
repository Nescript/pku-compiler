# EBNF 拓展巴克斯范式
可以用来描述编程语言的语法，由若干的`A ::= B;` 组成

每一条都意味着当我们遇到 'A' 时，可以替换为 'B'

由此，`A` 被称为非终结符，因为可以推导出别的符号.

## 例子
考虑以下程序：
```c
int main() {
  // 忽略我的存在
  return 0;
}
```
其 EBNF 表示为：
```EBNF
CompUnit  ::= FuncDef;

FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";

Block     ::= "{" Stmt "}";
Stmt      ::= "return" Number ";";
Number    ::= INT_CONST;
```
由开始符号 `CompUnit` 开始推导，不断利用规则代换符号，可以得到：

```EBNF
"int" IDENT "(" ")" "{" "return" INT_CONST ";" "}"
```
除了非终结符, 在我们使用的 EBNF 中还会出现一些别的记法:
- `A | B` 表示可以推导出 `A`, 或者 `B`.
- `[...]` 表示方括号内包含的项可被重复 0 次或 1 次.
- `{...}` 表示花括号内包含的项可被重复 0 次或多次.

例如:
```EBNF
Params ::= Param {"," Param};
Param ::= Type IDENT;
Type ::= "int" | "long";
```
可以表示类似 `int param`, `int x, long y, int z` 这样的参数列表.

## 在 Flex 和 Bison 的实现

目前的理解是，我们通过 Flex 定义字符串匹配来捕获，在 Bison 中实现 EBNF 定义的逻辑

### Flex 在干什么 

1. 定义
```flex
/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

/* 块注释的状态 */
%x COMMENT
```
首先 Flex 包含定义捕获的部分。可以看到上述代码中我们定义了 `WhiteSpace` `LineComment` `Identifier` `Decimal` `Octal` `Hexadecimal` 这些 token 以及他们的匹配规则（正则表达式）

2. 捕获后的行为
```flex
{WhiteSpace}          { /* 忽略, 不做任何操作 */ }
{LineComment}         { /* 忽略, 不做任何操作 */ }

"int"                 { return INT; }
"return"              { return RETURN; }

{Identifier}          { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}             { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}               { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

"/*"                  { BEGIN(COMMENT); }

<COMMENT>"*/"         { BEGIN(INITIAL); }
<COMMENT>\n           { /* 忽略换行, 不做任何操作 */ }
<COMMENT>.            { /* 忽略普通字符, 不做任何操作 */ }
<COMMENT><<EOF>>      {
                          cerr << "Error: Unclosed comment" << endl;
                          yyterminate();
                      }
                
.                     { return yytext[0]; }
```
捕获定义的字符串后，就会触发这里的逻辑。可以看到针对每一种 token 类型都定义了相关的逻辑，比如我们忽略空白和换行，我们将八进制、十进制、十六进制数转换为整数，存储到 yylval 中，返回 `INT_CONST` 。

--- 



```koopa
fun @main(): i32 {  // main 函数的定义 
// fun 对应 FuncDef
// main 对应 IDENT
// FuncType 对应 i32
%entry:             // 入口基本块
// 对应 Block
  ret 0             // return 0
  // 对应 Stmt 和里面的 Number
}
```