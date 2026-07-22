#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

inline void print_indent(std::ostream& os, int indent) {
  for (int i = 0; i < indent; ++i) {
    os << "  ";
  }
}

class ASTContext {
public:
  inline static int reg_counter = 0;
  inline static std::stringstream ir_buffer;
  // 全局符号表：变量/常量名 -> 编译期常数值
  inline static std::unordered_map<std::string, int> symbol_table;

  // 分配并返回一个新的寄存器名字，如 "%0", "%1"
  static std::string NewReg() {
    return "%" + std::to_string(reg_counter++);
  }

  // 重置状态
  static void Reset() {
    reg_counter = 0;
    ir_buffer.str("");
    ir_buffer.clear();
    symbol_table.clear();
  }
};

class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void Dump(std::ostream& os, int indent) const = 0;
  virtual std::string OutputIR() = 0;
  virtual std::optional<int> CalcValue() = 0;
};

inline std::ostream& operator<<(std::ostream& os, const std::unique_ptr<BaseAST>& ast) {
  if (ast) ast->Dump(os, 0);
  else os << "nullptr";
  return os;
}

class CompUnitAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_def;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "CompUnitAST {\n";
    if (func_def) func_def->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    ASTContext::Reset();
    if (func_def) func_def->OutputIR();
    return ASTContext::ir_buffer.str();
  }

  std::optional<int> CalcValue() override {
    assert(false && "CompUnitAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "FuncDefAST {\n";
    if (func_type) func_type->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "ident: " << ident << "\n";
    if (block) block->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string type_str = func_type ? func_type->OutputIR() : "i32";
    ASTContext::ir_buffer << "fun @" << ident << "(): " << type_str << " {\n";
    if (block) block->OutputIR();
    ASTContext::ir_buffer << "}\n";
    return "";
  }

  std::optional<int> CalcValue() override {
    assert(false && "FuncDefAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class FuncTypeAST : public BaseAST {
public:
  std::string type;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "FuncTypeAST {\n";
    print_indent(os, indent + 1);
    os << "type: " << type << "\n";
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    if (type == "int") return "i32";
    return "UNDEFINE TYPE";
  }

  std::optional<int> CalcValue() override {
    assert(false && "FuncTypeAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> block_item_list;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "BlockAST {\n";
    if (block_item_list) block_item_list->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    ASTContext::ir_buffer << "@entry:\n";
    if (block_item_list) block_item_list->OutputIR();
    return "";
  }

  std::optional<int> CalcValue() override {
    assert(false && "BlockAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class BlockItem_listAST : public BaseAST {
public:
  std::vector<std::unique_ptr<BaseAST>> block_item_vec;

  void Dump(std::ostream& os, int indent) const override {
    for (const auto& item : block_item_vec) {
      if (item) item->Dump(os, indent);
    }
  }

  std::string OutputIR() override {
    for (const auto& item : block_item_vec) {
      if (item) item->OutputIR();
    }
    return "";
  }

  std::optional<int> CalcValue() override {
    assert(false && "BlockItem_listAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class StmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> exp;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "StmtAST {\n";
    if (exp) exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string exp_name = exp ? exp->OutputIR() : "0";
    ASTContext::ir_buffer << "  ret " << exp_name << "\n";
    return "";
  }

  std::optional<int> CalcValue() override {
    assert(false && "StmtAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class numberAST : public BaseAST {
public:
  int value;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "numberAST {\n";
    print_indent(os, indent + 1);
    os << "value: " << value << "\n";
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    return std::to_string(value);
  }

  std::optional<int> CalcValue() override {
    return value;
  }
};

class LValAST : public BaseAST {
public:
  std::string ident;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "LValAST {\n";
    print_indent(os, indent + 1);
    os << "ident: " << ident << "\n";
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    auto val = CalcValue();
    if (val.has_value()) {
      return std::to_string(val.value());
    }
    return ident;
  }

  std::optional<int> CalcValue() override {
    auto it = ASTContext::symbol_table.find(ident);
    if (it != ASTContext::symbol_table.end()) {
      return it->second;
    }
    return std::nullopt;
  }
};

class UnaryExpAST : public BaseAST {
public:
  std::string unary_op;
  std::unique_ptr<BaseAST> unary_exp;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "UnaryExpAST {\n";
    print_indent(os, indent + 1);
    os << "unary_op: " << unary_op << "\n";
    if (unary_exp) unary_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string exp_name = unary_exp ? unary_exp->OutputIR() : "0";
    if (unary_op == "+") {
      return exp_name;
    }
    if (unary_op == "-") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = sub 0, " << exp_name << "\n";
      return reg;
    }
    if (unary_op == "!") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = eq " << exp_name << ", 0\n";
      return reg;
    }
    return exp_name;
  }

  std::optional<int> CalcValue() override {
    if (!unary_exp) return std::nullopt;
    auto val = unary_exp->CalcValue();
    if (!val.has_value()) return std::nullopt;

    if (unary_op == "+") return val.value();
    if (unary_op == "-") return -val.value();
    if (unary_op == "!") return !val.value();
    return std::nullopt;
  }
};

class MulExp : public BaseAST {
public:
  std::unique_ptr<BaseAST> mul_exp;
  std::string op;
  std::unique_ptr<BaseAST> unary_exp;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "MulExp {\n";
    if (mul_exp) mul_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << "\n";
    if (unary_exp) unary_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string mul_exp_name = mul_exp ? mul_exp->OutputIR() : "0";
    std::string unary_exp_name = unary_exp ? unary_exp->OutputIR() : "0";
    std::string reg = ASTContext::NewReg();

    if (op == "*") {
      ASTContext::ir_buffer << "  " << reg << " = mul " << mul_exp_name << ", " << unary_exp_name << "\n";
    } else if (op == "/") {
      ASTContext::ir_buffer << "  " << reg << " = div " << mul_exp_name << ", " << unary_exp_name << "\n";
    } else if (op == "%") {
      ASTContext::ir_buffer << "  " << reg << " = mod " << mul_exp_name << ", " << unary_exp_name << "\n";
    }
    return reg;
  }

  std::optional<int> CalcValue() override {
    if (!mul_exp || !unary_exp) return std::nullopt;
    auto lhs = mul_exp->CalcValue();
    auto rhs = unary_exp->CalcValue();

    if (lhs.has_value() && rhs.has_value()) {
      if (op == "*") return lhs.value() * rhs.value();
      if (op == "/") {
        assert(rhs.value() != 0 && "Division by zero in constant expression");
        return lhs.value() / rhs.value();
      }
      if (op == "%") {
        assert(rhs.value() != 0 && "Modulo by zero in constant expression");
        return lhs.value() % rhs.value();
      }
    }
    return std::nullopt;
  }
};

class AddExp : public BaseAST {
public:
  std::unique_ptr<BaseAST> add_exp;
  std::string op;
  std::unique_ptr<BaseAST> mul_exp;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "AddExp {\n";
    if (add_exp) add_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << "\n";
    if (mul_exp) mul_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string add_exp_name = add_exp ? add_exp->OutputIR() : "0";
    std::string mul_exp_name = mul_exp ? mul_exp->OutputIR() : "0";
    std::string reg = ASTContext::NewReg();

    if (op == "+") {
      ASTContext::ir_buffer << "  " << reg << " = add " << add_exp_name << ", " << mul_exp_name << "\n";
    } else if (op == "-") {
      ASTContext::ir_buffer << "  " << reg << " = sub " << add_exp_name << ", " << mul_exp_name << "\n";
    }
    return reg;
  }

  std::optional<int> CalcValue() override {
    if (!add_exp || !mul_exp) return std::nullopt;
    auto lhs = add_exp->CalcValue();
    auto rhs = mul_exp->CalcValue();

    if (lhs.has_value() && rhs.has_value()) {
      if (op == "+") return lhs.value() + rhs.value();
      if (op == "-") return lhs.value() - rhs.value();
    }
    return std::nullopt;
  }
};

class RelExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> rel_exp;
  std::string op;
  std::unique_ptr<BaseAST> add_exp;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "RelExp {\n";
    if (rel_exp) rel_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << "\n";
    if (add_exp) add_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string rel_exp_name = rel_exp ? rel_exp->OutputIR() : "0";
    std::string add_exp_name = add_exp ? add_exp->OutputIR() : "0";
    std::string reg = ASTContext::NewReg();

    if (op == ">") {
      ASTContext::ir_buffer << "  " << reg << " = gt " << rel_exp_name << ", " << add_exp_name << "\n";
    } else if (op == "<") {
      ASTContext::ir_buffer << "  " << reg << " = lt " << rel_exp_name << ", " << add_exp_name << "\n";
    } else if (op == "<=") {
      ASTContext::ir_buffer << "  " << reg << " = le " << rel_exp_name << ", " << add_exp_name << "\n";
    } else if (op == ">=") {
      ASTContext::ir_buffer << "  " << reg << " = ge " << rel_exp_name << ", " << add_exp_name << "\n";
    }
    return reg;
  }

  std::optional<int> CalcValue() override {
    if (!rel_exp || !add_exp) return std::nullopt;
    auto lhs = rel_exp->CalcValue();
    auto rhs = add_exp->CalcValue();

    if (lhs.has_value() && rhs.has_value()) {
      if (op == ">") return lhs.value() > rhs.value();
      if (op == "<") return lhs.value() < rhs.value();
      if (op == "<=") return lhs.value() <= rhs.value();
      if (op == ">=") return lhs.value() >= rhs.value();
    }
    return std::nullopt;
  }
};

class EqExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> eq_exp;
  std::string op;
  std::unique_ptr<BaseAST> rel_exp;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "EqExpAST {\n";
    if (eq_exp) eq_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << "\n";
    if (rel_exp) rel_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string eq_exp_name = eq_exp ? eq_exp->OutputIR() : "0";
    std::string rel_exp_name = rel_exp ? rel_exp->OutputIR() : "0";
    std::string reg = ASTContext::NewReg();

    if (op == "==") {
      ASTContext::ir_buffer << "  " << reg << " = eq " << eq_exp_name << ", " << rel_exp_name << "\n";
    } else if (op == "!=") {
      ASTContext::ir_buffer << "  " << reg << " = ne " << eq_exp_name << ", " << rel_exp_name << "\n";
    }
    return reg;
  }

  std::optional<int> CalcValue() override {
    if (!eq_exp || !rel_exp) return std::nullopt;
    auto lhs = eq_exp->CalcValue();
    auto rhs = rel_exp->CalcValue();

    if (lhs.has_value() && rhs.has_value()) {
      if (op == "==") return lhs.value() == rhs.value();
      if (op == "!=") return lhs.value() != rhs.value();
    }
    return std::nullopt;
  }
};

class LAndExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> land_exp;
  std::string op;
  std::unique_ptr<BaseAST> eq_exp;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "LAndExpAST {\n";
    if (land_exp) land_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << "\n";
    if (eq_exp) eq_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string land_exp_name = land_exp ? land_exp->OutputIR() : "0";
    std::string eq_exp_name = eq_exp ? eq_exp->OutputIR() : "0";
    std::string land_exp_reg = ASTContext::NewReg();
    std::string eq_exp_reg = ASTContext::NewReg();
    std::string result_reg = ASTContext::NewReg();

    ASTContext::ir_buffer << "  " << land_exp_reg << " = ne " << land_exp_name << ", 0\n";
    ASTContext::ir_buffer << "  " << eq_exp_reg << " = ne " << eq_exp_name << ", 0\n";
    ASTContext::ir_buffer << "  " << result_reg << " = and " << land_exp_reg << ", " << eq_exp_reg << "\n";
    return result_reg;
  }

  std::optional<int> CalcValue() override {
    if (!land_exp || !eq_exp) return std::nullopt;
    auto lhs = land_exp->CalcValue();
    auto rhs = eq_exp->CalcValue();

    if (lhs.has_value() && rhs.has_value()) {
      return (lhs.value() != 0) && (rhs.value() != 0);
    }
    return std::nullopt;
  }
};

class LOrExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lor_exp;
  std::string op;
  std::unique_ptr<BaseAST> land_exp;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "LOrExpAST {\n";
    if (lor_exp) lor_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << "\n";
    if (land_exp) land_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    std::string lor_exp_name = lor_exp ? lor_exp->OutputIR() : "0";
    std::string land_exp_name = land_exp ? land_exp->OutputIR() : "0";
    std::string lor_exp_reg = ASTContext::NewReg();
    std::string land_exp_reg = ASTContext::NewReg();
    std::string result_reg = ASTContext::NewReg();

    ASTContext::ir_buffer << "  " << lor_exp_reg << " = ne " << lor_exp_name << ", 0\n";
    ASTContext::ir_buffer << "  " << land_exp_reg << " = ne " << land_exp_name << ", 0\n";
    ASTContext::ir_buffer << "  " << result_reg << " = or " << lor_exp_reg << ", " << land_exp_reg << "\n";
    return result_reg;
  }

  std::optional<int> CalcValue() override {
    if (!lor_exp || !land_exp) return std::nullopt;
    auto lhs = lor_exp->CalcValue();
    auto rhs = land_exp->CalcValue();

    if (lhs.has_value() && rhs.has_value()) {
      return (lhs.value() != 0) || (rhs.value() != 0);
    }
    return std::nullopt;
  }
};

class ConstDefAST : public BaseAST {
public:
  std::string ident;
  std::unique_ptr<BaseAST> const_init_val;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "ConstDefAST {\n";
    print_indent(os, indent + 1);
    os << "ident: " << ident << "\n";
    if (const_init_val) const_init_val->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    if (const_init_val) {
      auto val = const_init_val->CalcValue();
      assert(val.has_value() && "Constant initializer must be a constant expression");
      // 将常量名字与求出的常数值存入符号表
      ASTContext::symbol_table[ident] = val.value();
    }
    // 常量定义不需要生成运行时的赋值 IR
    return "";
  }

  std::optional<int> CalcValue() override {
    assert(false && "ConstDefAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class BTypeAST : public BaseAST {
public:
  std::string type;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "BTypeAST {\n";
    print_indent(os, indent + 1);
    os << "type: " << type << "\n";
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    return type;
  }

  std::optional<int> CalcValue() override {
    assert(false && "BTypeAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class ConstDef_listAST : public BaseAST {
public:
  std::vector<std::unique_ptr<BaseAST>> const_def_vec;

  void Dump(std::ostream& os, int indent) const override {
    for (const auto& item : const_def_vec) {
      if (item) item->Dump(os, indent);
    }
  }

  std::string OutputIR() override {
    for (const auto& item : const_def_vec) {
      if (item) item->OutputIR();
    }
    return "";
  }

  std::optional<int> CalcValue() override {
    assert(false && "ConstDef_listAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};

class ConstDeclAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> btype;
  std::unique_ptr<BaseAST> const_def_list;

  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "ConstDeclAST {\n";
    if (btype) btype->Dump(os, indent + 1);
    if (const_def_list) const_def_list->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}\n";
  }

  std::string OutputIR() override {
    if (btype) btype->OutputIR();
    if (const_def_list) const_def_list->OutputIR();
    return "";
  }

  std::optional<int> CalcValue() override {
    assert(false && "ConstDeclAST is not an expression, cannot calculate value");
    return std::nullopt;
  }
};