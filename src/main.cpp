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

  if (std::string(mode) == "-koopa") {
    // cout << ast << endl;
    outfile << ast->OutputIR() << endl;
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