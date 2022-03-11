# Playtime

## Exceptions

Using setjmp and longjmp to create a exception library

### Getting started

```
gcc exceptions.c -o exceptions
./exceptions
```

### Extension ideas

Use backtrace and backtrace_symbols in glibc to create a stack trace when thrown

```
gcc -g -std=c11 test_exceptions.c -o test_exceptions -rdynamic
```