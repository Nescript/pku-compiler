# EBNF 拓展巴克斯范式
可以用来描述编程语言的语法，由若干的`A ::= B;` 组成

每一条都意味着当我们遇到 'A' 时，可以替换为 'B'

由此，`A` 被称为非终结符，因为可以推导出别的符号.

## 例子
考虑以下程序：
```c
int main() {
  // 忽略我的存在
  return 0;
}
```
其 EBNF 表示为：
```EBNF
CompUnit  ::= FuncDef;

FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";

Block     ::= "{" Stmt "}";
Stmt      ::= "return" Number ";";
Number    ::= INT_CONST;
```
由开始符号 `CompUnit` 开始推导，不断利用规则代换符号，可以得到：

```EBNF
"int" IDENT "(" ")" "{" "return" INT_CONST ";" "}"
```
除了非终结符, 在我们使用的 EBNF 中还会出现一些别的记法:
- `A | B` 表示可以推导出 `A`, 或者 `B`.
- `[...]` 表示方括号内包含的项可被重复 0 次或 1 次.
- `{...}` 表示花括号内包含的项可被重复 0 次或多次.

例如:
```EBNF
Params ::= Param {"," Param};
Param ::= Type IDENT;
Type ::= "int" | "long";
```
可以表示类似 `int param`, `int x, long y, int z` 这样的参数列表.

