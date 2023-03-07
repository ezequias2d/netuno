/*
MIT License

Copyright (c) 2022 Ezequias Silva <ezequiasmoises@gmail.com> and the Netuno
contributors. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
kind(NONE)
// expr
kind(LITERAL)
kind(UNARY)
kind(BINARY)
kind(CONSTANT)
kind(VARIABLE)
kind(ASSIGN)
kind(LOGICAL)
kind(CALL)
kind(GET)

// stmt
kind(EXPR)
kind(RETURN)
kind(LABEL)
kind(BLOCK)
kind(IF)
kind(WHILE)
kind(UNTIL)
kind(BREAK)
kind(CONTINUE)
kind(DEF)
kind(PUBLIC)
kind(PRIVATE)
kind(SUB)
kind(VAR)
kind(PARAM)
kind(TYPE)
kind(MODULE)
kind(IMPORT)

// type
kind(BOOL)
kind(U32)
kind(U64)
kind(I32)
kind(I64)
kind(F32)
kind(F64)
kind(CUSTOM_TYPE)
