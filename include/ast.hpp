#pragma once

#include <endian.h>
#include <memory>
#include <iostream>
#include <ostream>
#include <string>

inline void print_indent(std::ostream& os, int indent) {
  for (int i = 0; i < indent; ++i) {
    os << "  ";
  }
}

class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void Dump(std::ostream& os, int indent) const = 0;
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
};

class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
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
};

class StmtAST : public BaseAST {
public:
  int number;
  // 这里是int还是别的longlong呢？要取决于语言的定义
  void Dump(std::ostream& os, int indent) const override {
    print_indent(os, indent);
    os << "StmtAST { " << std::endl;
    print_indent(os, indent + 1);
    os << "number: " << number << std::endl;
    print_indent(os, indent);
    os << "}" << std::endl;
  }
};