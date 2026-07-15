%option noinput
%option noyywrap
%option nounput

%{

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include "sysy.tab.hpp"

using namespace std;

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

%%

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
%%