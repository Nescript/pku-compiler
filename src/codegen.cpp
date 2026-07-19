#include "codegen.hpp"
#include "koopa.h"
#include <string>

std::string CodeGen::OutputASM() {
  return ASM;
}

CodeGen::CodeGen(const std::unique_ptr<BaseAST>& ast) {
  std::string IRstring = ast->OutputIR();
  const char* str = IRstring.c_str(); 
  // 解析字符串 str, 得到 Koopa IR 程序

  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);
  
  Visit(raw);

  koopa_delete_raw_program_builder(builder);
}

void CodeGen::Visit(const koopa_raw_program_t &program) {

  ASM += "  .text\n";
  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

void CodeGen::Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void CodeGen::Visit(const koopa_raw_function_t &func) {
  for (size_t i = 0; i < func->bbs.len; ++i) {
    auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
    for (size_t j = 0; j < bb->insts.len; ++j) {
      auto inst = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
      if (inst->ty->tag != KOOPA_RTT_UNIT) {
        stack_map[inst] = offset;
        offset += 4;
      }
    }
  }
  total_stack_size = (offset + 15) & ~15;

  ASM += "  .globl " + std::string(func->name + 1) + "\n";
  ASM += std::string(func->name + 1) + ":\n";
  if (total_stack_size > 0) {
    ASM += "  addi sp, sp, -" + std::to_string(total_stack_size) + "\n";
  }

  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void CodeGen::Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void CodeGen::Visit(const koopa_raw_value_t &value) {
  current_value = value;
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      Visit(kind.data.binary);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

void CodeGen::Visit(const koopa_raw_integer_t &integer) {
  ASM += std::to_string(integer.value);
  // 处理 integer 指令
}

void CodeGen::Visit(const koopa_raw_binary_t &binary) {
  const auto &op = binary.op;
  const auto &lhs = binary.lhs;
  const auto &rhs = binary.rhs;

  LoadValueToReg(lhs, "t0");
  LoadValueToReg(rhs, "t1");

  switch (op) {
    case KOOPA_RBO_EQ:
      ASM += "  xor t2, t0, t1\n";
      ASM += "  seqz t2, t2\n";
      break;
    case KOOPA_RBO_SUB:
      ASM += "  sub t2, t0, t1\n";
      break;
    case KOOPA_RBO_ADD:
      ASM += "  add t2, t0, t1\n";
      break;
    case KOOPA_RBO_DIV:
      ASM += "  div t2, t0, t1\n";
      break;
    case KOOPA_RBO_MOD:
      ASM += "  rem t2, t0, t1\n";
      break;
    case KOOPA_RBO_GE:
      ASM += "  slt t2, t0, t1\n";
      ASM += "  seqz t2, t2\n";
      break;
    case KOOPA_RBO_LE:
      ASM += "  sgt t2, t0, t1\n";
      ASM += "  seqz t2, t2\n";
      break;
    case KOOPA_RBO_GT:
      ASM += "  sgt t2, t0, t1\n";
      break;
    case KOOPA_RBO_LT:
      ASM += "  slt t2, t0, t1\n";
      break;
    case KOOPA_RBO_NOT_EQ:
      ASM += "  xor t2, t0, t1\n";
      ASM += "  snez t2, t2\n";
      break;
    case KOOPA_RBO_OR:
      ASM += "  snez t0, t0\n";
      ASM += "  snez t1, t1\n";
      ASM += "  or t2, t0, t1\n";
      break;
    case KOOPA_RBO_AND:
      ASM += "  snez t0, t0\n";
      ASM += "  snez t1, t1\n";
      ASM += "  and t2, t0, t1\n";
      break;
    case KOOPA_RBO_MUL:
      ASM += "  mul t2, t0, t1\n";
      break;
    default:
      assert(false);
  }
  ASM += "  sw t2, " + GetStackLoc(current_value) + "\n";
  // 处理二元运算指令
}

void CodeGen::Visit(const koopa_raw_return_t &ret) {
  LoadValueToReg(ret.value, "a0");
  if (total_stack_size > 0) {
    ASM += "  addi sp, sp, " + std::to_string(total_stack_size) + "\n";
  }
  ASM += "  ret";
}

CodeGen::~CodeGen() {}