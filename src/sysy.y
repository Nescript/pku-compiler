%code requires {
  #include <memory>
  #include <string>
}

%{

#include <iostream>
#include <memory>
#include <string>

int yylex();
void yyerror(std::unique_ptr<std::string> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<std::string> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
}

// 
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <str_val> FuncDef FuncType Block Stmt Number

// 这里写一些 Flex/Bison 相关的定义
// 对于 Flex, 这里可以定义某个符号对应的正则表达式
// 对于 Bison, 这里可以定义终结符/非终结符的类型

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    ast = unique_ptr<string>($1);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto type = unique_ptr<string>($1);
    auto ident = unique_ptr<string>($2);
    auto block = unique_ptr<string>($5);
    $$ = new string(*type + " " + *ident + "() " + *block);
  }
  ;

FuncType
  : INT {
    $$ = new string("int");
  }
  ;

Block
  : '{' Stmt '}' {
    auto stmt = unique_ptr<string>($2);
    $$ = new string("{ " + *stmt + " }");
  }
  ;

Stmt
  : RETURN Number ';' {
    auto number = unique_ptr<string>($2);
    $$ = new string("return " + *number + ";");
  }
  ;

Number
  : INT_CONST {
    $$ = new string(to_string($1));
  }
  ;
// 这里写 Flex/Bison 的规则描述
// 对于 Flex, 这里写的是 lexer 扫描到某个 token 后做的操作
// 对于 Bison, 这里写的是 parser 遇到某种语法规则后做的操作

%%

void yyerror(unique_ptr<string> &ast, const char *s) {
  std::cerr << "Error: " << s << std::endl;
}

// 这里写一些用户自定义的代码
// 比如你希望在生成的 C/C++ 文件里定义一个函数, 做一些辅助工作
// 你同时希望在之前的规则描述里调用你定义的函数
// 那么, 你可以把 C/C++ 的函数定义写在这里, 声明写在文件开头
