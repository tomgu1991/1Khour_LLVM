# Notes of Kaleidoscope language

---
> https://releases.llvm.org/20.1.0/docs/tutorial/MyFirstLanguageFrontend/index.html

## Language Spec
Example:
```
# Compute the x'th fibonacci number.
def fib(x)
  if x < 3 then
    1
  else
    fib(x-1)+fib(x-2)

# This expression will compute the 40th number.
fib(40)
```

Language Spec:
```
top ::= definition | external | expression | ';'

definition ::= 'def' prototype expression // 函数定义
external ::= 'extern' prototype // 外部声明
toplevelexpr ::= expression

prototype ::= id '(' id* ')' // 函数签名

expression ::= primary binoprhs | ifexpr// 表达式，递归形式
primary ::= identifierexpr | numberexpr | parenexpr
binoprhs ::= ('+' primary)* // 二元右侧
ifexpr ::= 'if' expression 'then' expression 'else' expression

identifierexpr ::= identifier | identifier '(' expression* ')'
numberexpr ::= number
parenexpr ::= '(' expression ')'

key: def | extern
identifier: [a-zA-Z][a-zA-Z0-9]*
Number: [0-9.]+
```