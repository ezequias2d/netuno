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
#include "nir_codegen.h"
#include "modules/numbers.h"
#include "netuno/nir/basic_block.h"
#include "netuno/nir/function.h"
#include "netuno/nir/module.h"
#include "netuno/nir/type.h"
#include "netuno/nir/value.h"
#include "parser.h"
#include "report.h"
#include "resolver.h"
#include "scanner.h"
#include "scope.h"
#include "type.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/nir/constant.h>
#include <netuno/nir/instruction.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/varint.h>
#include <stdarg.h>
#include <stdio.h>

static void addType(NT_CODEGEN *codegen, const NT_TYPE *type)
{
    NT_SYMBOL entry = {
        .symbol_name = ntRefString(type->typeName),
        .type = SYMBOL_TYPE_TYPE,
        .exprType = type,
        .weak = false,
    };
    ntInsertSymbol(codegen->scope, &entry);
}

static NT_CODEGEN *createCodegen(NIR_CONTEXT *context, NIR_MODULE *module, NT_TYPE *tmodule)
{
    NT_CODEGEN *codegen = (NT_CODEGEN *)ntMalloc(sizeof(NT_CODEGEN));

    codegen->report.had_error = false;

    codegen->context = context;
    codegen->function = NULL;
    codegen->block = NULL;

    codegen->tmodule = tmodule;
    codegen->module = module;
    codegen->scope = &tmodule->fields;

    codegen->functionScope = NULL;

    codegen->public = false;

    addType(codegen, ntI32Type(context));
    addType(codegen, ntI64Type(context));
    addType(codegen, ntU32Type(context));
    addType(codegen, ntU64Type(context));
    addType(codegen, ntF32Type(context));
    addType(codegen, ntF64Type(context));
    addType(codegen, ntStringType(context));

    return codegen;
}

void freeCodegen(NT_CODEGEN *codegen)
{
    if (codegen)
    {
        ntFree(codegen);
    }
}

static void ensureStmt(const NT_NODE *node, NT_NODE_KIND kind)
{
    assert(node->type.class == NC_STMT && node->type.kind == kind &&
           node->type.literalType == LT_NONE);
}

static void beginScope(NT_CODEGEN *codegen, NT_SCOPE_TYPE type)
{
    codegen->scope = ntCreateSymbolTable(codegen->scope, type, NULL);

    if (type & (STT_FUNCTION | STT_METHOD))
        codegen->functionScope = codegen->scope;
}

static void endScope(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node);

    NT_SCOPE *const oldScope = codegen->scope;
    codegen->scope = (NT_SCOPE *)oldScope->parent;

    if (oldScope->type & (STT_FUNCTION | STT_METHOD))
    {
        codegen->functionScope = (NT_SCOPE *)oldScope->parent;
        while (codegen->functionScope != NULL &&
               !(codegen->functionScope->type & (STT_FUNCTION | STT_METHOD)))
        {
            codegen->functionScope = (NT_SCOPE *)codegen->functionScope->parent;
        }
    }
    ntFreeSymbolTable(oldScope);
}

static void addLocal(NT_CODEGEN *codegen, NT_STRING *name, const NT_TYPE *type, NIR_VALUE *variable)
{
    // TODO: check type in vstack
    const NT_SYMBOL entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_VARIABLE,
        .exprType = type,
        .weak = false,
        .value = variable,
    };
    const bool result = ntInsertSymbol(codegen->scope, &entry);
    assert(result);
}

static void addParam(NT_CODEGEN *codegen, NT_STRING *name, const NT_TYPE *type, NIR_VALUE *value)
{
    const NT_SYMBOL entry = {
        .symbol_name = ntRefString(name),
        .type = SYMBOL_TYPE_PARAM,
        .exprType = type,
        .weak = false,
        .value = value,
    };
    const bool result = ntInsertSymbol(codegen->scope, &entry);
    assert(result);
}

// static void addFunction(NT_CODEGEN *codegen, const NT_STRING *name, NT_SYMBOL_TYPE symbolType,
//                         const NT_DELEGATE_TYPE *delegateType, size_t pc, bool public)
// {
//     assert(codegen);
//     assert(codegen->module);
//     assert(name);
//     assert(IS_VALID_OBJECT(name));
//     assert(delegateType);
//     assert(IS_VALID_TYPE(delegateType));
//     assert(((symbolType & SYMBOL_TYPE_FUNCTION) == SYMBOL_TYPE_FUNCTION) ||
//            ((symbolType & SYMBOL_TYPE_SUBROUTINE) == SYMBOL_TYPE_SUBROUTINE));

//     if (codegen->functionScope->parent != &codegen->module->type.fields)
//     {
//         ntAddModuleFunction(codegen->module, name, delegateType, pc, public);
//         return;
//     }

//     const NT_DELEGATE *delegate =
//         ntAddModuleFunction(codegen->module, name, delegateType, pc, public);
//     addSymbol(codegen, name, symbolType, (const NT_TYPE *)delegateType, (void *)delegate);
// }

static const NT_TYPE *findType(NT_CODEGEN *codegen, const NT_NODE *typeNode)
{
    const NT_TOKEN *name = &typeNode->token;

    if (name->type == TK_KEYWORD)
    {
        // primitive
        switch (name->id)
        {
        case KW_BOOL:
            return ntBoolType(codegen->context);
        case KW_I32:
            return ntI32Type(codegen->context);
        case KW_I64:
            return ntI64Type(codegen->context);
        case KW_U32:
            return ntU32Type(codegen->context);
        case KW_U64:
            return ntU64Type(codegen->context);
        case KW_F32:
            return ntF32Type(codegen->context);
        case KW_F64:
            return ntF64Type(codegen->context);
        case KW_STRING:
            return ntStringType(codegen->context);
        default: {
            char *typeLex = ntToChar(ntGetKeywordLexeme(typeNode->token.id));
            ntErrorAtNode(&codegen->report, typeNode, "The keyword '%s' is not a type.", typeLex);
            ntFree(typeLex);
            return ntErrorType();
        }
        }
    }
    else
    {
        // object
        NT_SYMBOL entry;
        if (!ntLookupSymbol2(codegen->scope, name->lexeme, name->lexemeLength, NULL, &entry))
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            ntErrorAtNode(&codegen->report, typeNode, "The type '%s' don't exist.", lexeme);
            ntFree(lexeme);
            return NULL;
        }

        if (entry.type != SYMBOL_TYPE_TYPE)
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            ntErrorAtNode(&codegen->report, typeNode, "The identifier '%s' is not a type.", lexeme);
            ntFree(lexeme);
            return NULL;
        }
        return entry.exprType;
    }
}

static NIR_TYPE *getNirStringType(NIR_CONTEXT *context)
{
    return nirGetInt32PtrType(context);
}

NIR_TYPE *toNirType(NIR_CONTEXT *c, const NT_TYPE *type);

static NIR_TYPE *getNirFunctionType(NIR_CONTEXT *c, const NT_TYPE *delegateType)
{
    NIR_TYPE *const returnType = toNirType(c, delegateType->delegate.returnType);

    const size_t paramCount = delegateType->delegate.paramCount;
    NIR_TYPE **params = ntMalloc(sizeof(NIR_TYPE *) * paramCount);
    for (size_t i = 0; i < paramCount; ++i)
    {
        params[i] = toNirType(c, delegateType->delegate.params[i].type);
    }

    NIR_TYPE *result = nirGetFunctionType(c, returnType, paramCount, params, false);

    ntFree(params);

    return result;
}

NIR_TYPE *toNirType(NIR_CONTEXT *c, const NT_TYPE *type)
{
    switch (type->objectType)
    {
    case NT_TYPE_ERROR:
        return nirGetErrorType(c);
    case NT_TYPE_UNDEFINED:
        assert(0 && "Undefined has no NIR type.");
        return NULL;
    case NT_TYPE_VOID:
        return nirGetVoidType(c);
    case NT_TYPE_STRING:
        return getNirStringType(c);
    case NT_TYPE_F64:
        return nirGetDoubleType(c);
    case NT_TYPE_F32:
        return nirGetFloatType(c);
    case NT_TYPE_U64:
    case NT_TYPE_I64:
        return nirGetInt64Type(c);
    case NT_TYPE_U32:
    case NT_TYPE_I32:
        return nirGetInt32Type(c);
    case NT_TYPE_BOOL:
        return nirGetInt1Type(c);
    case NT_TYPE_DELEGATE:
        return getNirFunctionType(c, type);
    case NT_TYPE_ASSEMBLY:
        assert(0 && "Assembly has no NIR type.");
        return NULL;
    case NT_TYPE_MODULE:
        assert(0 && "Module has no NIR type.");
        return NULL;
    case NT_TYPE_OBJECT:
        return nirGetOpaquePointerType(c);
    case NT_TYPE_CUSTOM:
        // TODO
        assert(0 && "TODO: Custom types");
        return NULL;
    default:
        assert(0 && "Invalid NT_TYPE_ID");
        return NULL;
    }
}

static bool findSymbol(NT_CODEGEN *codegen, const char_t *name, const size_t length,
                       NT_SYMBOL *pEntry)
{
    return ntLookupSymbol2(codegen->scope, name, length, NULL, pEntry);
}

static void doAssign(NT_CODEGEN *codegen, const NT_NODE *node, const char_t *name, size_t length,
                     const NT_TYPE *rightType, NIR_VALUE *value)
{
    assert(codegen);
    assert(node);
    assert(name);
    assert(length);
    assert(rightType);
    assert(value);

    NT_SYMBOL entry;
    if (!findSymbol(codegen, name, length, &entry))
    {
        ntErrorAtNode(&codegen->report, node, "The symbol must be declared.");
        return;
    }

    if (entry.exprType != rightType)
    {
        char *rightTypeName = ntStringToChar(rightType->typeName);
        char *variableTypeName = ntStringToChar(entry.exprType->typeName);
        ntErrorAtNode(&codegen->report, node,
                      "The variable type '%s' is incompatible with expression type '%s'.",
                      variableTypeName, rightTypeName);
        ntFree(rightTypeName);
        ntFree(variableTypeName);
        return;
    }

    NT_SYMBOL symbol;
    bool result = ntLookupSymbol2(codegen->scope, name, length, NULL, &symbol);
    if (!result)
    {
        ntErrorAtNode(&codegen->report, node, "The variable must be declared");
        return;
    }

    assert(entry.type & SYMBOL_TYPE_VARIABLE);
    nirCreateStore(value, symbol.value, codegen->block);
    // symbol.value = value;

    // ntUpdateSymbol(codegen->scope, &symbol);
}

static NIR_VALUE *number(NT_CODEGEN *codegen, const NT_NODE *node)
{
    char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);

    uint32_t u32;
    uint64_t u64;
    switch (node->token.type)
    {
    case TK_I32:
    case TK_U32:
        sscanf(str, "%u", &u32);
        break;
    case TK_F32:
        sscanf(str, "%f", (float *)&u32);
        break;
    case TK_I64:
    case TK_U64:
        sscanf(str, "%lu", &u64);
        break;
    case TK_F64:
        sscanf(str, "%lf", (double *)&u64);
        break;
    default:
        ntErrorAtNode(&codegen->report, node, "Invalid number token type! '%s'", node->token.type);
        break;
    }
    ntFree(str);

    bool sign = false;

    switch (node->token.type)
    {
    case TK_I32:
        sign = true;
    case TK_U32:
        return nirGetInt(nirGetInt32Type(codegen->context), u32, sign);
    case TK_I64:
        sign = true;
    case TK_U64:
        return nirGetInt(nirGetInt64Type(codegen->context), u32, sign);
    case TK_F32:
        return nirGetFloat(nirGetFloatType(codegen->context), *(float *)&u32);
    case TK_F64:
        return nirGetFloat(nirGetDoubleType(codegen->context), *(double *)&u64);
    default:
        ntErrorAtNode(&codegen->report, node, "Invalid number token type! '%s'", node->token.type);
        return NULL;
    }
}

static NIR_VALUE *string(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_LITERAL &&
           node->type.literalType == LT_STRING);

    // remove quotes
    const char_t *start = node->token.lexeme + 1;
    size_t length = node->token.lexemeLength - 2;

    char_t *str = ntEscapeString(start, &length);

    NIR_TYPE *charType = nirGetInt32Type(codegen->context);
    return nirGetConstantStringData(charType, length, str);
}

static NIR_VALUE *literal(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR);
    assert(node->type.kind == NK_LITERAL);

    switch (node->type.literalType)
    {
    case LT_BOOL:
        assert(node->token.type == TK_KEYWORD);
        switch (node->token.id)
        {
        case KW_FALSE:
            return nirGetIntFalse(nirGetInt1Type(codegen->context));
        case KW_TRUE:
            return nirGetIntTrue(nirGetInt1Type(codegen->context));
        default: {
            char *lexeme = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            ntErrorAtNode(
                &codegen->report, node,
                "AST invalid format, node id of a bool literal must be TK_TRUE or TK_FALSE "
                "cannot be '%s'!",
                lexeme);
            ntFree(lexeme);
            return NULL;
        }
        }
    case LT_NONE:
        return nirCreateUnaryInst(
            NIR_UNARY_CAST_INT_TO_PTR, nirGetOpaquePointerType(codegen->context),
            nirGetIntFalse(nirGetInt1Type(codegen->context)), U"none", codegen->block);
    case LT_STRING:
        return string(codegen, node);
    case LT_I32:
    case LT_I64:
    case LT_U32:
    case LT_U64:
    case LT_F32:
    case LT_F64:
        return number(codegen, node);
    default:
        ntErrorAtNode(&codegen->report, node, "Invalid literal type! '%d'", node->type.literalType);
        return NULL;
    }
}

static NIR_VALUE *expression(NT_CODEGEN *codegen, const NT_NODE *node, const bool needValue);

static NIR_VALUE *unary(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_UNARY);
    assert(node->token.type == TK_KEYWORD);
    assert((node->right != NULL) ^ (node->left != NULL));

    const NT_TYPE *type;

    if (node->left)
    {
        type = ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->left);

        const NT_NODE *identifier = node->left;
        assert(identifier);
        assert(identifier->type.kind == NK_VARIABLE);

        NIR_VALUE *const left = expression(codegen, node->left, true);
        NIR_VALUE *resultValue = NULL;

        switch (type->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
            switch (node->token.id)
            {
            case OP_INC:
                resultValue = nirCreateBinary(
                    NIR_BINARY_OP_ADD, left, nirGetInt(nirGetInt32Type(codegen->context), 1, false),
                    U"inc", codegen->block);
                break;
            case OP_DEC:
                resultValue = nirCreateBinary(
                    NIR_BINARY_OP_SUB, left, nirGetInt(nirGetInt32Type(codegen->context), 1, false),
                    U"dec", codegen->block);
                break;
            default:
                ntErrorAtNode(&codegen->report, node, "Invalid unary operation. %d(%c)",
                              node->token.id, (char)node->token.id);
                break;
            }
            break;
        case NT_TYPE_I64:
        case NT_TYPE_U64:
            switch (node->token.id)
            {
            case OP_INC:
                resultValue = nirCreateBinary(
                    NIR_BINARY_OP_ADD, left, nirGetInt(nirGetInt64Type(codegen->context), 1, false),
                    U"inc", codegen->block);
                break;
            case OP_DEC:
                resultValue = nirCreateBinary(
                    NIR_BINARY_OP_SUB, left, nirGetInt(nirGetInt64Type(codegen->context), 1, false),
                    U"dec", codegen->block);
                break;
            default:
                ntErrorAtNode(&codegen->report, node, "Invalid unary operation. %d(%c)",
                              node->token.id, (char)node->token.id);
                break;
            }
            break;
        case NT_TYPE_F32:
            switch (node->token.id)
            {
            case OP_INC:
                resultValue = nirCreateBinary(NIR_BINARY_OP_FADD, left,
                                              nirGetFloat(nirGetFloatType(codegen->context), 1.0),
                                              U"inc", codegen->block);
                break;
            case OP_DEC:
                resultValue = nirCreateBinary(NIR_BINARY_OP_FSUB, left,
                                              nirGetFloat(nirGetFloatType(codegen->context), 1.0),
                                              U"dec", codegen->block);
                break;
            default:
                ntErrorAtNode(&codegen->report, node, "Invalid unary operation. %d(%c)",
                              node->token.id, (char)node->token.id);
                break;
            }
            break;
        case NT_TYPE_F64:
            switch (node->token.id)
            {
            case OP_INC:
                resultValue = nirCreateBinary(NIR_BINARY_OP_FADD, left,
                                              nirGetFloat(nirGetDoubleType(codegen->context), 1.0),
                                              U"inc", codegen->block);
                break;
            case OP_DEC:
                resultValue = nirCreateBinary(NIR_BINARY_OP_FSUB, left,
                                              nirGetFloat(nirGetDoubleType(codegen->context), 1.0),
                                              U"dec", codegen->block);
                break;
            default:
                ntErrorAtNode(&codegen->report, node, "Invalid unary operation. %d(%c)",
                              node->token.id, (char)node->token.id);
                break;
            }
            break;
        default: {
            char *str = ntStringToChar(type->typeName);
            ntErrorAtNode(&codegen->report, node,
                          "Invalid logical not('%s') operation with type '%s'.",
                          node->token.id == OP_INC ? "++" : "--", str);
            ntFree(str);
            break;
        }
        }

        doAssign(codegen, node, identifier->token.lexeme, identifier->token.lexemeLength, type,
                 resultValue);
        return left;
    }

    type = ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->right);
    NIR_VALUE *const right = expression(codegen, node->right, true);

    NIR_TYPE *nirType = NULL;
    switch (node->token.id)
    {
    case '-': {
        // negate
        switch (type->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
        case NT_TYPE_I64:
        case NT_TYPE_U64:
        case NT_TYPE_F32:
        case NT_TYPE_F64:
            break;
        case NT_TYPE_STRING:
        case NT_TYPE_CUSTOM:
        // TODO: call operator.
        default: {
            char *str = ntStringToChar(type->typeName);
            ntErrorAtNode(&codegen->report, node, "Invalid negate('-') operation with type '%s'.",
                          str);
            ntFree(str);
            return NULL;
        }
        }

        return nirCreateNeg(right, U"neg", codegen->block);
    }
    case '!': {
        // logical not
        switch (type->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
            nirType = nirGetInt32Type(codegen->context);
            break;
        case NT_TYPE_I64:
        case NT_TYPE_U64:
            nirType = nirGetInt64Type(codegen->context);
            break;
        case NT_TYPE_F32:
            nirType = nirGetFloatType(codegen->context);
            break;
        case NT_TYPE_F64:
            nirType = nirGetDoubleType(codegen->context);
            break;
        case NT_TYPE_STRING:
        case NT_TYPE_CUSTOM:
        // TODO: call operator.
        default: {
            char *str = ntStringToChar(type->typeName);
            ntErrorAtNode(&codegen->report, node, "Invalid negate('-') operation with type '%s'.",
                          str);
            ntFree(str);
            return NULL;
        }
        }

        NIR_VALUE *constant;
        NIR_CMP_PREDICATE predicate;
        if (nirIsIntegerType(nirType))
        {
            constant = nirGetInt(nirType, 0, false);
            predicate = NIR_ICMP_NE;
        }
        else if (nirIsFloatType(nirType))
        {
            constant = nirGetFloat(nirType, 0);
            predicate = NIR_FCMP_NE;
        }
        else
        {
            assert(0);
            return NULL;
        }

        return nirCreateCmp(predicate, right, constant, U"lnot", codegen->block);
    }
    case '~': {
        // bitwise not
        switch (type->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
            nirType = nirGetInt32Type(codegen->context);
            break;
        case NT_TYPE_I64:
        case NT_TYPE_U64:
            nirType = nirGetInt64Type(codegen->context);
            break;
        // TODO: call operator.
        default: {
            char *str = ntStringToChar(type->typeName);
            ntErrorAtNode(&codegen->report, node, "Invalid negate('-') operation with type '%s'.",
                          str);
            ntFree(str);
            return NULL;
        }
        }

        NIR_VALUE *constant;
        if (nirIsIntegerType(nirType))
            constant = nirGetIntAllOnes(nirType);
        else
        {
            assert(0);
            return NULL;
        }

        return nirCreateBinary(NIR_BINARY_OP_XOR, right, constant, U"bnot", codegen->block);
    }
    case OP_INC: {
        switch (type->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
            nirType = nirGetInt32Type(codegen->context);
            break;
        case NT_TYPE_I64:
        case NT_TYPE_U64:
            nirType = nirGetInt64Type(codegen->context);
            break;
        case NT_TYPE_F32:
            nirType = nirGetFloatType(codegen->context);
            break;
        case NT_TYPE_F64:
            nirType = nirGetDoubleType(codegen->context);
            break;
        case NT_TYPE_STRING:
        case NT_TYPE_CUSTOM:
        // TODO: call operator.
        default: {
            char *str = ntStringToChar(type->typeName);
            ntErrorAtNode(&codegen->report, node, "Invalid negate('-') operation with type '%s'.",
                          str);
            ntFree(str);
            return NULL;
        }
        }

        NIR_VALUE *constant;
        if (nirIsIntegerType(nirType))
            constant = nirGetInt(nirType, 1, false);
        else if (nirIsFloatType(nirType))
            constant = nirGetFloat(nirType, 1.0);
        else
        {
            assert(0);
            return NULL;
        }

        return nirCreateBinary(NIR_BINARY_OP_ADD, right, constant, U"inc", codegen->block);
    }
    case OP_DEC: {
        switch (type->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
            nirType = nirGetInt32Type(codegen->context);
            break;
        case NT_TYPE_I64:
        case NT_TYPE_U64:
            nirType = nirGetInt64Type(codegen->context);
            break;
        case NT_TYPE_F32:
            nirType = nirGetFloatType(codegen->context);
            break;
        case NT_TYPE_F64:
            nirType = nirGetDoubleType(codegen->context);
            break;
        case NT_TYPE_STRING:
        case NT_TYPE_CUSTOM:
        // TODO: call operator.
        default: {
            char *str = ntStringToChar(type->typeName);
            ntErrorAtNode(&codegen->report, node, "Invalid negate('-') operation with type '%s'.",
                          str);
            ntFree(str);
            return NULL;
        }
        }

        NIR_VALUE *constant;
        if (nirIsIntegerType(nirType))
            constant = nirGetInt(nirType, 1, false);
        else if (nirIsFloatType(nirType))
            constant = nirGetFloat(nirType, 1.0);
        else
        {
            assert(0);
            return NULL;
        }

        return nirCreateBinary(NIR_BINARY_OP_SUB, right, constant, U"dec", codegen->block);
    }
    default:
        ntErrorAtNode(&codegen->report, node, "Invalid unary operation. %d(%c)", node->token.id,
                      (char)node->token.id);
        return NULL;
    }
}

static NIR_VALUE *cast(NT_CODEGEN *codegen, const NT_NODE *node, NIR_VALUE *value,
                       const NT_TYPE *from, const NT_TYPE *to)
{
    if (from == to)
        return value;

    NIR_TYPE *targetType = NULL;
    NIR_OPCODE opcode;

    switch (from->objectType)
    {
    case NT_TYPE_I32:
    case NT_TYPE_I64:
        switch (to->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
            opcode = NIR_UNARY_CAST_TRUNC;
            break;
        case NT_TYPE_I64:
        case NT_TYPE_U64:
            opcode = NIR_UNARY_CAST_SEXT;
            break;
        case NT_TYPE_F32:
        case NT_TYPE_F64:
            opcode = NIR_UNARY_CAST_SI_TO_FP;
            break;
        case NT_TYPE_STRING:
            break;
        default:
            goto error;
        }
        break;
    case NT_TYPE_U32:
    case NT_TYPE_U64:
        switch (to->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
            opcode = NIR_UNARY_CAST_TRUNC;
            break;
        case NT_TYPE_I64:
        case NT_TYPE_U64:
            opcode = NIR_UNARY_CAST_ZEXT;
            break;
        case NT_TYPE_F32:
        case NT_TYPE_F64:
            opcode = NIR_UNARY_CAST_UI_TO_FP;
            break;
        case NT_TYPE_STRING:
            break;
        default:
            goto error;
        }
        break;
    case NT_TYPE_F32:
    case NT_TYPE_F64:
        switch (to->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_I64:
            opcode = NIR_UNARY_CAST_FP_TO_SI;
            break;
        case NT_TYPE_U32:
        case NT_TYPE_U64:
            opcode = NIR_UNARY_CAST_FP_TO_UI;
            break;
        case NT_TYPE_F32:
        case NT_TYPE_F64:
            opcode = NIR_UNARY_CAST_FP_TRUNC;
            break;
        case NT_TYPE_STRING:
            break;
        default:
            goto error;
        }
        break;
    case NT_TYPE_STRING: {
        const char_t *prefix = U"to_";
        const size_t prefixLength = ntStrLen(prefix);
        const size_t sourceNameLength = ntStringLength(to->typeName);
        const char_t *sourceName = ntStringChars(to->typeName, NULL);
        const size_t finalLength = prefixLength + sourceNameLength;

        char_t *name = (char_t *)ntMalloc(sizeof(char_t) * (finalLength + 1));
        ntMemcpy(name, prefix, prefixLength * sizeof(char_t));
        ntMemcpy(name + prefixLength, sourceName, sourceNameLength * sizeof(char_t));
        name[prefixLength + sourceNameLength] = U'\0';

        NT_SYMBOL symbol;

        const bool result = ntLookupSymbol2(&to->fields, name, finalLength, NULL, &symbol);
        ntFree(name);

        if (!result)
            goto error;

        assert(symbol.type & SYMBOL_TYPE_FUNCTION);

        NIR_FUNCTION *function = (NIR_FUNCTION *)symbol.value;
        assert(function);

        NIR_TYPE *functionType = toNirType(codegen->context, symbol.exprType);
        assert(functionType);

        NIR_VALUE **args = (NIR_VALUE **)ntMalloc(sizeof(NIR_VALUE *) * 1);
        args[0] = value;

        return nirCreateCall(functionType, function, 1, args, U"str", codegen->block);
    }
    default:
        goto error;
    }

    switch (to->objectType)
    {
    case NT_TYPE_I32:
    case NT_TYPE_U32:
        targetType = nirGetInt32Type(codegen->context);
        break;
    case NT_TYPE_I64:
    case NT_TYPE_U64:
        targetType = nirGetInt64Type(codegen->context);
        break;
    case NT_TYPE_F32:
        targetType = nirGetFloatType(codegen->context);
        break;
    case NT_TYPE_F64:
        targetType = nirGetDoubleType(codegen->context);
        break;
    case NT_TYPE_STRING: {
        const char_t *prefix = U"to_";
        const size_t prefixLength = ntStrLen(prefix);
        const size_t sourceNameLength = ntStringLength(to->typeName);
        const char_t *sourceName = ntStringChars(to->typeName, NULL);
        const size_t finalLength = prefixLength + sourceNameLength;

        char_t *name = (char_t *)ntMalloc(sizeof(char_t) * (finalLength + 1));
        ntMemcpy(name, prefix, prefixLength * sizeof(char_t));
        ntMemcpy(name + prefixLength, sourceName, sourceNameLength * sizeof(char_t));
        name[prefixLength + sourceNameLength] = U'\0';

        NT_SYMBOL symbol;
        const bool result = ntLookupSymbol2(&from->fields, name, finalLength, NULL, &symbol);
        ntFree(name);

        if (!result)
            goto error;

        assert(symbol.type & SYMBOL_TYPE_FUNCTION);

        NIR_FUNCTION *function = (NIR_FUNCTION *)symbol.value;
        assert(function);

        NIR_TYPE *functionType = toNirType(codegen->context, symbol.exprType);
        assert(functionType);

        NIR_VALUE **args = (NIR_VALUE **)ntMalloc(sizeof(NIR_VALUE *) * 1);
        args[0] = value;

        return nirCreateCall(functionType, function, 1, args, U"str", codegen->block);
    }
    case NT_TYPE_ASSEMBLY:
    case NT_TYPE_MODULE:
    case NT_TYPE_OBJECT:
    case NT_TYPE_DELEGATE:
    case NT_TYPE_CUSTOM:
        // TODO
    default:
        goto error;
    }

    if (targetType != nirGetType(value))
        return nirCreateUnaryInst(opcode, targetType, value, U"cast", codegen->block);
    return value;
error : {

    // TODO: object cast operator?
    char *fromStr = ntStringToChar(from->typeName);
    char *dstStr = ntStringToChar(to->typeName);
    ntErrorAtNode(&codegen->report, node, "Invalid cast from '%s' to '%s'.", fromStr, dstStr);
    ntFree(fromStr);
    ntFree(dstStr);

    return NULL;
}
}

bool isTypeSigned(const NT_TYPE *type)
{
    switch (type->objectType)
    {
    case NT_TYPE_I32:
    case NT_TYPE_I64:
        return true;
    default:
        return false;
    }
}

static NIR_VALUE *callEquals(NT_CODEGEN *codegen, NIR_VALUE *left, NIR_VALUE *right)
{
    const char_t *equals = U"equals";
    const size_t equalsLength = ntStrLen(equals);

    NT_SYMBOL symbol;
    const NT_TYPE *stringType = ntStringType(codegen->context);
    const bool result = ntLookupSymbol2(&stringType->fields, equals, equalsLength, NULL, &symbol);
    assert(result);

    assert(symbol.type & SYMBOL_TYPE_FUNCTION);

    NIR_FUNCTION *function = (NIR_FUNCTION *)symbol.value;
    assert(function);

    NIR_TYPE *functionType = toNirType(codegen->context, symbol.exprType);
    assert(functionType);

    NIR_VALUE **args = (NIR_VALUE **)ntMalloc(sizeof(NIR_VALUE *) * 2);
    args[0] = left;
    args[1] = right;

    return nirCreateCall(functionType, function, 2, args, U"tmpequals", codegen->block);
}

static NIR_VALUE *callConcat(NT_CODEGEN *codegen, NIR_VALUE *left, NIR_VALUE *right)
{
    const char_t *concat = U"concat";
    const size_t concatLength = ntStrLen(concat);

    NT_SYMBOL symbol;
    const NT_TYPE *stringType = ntStringType(codegen->context);
    const bool result = ntLookupSymbol2(&stringType->fields, concat, concatLength, NULL, &symbol);
    assert(result);

    assert(symbol.type & SYMBOL_TYPE_FUNCTION);

    NIR_FUNCTION *function = (NIR_FUNCTION *)symbol.value;
    assert(function);

    NIR_TYPE *functionType = toNirType(codegen->context, symbol.exprType);
    assert(functionType);

    NIR_VALUE **args = (NIR_VALUE **)ntMalloc(sizeof(NIR_VALUE *) * 2);
    args[0] = left;
    args[1] = right;

    return nirCreateCall(functionType, function, 2, args, U"tmpconcat", codegen->block);
}

static NIR_VALUE *binary(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_BINARY);
    assert(node->left != NULL && node->right != NULL);

    const NT_TYPE *leftType =
        ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->left);
    const NT_TYPE *rightType =
        ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->right);
    const NT_TYPE *type = leftType->objectType < rightType->objectType ? leftType : rightType;
    const bool isConcat = type == ntStringType(codegen->context);

    NIR_VALUE *left = expression(codegen, node->left, true);

    // only cast if is not concat operation, because CONCAT instruction handles any object type
    // if (left && (!isConcat || !ntTypeIsAssignableFrom(ntObjectType(), leftType)))
    left = cast(codegen, node->left, left, leftType, type);

    NIR_VALUE *right = expression(codegen, node->right, true);
    // only cast if is not concat operation, because CONCAT instruction handles any object type
    // if (right && (!isConcat || !ntTypeIsAssignableFrom(ntObjectType(), rightType)))
    right = cast(codegen, node->right, right, rightType, type);

    const bool isInt = (!left || !right) ? true
                                         : ((left && nirIsIntegerType(nirGetType(left))) ||
                                            (right && nirIsIntegerType(nirGetType(right))));
    const bool isFloat = (!left || !right) ? true
                                           : ((left && nirIsFloatType(nirGetType(left))) ||
                                              (right && nirIsFloatType(nirGetType(right))));
    const bool isSigned = (!leftType || !rightType) ? true
                                                    : ((leftType && isTypeSigned(leftType)) ||
                                                       (right && isTypeSigned(rightType)));
    switch (node->token.id)
    {
    case OP_NE:
        if (leftType == rightType && leftType == ntStringType(codegen->context))
        {
            NIR_VALUE *value = callEquals(codegen, left, right);
            value = nirCreateNot(value, U"tmpequals", codegen->block);
            return value;
        }

        assert(isInt || isFloat);
        return nirCreateCmp(isInt ? NIR_ICMP_NE : NIR_FCMP_NE, left, right, U"ne", codegen->block);
    case OP_EQ:
        if (leftType == rightType && leftType == ntStringType(codegen->context))
        {
            return callEquals(codegen, left, right);
        }

        assert(isInt || isFloat);
        return nirCreateCmp(isInt ? NIR_ICMP_EQ : NIR_FCMP_EQ, left, right, U"eq", codegen->block);
    case '>':
        assert(isInt || isFloat);
        return nirCreateCmp(isInt ? (isSigned ? NIR_ICMP_SGT : NIR_ICMP_UGT) : NIR_FCMP_GT, left,
                            right, U"gt", codegen->block);
    case OP_GE:
        assert(isInt || isFloat);
        return nirCreateCmp(isInt ? (isSigned ? NIR_ICMP_SGE : NIR_ICMP_UGE) : NIR_FCMP_GE, left,
                            right, U"ge", codegen->block);
    case '<':
        assert(isInt || isFloat);
        return nirCreateCmp(isInt ? (isSigned ? NIR_ICMP_SLT : NIR_ICMP_ULT) : NIR_FCMP_LT, left,
                            right, U"lt", codegen->block);
    case OP_LE:
        assert(isInt || isFloat);
        return nirCreateCmp(isInt ? (isSigned ? NIR_ICMP_SLE : NIR_ICMP_ULE) : NIR_FCMP_LE, left,
                            right, U"le", codegen->block);
    case '+':
        assert(isInt || isFloat || isConcat);
        if (isConcat)
        {
            return callConcat(codegen, left, right);
        }
        return nirCreateBinary(isInt ? NIR_BINARY_OP_ADD : NIR_BINARY_OP_FADD, left, right, U"add",
                               codegen->block);
    case '-':
        assert(isInt || isFloat);
        return nirCreateBinary(isInt ? NIR_BINARY_OP_SUB : NIR_BINARY_OP_FSUB, left, right, U"sub",
                               codegen->block);
    case '*':
        assert(isInt || isFloat);
        return nirCreateBinary(isInt ? NIR_BINARY_OP_MUL : NIR_BINARY_OP_FMUL, left, right, U"mul",
                               codegen->block);
    case '/':
        assert(isInt || isFloat);
        return nirCreateBinary(isInt ? (isSigned ? NIR_BINARY_OP_SDIV : NIR_BINARY_OP_UDIV)
                                     : NIR_BINARY_OP_FDIV,
                               left, right, U"div", codegen->block);
    case '%':
        assert(isInt || isFloat);
        return nirCreateBinary(isInt ? (isSigned ? NIR_BINARY_OP_SREM : NIR_BINARY_OP_UREM)
                                     : NIR_BINARY_OP_FREM,
                               left, right, U"rem", codegen->block);
    case '|':
        assert(isInt);
        return nirCreateBinary(NIR_BINARY_OP_OR, left, right, U"or", codegen->block);
    case '&':
        assert(isInt);
        return nirCreateBinary(NIR_BINARY_OP_AND, left, right, U"and", codegen->block);
    case '^':
        assert(isInt);
        return nirCreateBinary(NIR_BINARY_OP_XOR, left, right, U"xor", codegen->block);
    default:
        goto error;
    }
error:
    ntErrorAtNode(&codegen->report, node, "Invalid binary operation. %d", node->token.id);
    return NULL;
}

static NIR_VALUE *variable(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_VARIABLE);

    NT_SYMBOL entry;
    if (!findSymbol(codegen, node->token.lexeme, node->token.lexemeLength, &entry))
    {
        ntErrorAtNode(&codegen->report, node, "The symbol must be declared.");
        return NULL;
    }

    if (entry.type & SYMBOL_TYPE_VARIABLE)
    {
        NIR_TYPE *valueType = nirGetPointeeType(nirGetType(entry.value));
        // NIR_TYPE *ptrType = nirGetPointerTo(valueType);

        return nirCreateUnaryInst(NIR_UNARY_MEMORY_LOAD, valueType, entry.value, U"ltmp",
                                  codegen->block);
    }

    return entry.value;
}

static NIR_VALUE *assign(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node != NULL);
    assert(codegen != NULL);
    assert(node->type.class == NC_EXPR && node->type.kind == NK_ASSIGN);
    assert(node->left->type.class == NC_EXPR && node->left->type.kind == NK_VARIABLE);

    const NT_TYPE *rightType =
        ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->right);

    NIR_VALUE *const value = expression(codegen, node->right, true);

    const NT_NODE *identifier = node->left;
    assert(identifier);

    doAssign(codegen, node, identifier->token.lexeme, identifier->token.lexemeLength, rightType,
             value);

    return value;
}

static NIR_VALUE *typeToBool(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE *type,
                             NIR_VALUE *value)
{
    NIR_VALUE *constant;
    switch (type->objectType)
    {
    case NT_TYPE_BOOL:
        return value;
    case NT_TYPE_I32:
    case NT_TYPE_U32:
    case NT_TYPE_I64:
    case NT_TYPE_U64:
        constant = nirGetInt(nirGetType(value), 0, false);
        return nirCreateCmp(NIR_ICMP_NE, value, constant, U"b", codegen->block);
    case NT_TYPE_F64:
    case NT_TYPE_F32:
        constant = nirGetFloat(nirGetType(value), 0.0);
        return nirCreateCmp(NIR_ICMP_NE, value, constant, U"b", codegen->block);
    default: {
        char *typeStr = ntStringToChar(type->typeName);
        ntErrorAtNode(&codegen->report, node, "Invalid implicit cast from type '%s' to 'bool'.",
                      typeStr);
        ntFree(typeStr);
        return NULL;
    }
    }
}

/*
[logical and]

entry:
    value1 := i1 expr1()
    br value1, next, end
next:
    value2 := i1 expr2()
end:
    value3 := phi i1 [value1, entry], [value2, next]
*/
static NIR_VALUE *logicalAnd(NT_CODEGEN *codegen, const NT_NODE *node, NIR_VALUE *left,
                             const NT_TYPE *rightType, const NT_NODE *rightNode)
{
    assert(codegen);
    assert(node);
    assert(rightType);
    assert(rightNode);

    NIR_BASIC_BLOCK *entryBlock = codegen->block;
    NIR_BASIC_BLOCK *nextBlock = nirCreateBasicBlock(codegen->context, U"label_next");
    NIR_BASIC_BLOCK *endBlock = nirCreateBasicBlock(codegen->context, U"label_end");

    nirCreateBranch2(nextBlock, endBlock, left, codegen->block);

    // next:
    codegen->block = nextBlock;
    NIR_VALUE *right = expression(codegen, rightNode, true);
    right = typeToBool(codegen, node, rightType, right);

    // end:
    codegen->block = endBlock;
    NIR_VALUE *phi = nirCreatePhi(nirGetInt1Type(codegen->context), U"logand", endBlock);

    nirAddIncoming(phi, left, entryBlock);
    nirAddIncoming(phi, right, nextBlock);

    return phi;
}

/*
[logical or]

entry:
    value1 = expr1()
    br value1, end, next
next:
    value2 = expr2()
    br end
end:
    value3 = phi [value1, entry], [value2, next]
*/
static NIR_VALUE *logicalOr(NT_CODEGEN *codegen, const NT_NODE *node, NIR_VALUE *left,
                            const NT_TYPE *rightType, const NT_NODE *rightNode)
{
    assert(codegen);
    assert(node);
    assert(rightType);
    assert(rightNode);

    NIR_BASIC_BLOCK *entryBlock = codegen->block;
    NIR_BASIC_BLOCK *nextBlock = nirCreateBasicBlock(codegen->context, U"label_next");
    NIR_BASIC_BLOCK *endBlock = nirCreateBasicBlock(codegen->context, U"label_end");

    nirCreateBranch2(endBlock, nextBlock, left, codegen->block);

    // next:
    codegen->block = nextBlock;
    NIR_VALUE *right = expression(codegen, rightNode, true);
    right = typeToBool(codegen, node, rightType, right);
    nirCreateBranch1(endBlock, nextBlock);

    // end:
    codegen->block = endBlock;
    NIR_VALUE *phi = nirCreatePhi(nirGetInt1Type(codegen->context), U"logand", endBlock);

    nirAddIncoming(phi, left, entryBlock);
    nirAddIncoming(phi, right, nextBlock);

    return phi;
}

static NIR_VALUE *logical(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(codegen);
    assert(node);
    assert(node->type.class == NC_EXPR && node->type.kind == NK_LOGICAL);

    const NT_TYPE *type =
        ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->left);
    NIR_VALUE *left = expression(codegen, node->left, true);
    left = typeToBool(codegen, node, type, left);

    const NT_TYPE *rightType =
        ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->right);

    switch (node->token.id)
    {
    case OP_LOGAND:
        return logicalAnd(codegen, node, left, rightType, node->right);
    case OP_LOGOR:
        return logicalOr(codegen, node, left, rightType, node->right);
    default: {
        char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        ntErrorAtNode(&codegen->report, node, "Invalid logical operation with ID '%d'('%s').",
                      node->token.id, str);
        ntFree(str);
        return NULL;
    }
    }
}

static NIR_VALUE *get(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(codegen);
    assert(node);
    assert(node->type.class == NC_EXPR);
    assert(node->type.kind == NK_GET);

    const NT_SCOPE *scope = codegen->scope;
    NT_NODE *current = node->left;

    NT_SYMBOL entry;
    entry.type = SYMBOL_TYPE_NONE;
    do
    {
        const bool result = ntLookupSymbol2(scope, current->token.lexeme,
                                            current->token.lexemeLength, NULL, &entry);
        current = current->left;
        if (result && entry.exprType)
            scope = &entry.exprType->fields;
        else
            break;
    } while (current);

    if (entry.type == SYMBOL_TYPE_NONE || current != NULL)
    {
        ntErrorAtNode(&codegen->report, node->left, "Undeclared symbol");
        return NULL;
    }

    if (entry.type & SYMBOL_TYPE_MODULE)
    {
        const NT_TYPE *type = entry.exprType;
        const bool result = ntLookupSymbol2(&type->fields, node->token.lexeme,
                                            node->token.lexemeLength, NULL, &entry);

        if (result && (entry.type &
                       (SYMBOL_TYPE_MODULE | SYMBOL_TYPE_FUNCTION | SYMBOL_TYPE_SUBROUTINE)) != 0)
            return entry.value;
    }

    if (entry.type & SYMBOL_TYPE_VARIABLE)
    {
        NIR_TYPE *ptrType = nirGetType(entry.value);
        NIR_TYPE *valueType = nirGetPointeeType(ptrType);

        return nirCreateUnaryInst(NIR_UNARY_MEMORY_LOAD, valueType, entry.value, U"ltmp",
                                  codegen->block);
    }

    return entry.value;
}

static NIR_VALUE *call(NT_CODEGEN *codegen, const NT_NODE *node, const bool needValue)
{
    assert(codegen);
    assert(node);

    const NT_TYPE *type =
        ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->left);

    switch (type->objectType)
    {
    case NT_TYPE_I32:
    case NT_TYPE_I64:
    case NT_TYPE_U32:
    case NT_TYPE_U64:
    case NT_TYPE_F32:
    case NT_TYPE_F64:
    case NT_TYPE_STRING: {
        const size_t callArgsCount = ntListLen(node->data);
        if (callArgsCount > 1)
        {
            ntErrorAtNode(&codegen->report, node, "Cast operator accepts only one argument");
            return NULL;
        }
        else if (callArgsCount == 0)
        {
            ntErrorAtNode(&codegen->report, node, "Cast operator needs one argument");
            return NULL;
        }

        NT_NODE *arg;
        bool result = ntListGet(node->data, 0, (void **)&arg);
        assert(result);

        const NT_TYPE *fromType =
            ntEvalExprType(codegen->context, &codegen->report, codegen->scope, (NT_NODE *)arg);
        NIR_VALUE *value = expression(codegen, arg, true);

        return cast(codegen, node, value, fromType, type);
    }
    case NT_TYPE_DELEGATE: {
        const bool hasReturn = type->delegate.returnType != NULL;

        // if is subroutine and need a value as result, is a abstract tree error.
        if (needValue && !hasReturn)
        {
            char *name = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            ntErrorAtNode(&codegen->report, node, "A subroutine('%s') cannot return a value.",
                          name);
            ntFree(name);
            return NULL;
        }

        const size_t callArgsCount = ntListLen(node->data);
        if (callArgsCount != type->delegate.paramCount)
        {
            char *name = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            ntErrorAtNode(&codegen->report, node,
                          "The '%s' call has wrong number of parameters, expect number is "
                          "%d, not %d.",
                          name, type->delegate.paramCount, callArgsCount);
            ntFree(name);
            return NULL;
        }

        bool paramError = false;
        NIR_VALUE **valueArgs = ntMalloc(sizeof(NIR_VALUE *) * callArgsCount);
        for (size_t i = 0; i < callArgsCount; ++i)
        {
            const NT_NODE *arg = NULL;
            bool result = ntListGet(node->data, i, (void *)&arg);
            assert(result && arg);

            const NT_TYPE *paramType =
                ntEvalExprType(codegen->context, &codegen->report, codegen->scope, (NT_NODE *)arg);
            const NT_TYPE *expectType = type->delegate.params[i].type;

            valueArgs[i] = expression(codegen, arg, true);

            if (!ntTypeIsAssignableFrom(expectType, paramType))
            {
                char *expectTypeName = ntStringToChar(expectType->typeName);
                char *paramTypeName = ntStringToChar(paramType->typeName);
                char *paramName = ntStringToChar(type->delegate.params[i].name);

                ntErrorAtNode(&codegen->report, arg,
                              "The argument('%s', %d) expect a value of type '%s', not '%s'.",
                              paramName, i, expectTypeName, paramTypeName);

                ntFree(expectTypeName);
                ntFree(paramTypeName);
                ntFree(paramName);
                paramError = true;
            }
        }

        if (paramError)
        {
            // free args
            ntFree(valueArgs);
            return NULL;
        }

        // emit function name
        NIR_FUNCTION *function = (NIR_FUNCTION *)expression(codegen, node->left, needValue);
        NIR_TYPE *ftype = toNirType(codegen->context, type);

        return nirCreateCall(ftype, function, callArgsCount, valueArgs, U"calltmp", codegen->block);
    }
    default:
        ntErrorAtNode(&codegen->report, node,
                      "Call only can be perform with a delegate or a type with cast support");
        return NULL;
    }
}

static NIR_VALUE *expression(NT_CODEGEN *codegen, const NT_NODE *node, const bool needValue)
{
    assert(node->type.class == NC_EXPR);

    switch (node->type.kind)
    {
    case NK_LITERAL:
        return literal(codegen, node);
    case NK_UNARY:
        return unary(codegen, node);
    case NK_BINARY:
        return binary(codegen, node);
    case NK_VARIABLE:
        return variable(codegen, node);
    case NK_ASSIGN:
        return assign(codegen, node);
    case NK_LOGICAL:
        return logical(codegen, node);
    case NK_GET:
        return get(codegen, node);
    case NK_CALL:
        return call(codegen, node, needValue);
    default: {
        char *lexeme = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        ntErrorAtNode(&codegen->report, node, "CODEGEN unrecognized expression. (Lexeme: %s)",
                      lexeme);
        ntFree(lexeme);
        return NULL;
    }
    }
}

static void expressionStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node);
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_EXPR);
    assert(node->left);
    assert(node->left->type.class == NC_EXPR);

    switch (node->left->type.kind)
    {
    case NK_LITERAL:
    case NK_BINARY:
    case NK_CONSTANT:
    case NK_VARIABLE:
    case NK_LOGICAL:
    warning:
        ntWarningAtNode(node, "Expression result unused.");
        break;
    case NK_UNARY:
        switch (node->left->token.id)
        {
        case OP_INC:
        case OP_DEC:
            goto warning;
        default:
            break;
        }
        break;
    default:
        break;
    }

    expression(codegen, node->left, false);
}

static void statement(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE **returnType);

/*
[if]

entry:
    ifcond = fcmp // some condition
    br ifcond, then, else
then:
    *then code*
    br ifcont
else:
    *else code*
    br ifcont
ifcont:
    *code*
*/
static void ifStatement(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE **returnType)
{
    const bool hasElse = node->right != NULL;

    const NT_TYPE *thenReturnType = NULL;
    const NT_TYPE *elseReturnType = NULL;

    NIR_BASIC_BLOCK *const entryBlock = codegen->block;

    NIR_VALUE *condition = expression(codegen, node->condition, true);
    const NT_TYPE *conditionType =
        ntEvalExprType(codegen->context, &codegen->report, NULL, node->condition);
    condition = typeToBool(codegen, node, conditionType, condition);

    NIR_BASIC_BLOCK *ifcontBlock = nirCreateBasicBlock(codegen->context, U"ifcont");

    // then:
    NIR_BASIC_BLOCK *thenBlock = nirCreateBasicBlock(codegen->context, U"then");
    nirInsertBlockInto(thenBlock, codegen->function);
    codegen->block = thenBlock;

    statement(codegen, node->left, &thenReturnType);
    if (!nirGetBlockTerminator(codegen->block))
        nirCreateBranch1(ifcontBlock, codegen->block);

    NIR_BASIC_BLOCK *elseBlock = ifcontBlock;
    if (hasElse)
    {
        // else:
        elseBlock = nirCreateBasicBlock(codegen->context, U"else");
        nirInsertBlockInto(elseBlock, codegen->function);
        codegen->block = elseBlock;

        statement(codegen, node->right, &elseReturnType);
        if (!nirGetBlockTerminator(codegen->block))
            nirCreateBranch1(ifcontBlock, codegen->block);

        if (thenReturnType && elseReturnType && thenReturnType != elseReturnType)
        {
            char *expect = ntStringToChar((*returnType)->typeName);
            char *current = ntStringToChar(elseReturnType->typeName);
            ntErrorAtNode(&codegen->report, node,
                          "The else branch expect '%s' type as return, but is '%s'.", expect,
                          current);
            ntFree(expect);
            ntFree(current);
        }
    }
    nirCreateBranch2(thenBlock, elseBlock, condition, entryBlock);

    nirInsertBlockInto(ifcontBlock, codegen->function);
    codegen->block = ifcontBlock;

    if (thenReturnType && elseReturnType && !*returnType)
        *returnType = thenReturnType;
}

static void blockStatment(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE **returnType)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_BLOCK);

    const NT_TYPE *blockReturnType = NULL;
    beginScope(codegen, STT_NONE);
    const NT_SCOPE *const scope = codegen->scope;
    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        const NT_NODE *stmt = NULL;
        const bool result = ntListGet(node->data, i, (void **)&stmt);
        assert(result);

        statement(codegen, stmt, &blockReturnType);
    }
    if (*returnType == NULL)
        *returnType = blockReturnType;

    if (scope == codegen->scope)
        endScope(codegen, node);
}

/*
[loop]

entry:
loop:
    loopcond = fcmp // some condition
    br loopcond, loopcont, loopend
loopcont:
    *loop code*
    br loop
loopend:
    *code*
*/
static void conditionalLoopStatement(NT_CODEGEN *codegen, const NT_NODE *node, bool isWhile,
                                     const NT_TYPE **returnType)
{
    beginScope(codegen, STT_BREAKABLE);

    NIR_BASIC_BLOCK *loopBlock = nirCreateBasicBlock(codegen->context, U"loop");
    nirCreateBranch1(loopBlock, codegen->block);
    NIR_BASIC_BLOCK *continueBlock = nirCreateBasicBlock(codegen->context, U"loopcont");
    NIR_BASIC_BLOCK *endBlock = nirCreateBasicBlock(codegen->context, U"loopend");

    codegen->scope->loop = loopBlock;
    codegen->scope->endLoop = endBlock;

    // loop:
    nirInsertBlockInto(loopBlock, codegen->function);
    codegen->block = loopBlock;
    NIR_VALUE *condition = expression(codegen, node->condition, true);
    const NT_TYPE *conditionType =
        ntEvalExprType(codegen->context, &codegen->report, NULL, (NT_NODE *)node->condition);
    condition = typeToBool(codegen, node->condition, conditionType, condition);

    if (isWhile)
        nirCreateBranch2(continueBlock, endBlock, condition, loopBlock);
    else
        nirCreateBranch2(endBlock, continueBlock, condition, loopBlock);

    // loopcont:
    nirInsertBlockInto(continueBlock, codegen->function);
    codegen->block = continueBlock;
    statement(codegen, node->left, returnType);

    if (!nirGetBlockTerminator(codegen->block))
        nirCreateBranch1(loopBlock, codegen->block);

    // loopend:
    nirInsertBlockInto(endBlock, codegen->function);
    codegen->block = endBlock;

    endScope(codegen, node);
}

static void untilStatment(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_UNTIL);

    const NT_TYPE *returnType = NULL;
    conditionalLoopStatement(codegen, node, false, &returnType);
}

static void whileStatment(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_WHILE);

    const NT_TYPE *returnType = NULL;
    conditionalLoopStatement(codegen, node, true, &returnType);
}

static void declareVariable(NT_CODEGEN *codegen, const NT_NODE *node)
{
    const NT_TYPE *type = NULL;
    if (node->left != NULL)
    {
        type = findType(codegen, node->left);
        if (node->right)
        {
            const NT_TYPE *initType =
                ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->right);
            if (type != initType)
            {
                ntErrorAtNode(&codegen->report, node,
                              "Invalid initalizer type. Incompatible type!");
                return;
            }
        }
    }
    else
    {
        if (node->right == NULL)
        {
            ntErrorAtNode(&codegen->report, node,
                          "Variable declarations must has a type or initializer.");
            return;
        }
        type = ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->right);
    }

    NIR_VALUE *count = nirGetInt(nirGetInt32Type(codegen->context), 1, false);
    NIR_TYPE *nirType = toNirType(codegen->context, type);
    NIR_TYPE *ptrType = nirGetPointerTo(nirType);

    char_t *name = (char_t *)ntMalloc(sizeof(char_t) * (node->token.lexemeLength + 1));
    ntMemcpy(name, node->token.lexeme, sizeof(char_t) * node->token.lexemeLength);
    name[node->token.lexemeLength] = U'\0';

    NIR_VALUE *variable =
        nirCreateUnaryInst(NIR_UNARY_MEMORY_ALLOCA, ptrType, count, name, codegen->block);

    if (node->right)
    {
        NIR_VALUE *initialValue = expression(codegen, node->right, true);
        nirCreateStore(initialValue, variable, codegen->block);
    }

    NT_STRING *varName = ntTakeString(name, node->token.lexemeLength);
    addLocal(codegen, varName, type, variable);
}

static void varStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    ensureStmt(node, NK_VAR);
    declareVariable(codegen, node);
}

static void endFunctionScope(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE **returnType,
                             bool isEndScope)
{
    const NT_SCOPE *functionScope = codegen->scope;
    while (!(functionScope->type & (STT_FUNCTION | STT_METHOD)))
    {
        functionScope = functionScope->parent;
    }

    NIR_VALUE *returnValue = NULL;

    if (functionScope->type & STT_FUNCTION)
    {
        if (node->left == NULL)
        {
            ntErrorAtNode(&codegen->report, node,
                          "Critical codegen error! The scope need a return value.");
        }
        assert(node->left);

        const NT_TYPE *type =
            ntEvalExprType(codegen->context, &codegen->report, codegen->scope, node->left);
        if (codegen->scope->scopeReturnType == NULL ||
            codegen->scope->scopeReturnType == ntUndefinedType())
            codegen->scope->scopeReturnType = type;

        assert(codegen->scope->scopeReturnType == type);

        returnValue = expression(codegen, node->left, true);

        if (returnType && *returnType == NULL)
            *returnType = type;
    }

    if (!nirGetBlockTerminator(codegen->block))
        nirCreateReturn(returnValue, codegen->block);

    if (isEndScope)
    {
        while (!(codegen->scope->type & (STT_FUNCTION | STT_METHOD)))
        {
            NT_SCOPE *oldScope = codegen->scope;
            codegen->scope = (NT_SCOPE *)oldScope->parent;
            ntFreeSymbolTable(oldScope);
        }
        codegen->scope = (NT_SCOPE *)functionScope->parent;
    }
}

static NIR_FUNCTION *addFunction(NT_CODEGEN *codegen, NT_STRING *name, NT_SYMBOL_TYPE symbolType,
                                 const NT_TYPE *delegateType, bool public)
{
    assert(codegen);
    assert(codegen->module);
    assert(name);
    assert(delegateType);
    assert(IS_VALID_TYPE(delegateType));
    assert(((symbolType & SYMBOL_TYPE_FUNCTION) == SYMBOL_TYPE_FUNCTION) ||
           ((symbolType & SYMBOL_TYPE_SUBROUTINE) == SYMBOL_TYPE_SUBROUTINE));

    NIR_TYPE *functionType = toNirType(codegen->context, delegateType);

    const bool anon = name == NULL;
    if (anon)
        name = nirGetPrefixedId(codegen->context, U"@anonymous");

    NIR_FUNCTION *function =
        nirGetOrInsertFunction(codegen->module, ntStringChars(name, NULL), functionType);
    assert(function);
    codegen->function = function;

    NIR_BASIC_BLOCK *entryBlock = nirCreateBasicBlock(codegen->context, U"entry");
    codegen->block = entryBlock;
    nirInsertBlockInto(entryBlock, function);

    const NT_SYMBOL entry = {
        .symbol_name = name,
        .type = symbolType | (public ? SYMBOL_TYPE_PUBLIC : SYMBOL_TYPE_PRIVATE),
        .exprType = delegateType,
        .weak = false,
        .value = (NIR_VALUE *)function,
    };
    const bool result = ntInsertSymbol((NT_SCOPE *)codegen->scope->parent, &entry);
    assert(result);

    return function;
}

static void declareFunction(NT_CODEGEN *codegen, const NT_NODE *node, const bool returnValue)
{
    const char_t *name = node->token.lexeme;
    const size_t nameLen = node->token.lexemeLength;

    const NT_SYMBOL_TYPE symbolType = returnValue ? SYMBOL_TYPE_FUNCTION : SYMBOL_TYPE_SUBROUTINE;
    bool hasReturn = false;

    beginScope(codegen, returnValue ? STT_FUNCTION : STT_METHOD);

    const NT_LIST params = node->data;
    const size_t paramCount = ntListLen(node->data);

    NT_ARRAY paramsArray;
    ntInitArray(&paramsArray);
    const NT_TYPE *returnType = NULL;

    if (returnValue)
    {
        if (node->left)
            returnType = findType(codegen, node->left);
        else
            returnType = ntEvalBlockReturnType(codegen->context, &codegen->report, codegen->scope,
                                               node->right);
    }
    else
        returnType = ntVoidType();

    for (size_t i = 0; i < paramCount; ++i)
    {
        NT_NODE *paramNode = NULL;
        const bool result = ntListGet(params, i, (void **)&paramNode);
        assert(result && paramNode);

        const NT_NODE *typeNode = paramNode->left;
        const NT_TYPE *type = findType(codegen, typeNode);
        NT_STRING *paramName = ntCopyString(paramNode->token.lexeme, paramNode->token.lexemeLength);

        const NT_PARAM param = {
            .name = paramName,
            .type = type,
        };
        ntArrayAdd(&paramsArray, &param, sizeof(NT_PARAM));
    }

    const NT_TYPE *delegateType =
        ntTakeDelegateType(returnType, paramCount, (NT_PARAM *)paramsArray.data);
    NT_STRING *funcName = ntCopyString(name, nameLen);

    NIR_FUNCTION *function =
        addFunction(codegen, funcName, symbolType, delegateType, codegen->public);
    for (size_t i = 0; i < paramCount; ++i)
    {
        NT_PARAM param;
        const bool result = ntArrayGet(&paramsArray, i * sizeof(NT_PARAM), &param,
                                       sizeof(NT_PARAM)) == sizeof(NT_PARAM);
        assert(result);

        NIR_VALUE *paramValue = nirGetParamValue(function, i);
        addParam(codegen, param.name, param.type, paramValue);
    }

    const NT_TYPE *statmentReturn = NULL;
    for (size_t i = 0; i < ntListLen(node->right->data); ++i)
    {
        const NT_NODE *stmt = NULL;
        const bool result = ntListGet(node->right->data, i, (void **)&stmt);
        assert(result && stmt);

        statement(codegen, stmt, &statmentReturn);
        if (statmentReturn != NULL)
            hasReturn |= true;
    }

    ntDeinitArray(&paramsArray);

    if (statmentReturn != NULL && statmentReturn != returnType)
    {
        char *expect = ntStringToChar(returnType->typeName);
        char *result = ntStringToChar(statmentReturn->typeName);

        ntErrorAtNode(&codegen->report, node, "Incompatible return type! Expect %s, but return %s.",
                      expect, result);

        ntFree(expect);
        ntFree(result);
    }

    if (returnValue)
    {
        if (!hasReturn)
        {
            char *lname = ntToCharFixed(name, nameLen);
            ntErrorAtNode(&codegen->report, node,
                          "Function '%s' doesn't  return a value on all code paths.", lname);
            ntFree(lname);

            assert(!(codegen->scope->type & STT_METHOD));
            endScope(codegen, node);

            NIR_VALUE *terminator = nirGetBlockTerminator(codegen->block);
            if (terminator == NULL || nirGetOpcode(terminator) != NIR_TERM_RET)
                nirCreateReturn(NULL, codegen->block);
        }
        else
        {
            endScope(codegen, node);
            NIR_VALUE *terminator = nirGetBlockTerminator(codegen->block);
            if (terminator == NULL || nirGetOpcode(terminator) != NIR_TERM_RET)
                nirCreateReturn(NULL, codegen->block);
        }
    }
    else
    {
        endFunctionScope(codegen, node, NULL, true);

        NIR_VALUE *terminator = nirGetBlockTerminator(codegen->block);
        if (terminator == NULL || nirGetOpcode(terminator) != NIR_TERM_RET)
            nirCreateReturn(NULL, codegen->block);
    }
}

static void defStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    ensureStmt(node, NK_DEF);
    declareFunction(codegen, node, true);
}

static void subStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    ensureStmt(node, NK_SUB);
    declareFunction(codegen, node, false);
}

static void declaration(NT_CODEGEN *codegen, const NT_NODE *node)
{
    switch (node->type.kind)
    {
    case NK_DEF:
        defStatement(codegen, node);
        break;
    case NK_SUB:
        subStatement(codegen, node);
        break;
    case NK_VAR:
        varStatement(codegen, node);
        break;
    case NK_IMPORT:
        break;
    default:
        ntErrorAtNode(&codegen->report, node, "Expect a declaration");
        break;
    }
}

static void returnStatement(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE **returnType)
{
    ensureStmt(node, NK_RETURN);
    endFunctionScope(codegen, node, returnType, false);
}

static void breakStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    ensureStmt(node, NK_BREAK);

    const NT_SCOPE *breakScope = codegen->scope;
    while (breakScope && !(breakScope->type & STT_BREAKABLE))
    {
        breakScope = breakScope->parent;
    }

    if (!breakScope)
    {
        ntErrorAtNode(&codegen->report, node,
                      "Invalid break statement, break is not in a breakable scope!");
        return;
    }

    assert(breakScope->endLoop);
    nirCreateBranch1(breakScope->endLoop, codegen->block);
}

static void continueStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    ensureStmt(node, NK_CONTINUE);

    const NT_SCOPE *continueScope = codegen->scope;
    while (continueScope && !(continueScope->type & STT_BREAKABLE))
    {
        continueScope = continueScope->parent;
    }

    if (!continueScope)
    {
        ntErrorAtNode(&codegen->report, node,
                      "Invalid break statement, break is not in a breakable scope!");
        return;
    }

    assert(continueScope->loop);
    nirCreateBranch1(continueScope->loop, codegen->block);
}

static void statement(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE **returnType)
{
    if (node->type.class != NC_STMT)
    {
        ntErrorAtNode(&codegen->report, node, "Invalid node, the node must be a statment!");
        return;
    }

    assert(returnType);

    if (*returnType)
        ntWarningAtNode(node, "Unreachable code!");

    switch (node->type.kind)
    {
    case NK_EXPR:
        expressionStatement(codegen, node);
        break;
    case NK_IF:
        ifStatement(codegen, node, returnType);
        break;
    case NK_BLOCK:
        blockStatment(codegen, node, returnType);
        break;
    case NK_UNTIL:
        untilStatment(codegen, node);
        break;
    case NK_WHILE:
        whileStatment(codegen, node);
        break;
    case NK_VAR:
        varStatement(codegen, node);
        break;
    case NK_RETURN:
        returnStatement(codegen, node, returnType);
        break;
    case NK_BREAK:
        breakStatement(codegen, node);
        break;
    case NK_CONTINUE:
        continueStatement(codegen, node);
        break;
    default: {
        const char *const label = ntGetKindLabel(node->type.kind);
        ntErrorAtNode(&codegen->report, node,
                      "Invalid statment. The statement with kind '%s' is invalid.", label);
        break;
    }
    }
}

static NIR_MODULE *module(NIR_CONTEXT *context, const NT_NODE *node)
{
    assert(node);
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_MODULE);

    NT_STRING *name = ntCopyString(node->token.lexeme, node->token.lexemeLength);
    NIR_MODULE *module = nirCreateModule(ntStringChars(name, NULL));
    assert(module);

    NT_TYPE *tmodule = ntTakeType(NT_TYPE_MODULE, name, ntObjectType());
    NT_CODEGEN *codegen = createCodegen(context, module, tmodule);
    assert(codegen);

    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        const NT_NODE *stmt = NULL;
        const bool result = ntListGet(node->data, i, (void **)&stmt);
        assert(result);

        if (stmt->type.class == NC_STMT && stmt->type.kind == NK_PUBLIC)
            codegen->public = true;
        else if (stmt->type.class == NC_STMT && stmt->type.kind == NK_PRIVATE)
            codegen->public = false;
        declaration(codegen, stmt);
    }

    if (codegen->report.had_error)
    {
        nirDestroyModule(module);
        module = NULL;
    }

    freeCodegen(codegen);

    return module;
}

NIR_MODULE *ntNirGen(NIR_CONTEXT *context, const NT_NODE *moduleNode)
{
    assert(context);
    assert(moduleNode);

    return module(context, moduleNode);
}
