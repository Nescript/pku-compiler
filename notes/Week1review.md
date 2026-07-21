# 概览

在第一周，我们完成了 lv1-lv3 的内容，也就是说，现在的编译器可以处理有一个 main 函数的，包含运算符（一元，二元）的简单程序。

我们的前端可以将这样的程序转化为 Koopa IR 表示，后端则进一步处理生成 riscv 汇编代码。

接下来将从前端和后端两部分分别进行说明：

# 前端

## flex

写 `sysy.l` 要做的就是：将要转换的程序的各个元素识别为一个个 token

以 `return 0;` 为例，我们希望程序能识别出这是一个 return 关键词和一个整数 0。这就是靠 flex 实现的。程序本质是字符串，sysy.l 中定义了很多字符串匹配规则（正则表达式实现的），按照从上到下的顺序进行。在匹配后会返回 token 类型（在 Bison 文件中定义的），并在 yytext 中存储该 token 的具体值。

当然，并不是一定需要返回 token 类型和存储具体值的。就比如上面的 return,匹配后返回 token 类型 RETURN,那也没有返回具体值的必要。但对整数，我们既需要返回这是个整数 INT_CONST，也需要返回其具体的值到 yytext 中。

## Bison

Bison 和 flex 是配合使用的，而且配合相当紧密。我们在 'sysy.y' 文件中定义了涉及的 token 类型，期待着 flex 帮我们捕捉。
我们还要在此实现 EBNF 表达式定义的逻辑。具体来说：
我们会列出符号和他们的推导规则，如:
```bison
AddExp
  : MulExp {
    $$ = $1;
  }
  | AddExp '+' MulExp {
    auto ast = new AddExp();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = "+";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | AddExp '-' MulExp {
    auto ast = new AddExp();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = "-";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
  
```
这里表明 AddExp 可以推导为 MulExp，或者 AddExp + MulExp，或者 AddExp - MulExp。一步步推导过去，最后会推导成全是终结符的形式。
当然这里有一些令人困惑的地方：
1. 运算的优先级是如何确定的？后续 IR 生成的过程根本没有特别处理运算优先级那一块
答案是靠 EBNF 的结构。EBNF 的规则越往下是越优先的。譬如说 MulExp 在 AddExp 之下，优先级更高。这点体现在实现上，是优先级低的元素的可以由优先级高的式子组成。那么推导时，我们会匹配到优先级高的式子，完成他们的移进和规约（）

2. `$$ = $1`?

```bison
AddExp
  : MulExp {
    $$ = $1;
  }
```
就譬如上面这一段，会发现我们直接将 $1 的值赋给了 $$，没有新建什么 ast 对象。这里可以直观理解为，我当然也可以在此处新建 ast 对象，但最后我的 ast 树的结构就像是

```text
    不透传:                         透传 $$ = $1 :
   AddExpAST                       MulExpAST
      |                              /   \
   MulExpAST                       "1"    "2"
    /    \
  "1"    "2"
```

所以我们这样可以避免建立无用的节点，精简了 AST 树的结构

# AST

AST 是 IR 之上的一种由原本程序生成的抽象结构。建立 ast 有助于帮助我们进行代码优化和生成 IR.
具体来说，我们会将原本的代码转化为一个树状结构的 AST，如上面分析透传用处时画的树那样。 
```bison
  | AddExp '-' MulExp {
    auto ast = new AddExp();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = "-";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
```
以上述片段为例，要执行这个减法，就很明确是左减右，依照这个约定，遍历的顺序也很明确了：先左后右

顺序也体现在了 AST 内部的 Dump 和 outputIR 中：
```cpp
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
```
可以看到先调用了左子树的 OutputIR 方法，然后调用右子树的。
接着再往 ss 里塞入这一句的 IR 实现。这样的话 ss 中就先有左子树那边的一系列语句，然后右子树的一系列，最后到这一句。

## IR 生成

