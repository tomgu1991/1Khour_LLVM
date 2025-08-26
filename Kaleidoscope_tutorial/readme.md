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
          ::= binary LETTER number? (id, id)
          ::= unary LETTER number? (id)

expression ::= unary binoprhs// 表达式，递归形式
binoprhs ::= ('+' unary)* // 二元右侧，对于自定义的操作符，那么就可以认为op+expr相当于一个函数调用
unary ::= primary | '!'unary 
primary ::= identifierexpr | numberexpr | parenexpr | ifexpr | forexpr | varexpr
ifexpr ::= 'if' expression 'then' expression 'else' expression
forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
varexpr ::= 'var' identifier ('=' expression)?
                (',' identifier ('=' expression)?)* 'in' expression

identifierexpr ::= identifier | identifier '(' expression* ')'
numberexpr ::= number
parenexpr ::= '(' expression ')'

key: def | extern | if | then | else | for | in | unary | binary
identifier: [a-zA-Z][a-zA-Z0-9]*
Number: [0-9.]+
```
* 对于自定义操作符号，unary之后就是它的优先级数字