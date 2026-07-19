#pragma once

#include <string>
#include <memory>
#include "ast.hpp"
#include "koopa.h"
#include <cassert>
#include <unordered_map>

class CodeGen {
private:
  koopa_program_t program;
  koopa_raw_value_t current_value;
  koopa_raw_program_builder_t builder;
  std::string ASM;
  std::unordered_map<koopa_raw_value_t, int> stack_map;
  int offset = 0;
  int total_stack_size;
  std::string GetStackLoc(const koopa_raw_value_t &value) {
    auto it = stack_map.find(value);
    if (it != stack_map.end()) {
      return std::to_string(it->second) + "(sp)";
    }
    return "";
  }
  void LoadValueToReg(const koopa_raw_value_t &value, const std::string &reg) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
      ASM += "  li " + reg + ", " + std::to_string(value->kind.data.integer.value);
      ASM += "\n";
    }
    else {
      ASM += "  lw " + reg + ", " + GetStackLoc(value) + "\n";
    }
  }
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

  // 访问 integer 指令
  void Visit(const koopa_raw_integer_t &integer);


  void Visit(const koopa_raw_return_t &ret);

  // 访问二元运算指令
  void Visit(const koopa_raw_binary_t &binary);
};