#pragma once

#include <string>
#include <memory>
#include "ast.hpp"
#include "koopa.h"
#include <cassert>

class CodeGen {
private:
  koopa_program_t program;
  koopa_raw_program_builder_t builder;
  std::string ASM;
public:
  CodeGen(const std::unique_ptr<BaseAST>& ast);
  ~CodeGen();
  std::string OutputASM();

  // 访问 raw program
  void Visit(const koopa_raw_program_t &program);

  // 访问 raw slice
  void Visit(const koopa_raw_slice_t &slice);

  // 访问函数
  void Visit(const koopa_raw_function_t &func);

  // 访问基本块
  void Visit(const koopa_raw_basic_block_t &bb);

  // 访问指令
  void Visit(const koopa_raw_value_t &value);


  void Visit(const koopa_raw_integer_t &integer);


  void Visit(const koopa_raw_return_t &ret);
};