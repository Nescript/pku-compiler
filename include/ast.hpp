#pragma once
#include <endian.h>
#include <memory>
#include <iostream>
#include <ostream>
#include <string>
#include <sstream>

inline void print_indent(std::ostream& os, int indent) {
  for (int i = 0; i < indent; ++i) {
    os << "  ";
  }
}

class ASTContext {
public:
    // 使用 C++17 的 inline static 在类内直接初始化，防止多重包含时的链接冲突
    inline static int reg_counter = 0;
    inline static std::stringstream ir_buffer;
    
    // 分配并返回一个新的寄存器名字，如 "%0", "%1"
    static std::string NewReg() {
        return "%" + std::to_string(reg_counter++);
    }
    // 重置状态（用于新文件解析）
    static void Reset() {
        reg_counter = 0;
        ir_buffer.str("");
        ir_buffer.clear();
    }
};

class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void Dump(std::ostream& os, int indent) const = 0;
  virtual std::string OutputIR() = 0;
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
    os << "CompUnitAST { " << std::endl;
    func_def->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    ASTContext::Reset();
    func_def->OutputIR();
    return ASTContext::ir_buffer.str();
  }

};

class FuncDefAST : public BaseAST {
public:
  
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  // std::string IR;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "FuncDefAST { " << std::endl;
    func_type->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "ident: " << ident << std::endl;
    block->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string type_str = func_type->OutputIR();
    ASTContext::ir_buffer << "fun @" << ident << "(): " << type_str << " {\n";
    block->OutputIR();
    ASTContext::ir_buffer << "}\n";
    return ""; 
  }
};

class FuncTypeAST : public BaseAST {
public:
  std::unique_ptr<std::string> type;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "FuncTypeAST { " << std::endl;
    print_indent(os, indent + 1);
    os << "type: " << *type << std::endl;
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    if (*type == "int") return "i32"; // 要思考空格的位置
    else return "UNDEFINE TYPE"; 
  }
};

class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "BlockAST { " << std::endl;
    stmt->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    ASTContext::ir_buffer << "@entry:\n";
    stmt->OutputIR();
    return "";
  }
};

class StmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> exp;
  // 这里是int还是别的longlong呢？要取决于语言的定义
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "StmtAST { " << std::endl;
    exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string exp_name = exp->OutputIR(); 
    ASTContext::ir_buffer << "  ret " << exp_name << "\n";
    return "";
  }
};

class UnaryExpAST : public BaseAST {
public:
  std::unique_ptr<std::string> unary_op;
  std::unique_ptr<BaseAST> unary_exp;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "UnaryExpAST {" << std::endl;
    print_indent(os, indent + 1);
    os << "unary_op: " << *unary_op << std::endl;
    unary_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string exp_name = unary_exp->OutputIR();
    if (*unary_op == "+") {
      return exp_name;
    }
    if (*unary_op == "-") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = sub 0, " << exp_name << "\n";
      return reg;
    }
    if (*unary_op == "!") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = eq " << exp_name << ", 0\n";
      return reg;
    }
  }
};

class numberAST : public BaseAST {
public:
  int value;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "numberAST { " << std::endl;
    print_indent(os, indent + 1);
    os << "value: " << value << std::endl;
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    return std::to_string(value);
  }
};

class MulExp : public BaseAST {
public:
  std::unique_ptr<BaseAST> mul_exp;
  std::string op;
  std::unique_ptr<BaseAST> unary_exp;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "MulExp {" << std::endl;
    print_indent(os, indent + 1);
    os << "mul_exp: " << std::endl;
    mul_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << std::endl;
    print_indent(os, indent + 1);
    os << "unary_exp: " << std::endl;
    unary_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string mul_exp_name = mul_exp->OutputIR();
    std::string unary_exp_name = unary_exp->OutputIR();
    if (op == "*") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = mul " << mul_exp_name << ", " << unary_exp_name << "\n";
      return reg;
    }
    if (op == "/") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = div " << mul_exp_name << ", " << unary_exp_name << "\n";
      return reg;
    }
    if (op == "%") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = mod " << mul_exp_name << ", " << unary_exp_name << "\n";
      return reg;
    }
    return "";
  }
};

class AddExp : public BaseAST {
public:
  std::unique_ptr<BaseAST> add_exp;
  std::string op;
  std::unique_ptr<BaseAST> mul_exp;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "AddExp {" << std::endl;
    print_indent(os, indent + 1);
    os << "add_exp: " << std::endl;
    add_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << std::endl;
    print_indent(os, indent + 1);
    os << "mul_exp: " << std::endl;
    mul_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string add_exp_name = add_exp->OutputIR();
    std::string mul_exp_name = mul_exp->OutputIR();
    if (op == "+") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = add " << add_exp_name << ", " << mul_exp_name << "\n";
      return reg;
    }
    if (op == "-") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = sub " << add_exp_name << ", " << mul_exp_name << "\n";
      return reg;
    }
    return "";
  }
};

class RelExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> rel_exp;
  std::string op;
  std::unique_ptr<BaseAST> add_exp;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "RelExp {" << std::endl;
    print_indent(os, indent + 1);
    os << "rel_exp: " << std::endl;
    rel_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << std::endl;
    print_indent(os, indent + 1);
    os << "add_exp: " << std::endl;
    add_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string rel_exp_name = rel_exp->OutputIR();
    std::string add_exp_name = add_exp->OutputIR();
    if (op == ">") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = gt " << rel_exp_name << ", " << add_exp_name << "\n";
      return reg;
    }
    if (op == "<") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = lt " << rel_exp_name << ", " << add_exp_name << "\n";
      return reg;
    }
    if (op == "<=") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = le " << rel_exp_name << ", " << add_exp_name << "\n";
      return reg;
    }
    if (op == ">=") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = ge " << rel_exp_name << ", " << add_exp_name << "\n";
      return reg;
    }
    return "";
  }
};

class EqExp : public BaseAST {
public:
  std::unique_ptr<BaseAST> eq_exp;
  std::string op;
  std::unique_ptr<BaseAST> rel_exp;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "EqExp {" << std::endl;
    print_indent(os, indent + 1);
    os << "eq_exp: " << std::endl;
    eq_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << std::endl;
    print_indent(os, indent + 1);
    os << "rel_exp: " << std::endl;
    rel_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string eq_exp_name = eq_exp->OutputIR();
    std::string rel_exp_name = rel_exp->OutputIR();
    if (op == "==") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = eq " << eq_exp_name << ", " << rel_exp_name << "\n";
      return reg;
    }
    if (op == "!=") {
      std::string reg = ASTContext::NewReg();
      ASTContext::ir_buffer << "  " << reg << " = ne " << eq_exp_name << ", " << rel_exp_name << "\n";
      return reg;
    }
    return "";
  }
};

class LAndExp : public BaseAST {
public:
  std::unique_ptr<BaseAST> land_exp;
  std::string op;
  std::unique_ptr<BaseAST> eq_exp;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "LAndExp {" << std::endl;
    print_indent(os, indent + 1);
    os << "land_exp: " << std::endl;
    land_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << std::endl;
    print_indent(os, indent + 1);
    os << "eq_exp: " << std::endl;
    eq_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string land_exp_name = land_exp->OutputIR();
    std::string eq_exp_name = eq_exp->OutputIR();
    std::string land_exp_reg = ASTContext::NewReg();
    std::string eq_exp_reg = ASTContext::NewReg();
    std::string result_reg = ASTContext::NewReg();
    ASTContext::ir_buffer << "  " << land_exp_reg << " = ne " << land_exp_name << ", " << 0 << "\n";
    ASTContext::ir_buffer << "  " << eq_exp_reg << " = ne " << eq_exp_name << ", " << 0 << "\n";
    ASTContext::ir_buffer << "  " << result_reg << " = and " << land_exp_reg << ", " << eq_exp_reg << "\n";
    return result_reg;
  }
};

class LOrExp : public BaseAST {
public:
  std::unique_ptr<BaseAST> lor_exp;
  std::string op;
  std::unique_ptr<BaseAST> land_exp;
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "LOrExp {" << std::endl;
    print_indent(os, indent + 1);
    os << "lor_exp: " << std::endl;
    lor_exp->Dump(os, indent + 1);
    print_indent(os, indent + 1);
    os << "op: " << op << std::endl;
    print_indent(os, indent + 1);
    os << "land_exp: " << std::endl;
    land_exp->Dump(os, indent + 1);
    print_indent(os, indent);
    os << "}" << std::endl;
  }
  std::string OutputIR() override {
    std::string lor_exp_name = lor_exp->OutputIR();
    std::string land_exp_name = land_exp->OutputIR();
    std::string lor_exp_reg = ASTContext::NewReg();
    std::string land_exp_reg = ASTContext::NewReg();
    std::string result_reg = ASTContext::NewReg();
    ASTContext::ir_buffer << "  " << lor_exp_reg << " = ne " << lor_exp_name << ", " << 0 << "\n";
    ASTContext::ir_buffer << "  " << land_exp_reg << " = ne " << land_exp_name << ", " << 0 << "\n";
    ASTContext::ir_buffer << "  " << result_reg << " = or " << lor_exp_reg << ", " << land_exp_reg << "\n";
    return result_reg;
  }
};
// 我们应该弄一个临时寄存器计数器，用来给寄存器命名。
// 对UnaryExp, 他会调用下一级的outputIR
// 下一级的outputIR要么是number,要么是另一层UnaryExp

// 所以我们下一级outputIR应该返回一个字符串来给他操作，要么是数字，要么是一个临时寄存器
// 这个字符串应该如何处理呢？考虑同一时间只有一个东西要outputIR,可以考虑建立一个静态成员