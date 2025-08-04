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

```
key: def | extern
identifier: [a-zA-Z][a-zA-Z0-9]*
Number: [0-9.]+
```