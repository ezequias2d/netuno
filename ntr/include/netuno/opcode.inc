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
bytecode(ZERO_32)
bytecode(ZERO_64)
bytecode(ZERO_F32)
bytecode(ZERO_F64)
bytecode(ONE_32)
bytecode(ONE_64)
bytecode(ONE_F32)
bytecode(ONE_F64)
bytecode(CONST_32)
bytecode(CONST_64)
bytecode(CONST_OBJECT)

bytecode(CALL)
bytecode(RETURN)

bytecode(BRANCH)
bytecode(BRANCH_Z_32)
bytecode(BRANCH_Z_64)
bytecode(BRANCH_NZ_32)
bytecode(BRANCH_NZ_64)

bytecode(POP)
bytecode(POP_32)
bytecode(POP_64)

bytecode(LOAD_SP_32)
bytecode(LOAD_SP_64)
bytecode(STORE_SP_32)
bytecode(STORE_SP_64)

bytecode(EQ_32)
bytecode(EQ_F32)
bytecode(EQ_64)
bytecode(EQ_F64)

bytecode(NE_32)
bytecode(NE_F32)
bytecode(NE_64)
bytecode(NE_F64)

bytecode(GT_I32)
bytecode(GT_U32)
bytecode(GT_F32)
bytecode(GT_I64)
bytecode(GT_U64)
bytecode(GT_F64)

bytecode(LT_I32)
bytecode(LT_U32)
bytecode(LT_F32)
bytecode(LT_I64)
bytecode(LT_U64)
bytecode(LT_F64)

bytecode(GE_I32)
bytecode(GE_U32)
bytecode(GE_F32)
bytecode(GE_I64)
bytecode(GE_U64)
bytecode(GE_F64)

bytecode(LE_I32)
bytecode(LE_U32)
bytecode(LE_F32)
bytecode(LE_I64)
bytecode(LE_U64)
bytecode(LE_F64)

bytecode(NEG_I32)
bytecode(NEG_I64)
bytecode(NEG_F32)
bytecode(NEG_F64)

bytecode(NOT_32)
bytecode(NOT_64)
bytecode(IS_ZERO_32)
bytecode(IS_NOT_ZERO_32)
bytecode(IS_ZERO_64)
bytecode(IS_NOT_ZERO_64)
bytecode(IS_ZERO_F32)
bytecode(IS_NOT_ZERO_F32)
bytecode(IS_ZERO_F64)
bytecode(IS_NOT_ZERO_F64)

bytecode(CONCAT)

bytecode(ADD_I32)
bytecode(ADD_I64)
bytecode(ADD_F32)
bytecode(ADD_F64)

bytecode(SUB_I32)
bytecode(SUB_I64)
bytecode(SUB_F32)
bytecode(SUB_F64)

bytecode(MUL_I32)
bytecode(MUL_I64)
bytecode(MUL_F32)
bytecode(MUL_F64)

bytecode(DIV_U32)
bytecode(DIV_U64)
bytecode(DIV_I32)
bytecode(DIV_I64)
bytecode(DIV_F32)
bytecode(DIV_F64)

bytecode(REM_I32)
bytecode(REM_I64)
bytecode(REM_U32)
bytecode(REM_U64)
bytecode(REM_F32)
bytecode(REM_F64)

bytecode(EXTEND_I32)
bytecode(EXTEND_U32)
bytecode(WRAP_I64)
bytecode(PROMOTE_F32)
bytecode(DEMOTE_F64)

bytecode(CONVERT_F32_I32)
bytecode(CONVERT_F32_I64)
bytecode(CONVERT_F32_U32)
bytecode(CONVERT_F32_U64)
bytecode(CONVERT_F64_I32)
bytecode(CONVERT_F64_I64)
bytecode(CONVERT_F64_U32)
bytecode(CONVERT_F64_U64)
bytecode(CONVERT_I32_STR)
bytecode(CONVERT_U32_STR)
bytecode(CONVERT_I64_STR)
bytecode(CONVERT_U64_STR)
bytecode(CONVERT_F32_STR)
bytecode(CONVERT_F64_STR)
bytecode(CONVERT_STR_I32)
bytecode(CONVERT_STR_U32)
bytecode(CONVERT_STR_I64)
bytecode(CONVERT_STR_U64)
bytecode(CONVERT_STR_F32)
bytecode(CONVERT_STR_F64)

bytecode(TRUNCATE_I32_F32)
bytecode(TRUNCATE_I64_F32)
bytecode(TRUNCATE_U32_F32)
bytecode(TRUNCATE_U64_F32)
bytecode(TRUNCATE_I32_F64)
bytecode(TRUNCATE_I64_F64)
bytecode(TRUNCATE_U32_F64)
bytecode(TRUNCATE_U64_F64)

bytecode(MIN_F32)
bytecode(MIN_F64)
bytecode(MAX_F32)
bytecode(MAX_F64)
bytecode(NEAREST_F32)
bytecode(NEAREST_F64)
bytecode(CEIL_F32)
bytecode(CEIL_F64)
bytecode(FLOOR_F32)
bytecode(FLOOR_F64)
bytecode(TRUNCATE_F32)
bytecode(TRUNCATE_F64)
bytecode(ABS_F32)
bytecode(ABS_F64)
bytecode(SQRT_F32)
bytecode(SQRT_F64)
bytecode(COPYSIGN_F32)
bytecode(COPYSIGN_F64)
bytecode(AND_I32)
bytecode(AND_I64)
bytecode(OR_I32)
bytecode(OR_I64)
bytecode(XOR_I32)
bytecode(XOR_I64)
bytecode(SHL_I32)
bytecode(SHL_I64)
bytecode(SHR_I32)
bytecode(SHR_I64)
bytecode(SHR_U32)
bytecode(SHR_U64)
bytecode(ROL_I32)
bytecode(ROL_I64)
bytecode(ROR_I32)
bytecode(ROR_I64)
bytecode(CLZ_I32)
bytecode(CLZ_I64)
bytecode(CTZ_I32)
bytecode(CTZ_I64)
bytecode(POPCNT_I32)
bytecode(POPCNT_I64)
