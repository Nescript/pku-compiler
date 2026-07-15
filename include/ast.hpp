#pragma once

#include <endian.h>
#include <memory>
#include <iostream>
#include <ostream>
#include <string>

class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void Dump(std::ostream& os) const = 0;
};

inline std::ostream& operator<<(std::ostream& os, const std::unique_ptr<BaseAST>& ast) {
  if (ast) ast->Dump(os);
  else os << "nullptr";
  return os;
}

class CompUnitAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_def;
  void Dump(std::ostream& os) const override {
    os << "CompUnitAST { " << func_def << " }";
  }
};

class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  void Dump(std::ostream& os) const override {
    os << "FuncDefAST { " << func_type << ", " << ident << ", " << block << " }";
  }
};

class FuncTypeAST : public BaseAST {
public:
  std::unique_ptr<std::string> type;
  void Dump(std::ostream& os) const override {
    os << "FuncTypeAST { " << *type << " }";
  }
};

class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;
  void Dump(std::ostream& os) const override {
    os << "BlockAST { " << stmt << " }";
  }
};

class StmtAST : public BaseAST {
public:
  int number;
  // 这里是int还是别的longlong呢？要取决于语言的定义
  void Dump(std::ostream& os) const override {
    os << "StmtAST { " << number << " }";
  }
};