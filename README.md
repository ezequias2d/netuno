<picture>
  <source media="(prefers-color-scheme: dark)" srcset="netuno-dark.svg">
  <img src="netuno.svg" alt="Netuno logo" height="70">
</picture>

# The Netuno Programming Language (WIP)

Netuno is a imperative, modular, functional, object-oriented, statically-typed, object-oriented programming language. It has a BASIC-like syntax, is memory-safe by default and can be easily embedded.

## Sample Code
```Ruby
import console

def fibonacci(n: uint): uint
  if n == 0u => return 0u
  if n == 1u => return 1u

  return fibonacci(n - 1u) + fibonacci(n - 2u)
end

def main()
  var input: string
  while (input = console.readline()) != "exit"
    ; this is a line comment
    var n = uint(input)
    n = fibonacci(n)
    console.write("Result: " + n + "\n")
  next
  return 0 ; ok
end

```

## Compiler and Runtime
The repository is divided by the compiler(ntc) and the runtime(ntr).

### Natch
Natch(ntc) is the Netuno Compiler, currently supports Notable Bytecode generation.

Progress
----------
- [x] Lexer
- [x] Parser
- [x] Semantic Analysis
- [ ] Optimization
- [ ] C codegen
- [x] Notable Bytecode Codegen
  - [x] Conditional
  - [x] Loops
  - [x] Break
  - [x] Continue
  - [x] Import
  - [x] Variable
  - [x] Module
  - [x] Assignment
  - [x] Expressions
  - [x] Functions and subroutines
  - [x] String
  - [ ] Reference
  - [ ] Array
  - [ ] Custom type declaration
  - [ ] Object methods

### Nitro
Nitro(ntr) is the Netuno Runtime, it has a stack virtual machine that can runs Notable Bytecode.

## Building
### Debug Mode
Debug trace will be enable and will print Abstract Syntax Tree in parser stage and enable VM stack debug trace.
```
  $ cmake -DCMAKE_BUILD_TYPE=Debug .
  $ cmake --build .
  $ ./bin/ntc sample.nt
```

### Release Mode
Debug tracing is disabled and all optimizations in the C compiler are enabled.
```
  $ cmake -DCMAKE_BUILD_TYPE=Release .
  $ cmake --build .
  $ ./bin/ntc sample.nt
```
