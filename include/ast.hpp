#pragma once

#include <endian.h>
#include <memory>
#include <iostream>
#include <ostream>

class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
};

inline std::ostream& operator<<(std::ostream& os, const std::unique_ptr<BaseAST>& ast) {
  if (ast) ast->Dump();
  else os << "nullptr";
  return os;
}

class CompUnitAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_def;
  void Dump(std::ostream& os) {
    os << "CompUnitAST { " << func_def << " }";
  }
};

class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  void Dump(std::ostream& os) {
    os << func_type << " " << ident << " " << block;
  }
};

class FuncTypeAST : public BaseAST {
public:
  std::string type;
  void Dump(std::ostream& os) {
    os << type;
  }
};

class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;
  void Dump(std::ostream& os) {
    os << "{ " << stmt << " }";
  }
};

class StmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> number;
  void Dump(std::ostream& os) {
    os << "return " << number << ";";
  }
};

class NumberAST : public BaseAST {
public:
  int int_const;
  void Dump(std::ostream& os) {
    os << int_const;
  }
};