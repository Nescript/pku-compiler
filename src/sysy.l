// 这里写一些选项, 可以控制 Flex/Bison 的某些行为
%option noinput
%option noyywarp
%option nounput

%{

#include <cstdlib>
#include <cstdio>
#include "sysy.tab.hpp"

using namespace std;
// 这里写一些全局的代码
// 因为最后要生成 C/C++ 文件, 实现主要逻辑的部分都是用 C/C++ 写的
// 难免会用到头文件, 所以通常头文件和一些全局声明/定义写在这里

%}

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

// 这里写一些 Flex/Bison 相关的定义
// 对于 Flex, 这里可以定义某个符号对应的正则表达式
// 对于 Bison, 这里可以定义终结符/非终结符的类型

%%

{WhiteSpace}          { /* 忽略, 不做任何操作 */ }
{LineComment}         { /* 忽略, 不做任何操作 */ }

"int"                 { return INT; }
"return"              { return RETURN; }

{Identifier}          { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}             { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}               { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

.                     { return yytext[0]; }

/* 块注释 */      
"/*"                  { BEGIN(COMMENT); }

<COMMENT>"*/"         { BEGIN(INITIAL); }
<COMMENT>\n           { /* 忽略换行, 不做任何操作 */ }
<COMMENT>.            { /* 忽略普通字符, 不做任何操作 */ }
<COMMENT><<EOF>>      {
                          std::cerr << "Error: Unclosed comment" << std::endl;
                          yyterminate();
                      }
                    
// 所以上面基本上就是忽略不要的，然后针对那些要的，存储匹配的/转换后的（对于那种需要转换的）字符串，返回其 token 类型

// 这里写 Flex/Bison 的规则描述
// 对于 Flex, 这里写的是 lexer 扫描到某个 token 后做的操作
// 对于 Bison, 这里写的是 parser 遇到某种语法规则后做的操作

%%

// 这里写一些用户自定义的代码
// 比如你希望在生成的 C/C++ 文件里定义一个函数, 做一些辅助工作
// 你同时希望在之前的规则描述里调用你定义的函数
// 那么, 你可以把 C/C++ 的函数定义写在这里, 声明写在文件开头
