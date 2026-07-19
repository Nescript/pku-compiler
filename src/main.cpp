extern int yydebug; // 1. 声明 Bison 内部的调试开关
#include <iostream>
#include <cstdio>
#include <memory>
#include <string>
#include <fstream>
#include <cassert>
#include "ast.hpp"
#include "codegen.hpp"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // yydebug = 1;
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  yyin = fopen(input, "r");
  assert(yyin);
  std::ofstream outfile(output);

  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  // cout << ast << endl;
  ast->OutputIR();

  if (std::string(mode) == "-koopa") {
    // cout << ast << endl;
    std::string final_koopa_ir = ASTContext::ir_buffer.str();
    outfile << final_koopa_ir << endl;
  } 
  else if (std::string(mode) == "-riscv") {
    // cout << ast << endl;
    CodeGen codegen(ast);
    outfile << codegen.OutputASM() << endl;
  } else {
    assert(0);
  }
  

  return 0;

}