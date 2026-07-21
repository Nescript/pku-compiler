%define parse.trace
%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST* ast_val;
}

// 
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST
%token LE   // <=
%token GE   // >=
%token EQ   // ==
%token NE   // !=
%token AND  // &&
%token OR   // ||

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt PrimaryExp UnaryExp Exp Number
%type <ast_val> MulExp AddExp RelExp EqExp LAndExp LOrExp
%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = std::move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = make_unique<string>("int");
    $$ = ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    auto ast = new numberAST();
    ast->value = $1;
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    $$ = $1;
  }
  | '+' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->unary_op = unique_ptr<string>(new string("+"));
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '-' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->unary_op = unique_ptr<string>(new string("-"));
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '!' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->unary_op = unique_ptr<string>(new string("!"));
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    $$ = $1;
  }
  ;

PrimaryExp 
  : '(' Exp ')' {
    $$ = $2;
  }
  | Number {
    $$ = $1;
  }
  ;

MulExp
  : UnaryExp {
    $$ = $1;
  }
  | MulExp '*' UnaryExp {
    auto ast = new MulExp();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = "*";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    auto ast = new MulExp();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = "/";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    auto ast = new MulExp();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = "%";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    $$ = $1;
  }
  | AddExp '+' MulExp {
    auto ast = new AddExp();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = "+";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | AddExp '-' MulExp {
    auto ast = new AddExp();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = "-";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    $$ = $1;
  }
  | RelExp '<' AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = "<";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp '>' AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = ">";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp LE AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = "<=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp GE AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = ">=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    $$ = $1;
  }
  | EqExp EQ RelExp {
    auto ast = new EqExp();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->op = "==";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | EqExp NE RelExp {
    auto ast = new EqExp();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->op = "!=";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    $$ = $1;
  }
  | LAndExp AND EqExp {
    auto ast = new LAndExp();
    ast->land_exp = unique_ptr<BaseAST>($1);
    ast->op = "&&";
    ast->eq_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    $$ = $1;
  }
  | LOrExp OR LAndExp {
    auto ast = new LOrExp();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    ast->op = "||";
    ast->land_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
%%

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  std::cerr << "Error: " << s << std::endl;
}

// 这里写一些用户自定义的代码
// 比如你希望在生成的 C/C++ 文件里定义一个函数, 做一些辅助工作
// 你同时希望在之前的规则描述里调用你定义的函数
// 那么, 你可以把 C/C++ 的函数定义写在这里, 声明写在文件开头
