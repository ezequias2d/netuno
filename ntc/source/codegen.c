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
#include "codegen.h"
#include "parser.h"
#include "report.h"
#include "resolver.h"
#include "scanner.h"
#include <assert.h>
#include <netuno/delegate.h>
#include <netuno/memory.h>
#include <netuno/module.h>
#include <netuno/object.h>
#include <netuno/opcode.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/symbol.h>
#include <netuno/type.h>
#include <netuno/varint.h>
#include <stdarg.h>
#include <stdio.h>

static void addType(NT_MODGEN *modgen, const NT_TYPE *type)
{
    NT_SYMBOL_ENTRY entry = {
        .symbol_name = type->typeName,
        .type = SYMBOL_TYPE_TYPE,
        .data = 0,
        .exprType = type,
        .weak = false,
    };
    ntInsertSymbol(modgen->scope, &entry);
}

NT_MODGEN *ntCreateModgen(NT_CODEGEN *codegen, NT_MODULE *module)
{
    NT_MODGEN *modgen = (NT_MODGEN *)ntMalloc(sizeof(NT_MODGEN));

    modgen->module = module;
    modgen->scope = (NT_SYMBOL_TABLE *)&module->type.fields;
    modgen->functionScope = modgen->scope;
    modgen->stack = ntCreateVStack();
    modgen->report.had_error = false;
    modgen->codegen = codegen;
    modgen->public = false;

    addType(modgen, ntI32Type());
    addType(modgen, ntI64Type());
    addType(modgen, ntU32Type());
    addType(modgen, ntU64Type());
    addType(modgen, ntF32Type());
    addType(modgen, ntF64Type());
    addType(modgen, ntStringType());

    return modgen;
}

NT_CODEGEN *ntCreateCodegen(NT_ASSEMBLY *assembly)
{
    NT_CODEGEN *codegen = (NT_CODEGEN *)ntMalloc(sizeof(NT_CODEGEN));
    codegen->assembly = assembly;
    return codegen;
}

void ntFreeCodegen(NT_CODEGEN *codegen)
{
    if (codegen)
    {
        ntFree(codegen);
    }
}

void ntFreeModgen(NT_MODGEN *modgen)
{
    if (modgen)
    {
        ntFreeVStack(modgen->stack);
        ntFree(modgen);
    }
}

static size_t emit(NT_MODGEN *modgen, const NT_NODE *node, uint8_t value)
{
    return ntWriteModule(modgen->module, value, node->token.line);
}

static size_t push(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE *type)
{
    assert(type);
    assert(node);
    return ntVPush(modgen->stack, type);
}

static size_t pop(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE *type)
{
    const NT_TYPE *popType = NULL;
    const size_t result = ntVPop(modgen->stack, &popType);

    if (popType != type)
    {
        ntErrorAtNode(
            &modgen->report, node,
            "Sometime are wrong in modgen, the popped type dont match with pop type are wrong.");
    }

    return result;
}

static void emitConstantI32(NT_MODGEN *modgen, const NT_NODE *node, uint32_t value)
{
    switch (value)
    {
    case 0:
        emit(modgen, node, BC_ZERO_32);
        break;
    case 1:
        emit(modgen, node, BC_ONE_32);
        break;
    default:
        emit(modgen, node, BC_CONST_32);
        const uint64_t index = ntAddConstant32(modgen->module, value);
        ntWriteModuleVarint(modgen->module, index, node->token.line);
        break;
    }
    push(modgen, node, ntI32Type());
}

static void emitConstantI64(NT_MODGEN *modgen, const NT_NODE *node, uint64_t value)
{
    switch (value)
    {
    case 0:
        emit(modgen, node, BC_ZERO_64);
        break;
    case 1:
        emit(modgen, node, BC_ONE_64);
        break;
    default:
        emit(modgen, node, BC_CONST_64);
        const uint64_t index = ntAddConstant64(modgen->module, value);
        ntWriteModuleVarint(modgen->module, index, node->token.line);
        break;
    }
    push(modgen, node, ntI64Type());
}

static void emitConstantU32(NT_MODGEN *modgen, const NT_NODE *node, uint32_t value)
{
    switch (value)
    {
    case 0:
        emit(modgen, node, BC_ZERO_32);
        break;
    case 1:
        emit(modgen, node, BC_ONE_32);
        break;
    default:
        emit(modgen, node, BC_CONST_32);
        const uint64_t index = ntAddConstant32(modgen->module, value);
        ntWriteModuleVarint(modgen->module, index, node->token.line);
        break;
    }
    push(modgen, node, ntU32Type());
}

static void emitConstantU64(NT_MODGEN *modgen, const NT_NODE *node, uint64_t value)
{
    switch (value)
    {
    case 0:
        emit(modgen, node, BC_ZERO_64);
        break;
    case 1:
        emit(modgen, node, BC_ONE_64);
        break;
    default:
        emit(modgen, node, BC_CONST_64);
        const uint64_t index = ntAddConstant64(modgen->module, value);
        ntWriteModuleVarint(modgen->module, index, node->token.line);
        break;
    }
    push(modgen, node, ntU64Type());
}

static void emitConstantF32(NT_MODGEN *modgen, const NT_NODE *node, uint32_t value)
{
    emit(modgen, node, BC_CONST_32);
    const uint64_t index = ntAddConstant32(modgen->module, value);
    ntWriteModuleVarint(modgen->module, index, node->token.line);
    push(modgen, node, ntF32Type());
}

static void emitConstantF64(NT_MODGEN *modgen, const NT_NODE *node, uint64_t value)
{
    emit(modgen, node, BC_CONST_64);
    const uint64_t index = ntAddConstant64(modgen->module, value);
    ntWriteModuleVarint(modgen->module, index, node->token.line);
    push(modgen, node, ntF64Type());
}

static void emitConstantObject(NT_MODGEN *modgen, const NT_NODE *node, NT_OBJECT *object)
{
    emit(modgen, node, BC_CONST_OBJECT);
    const uint64_t index = ntAddConstantObject(modgen->codegen->assembly, object);
    ntWriteModuleVarint(modgen->module, index, node->token.line);
    push(modgen, node, object->type);
}

static size_t emitPop(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE *type)
{
    switch (type->objectType)
    {
    case NT_OBJECT_STRING:
    case NT_OBJECT_CUSTOM:
    case NT_OBJECT_I32:
    case NT_OBJECT_U32:
    case NT_OBJECT_F32:
        emit(modgen, node, BC_POP_32);
        break;
    case NT_OBJECT_F64:
    case NT_OBJECT_U64:
    case NT_OBJECT_I64:
        emit(modgen, node, BC_POP_64);
        break;
    default:
        ntWarningAtNode(node, "Invalid objectType pop.");
        return 0;
    }
    return pop(modgen, node, type);
}

static void vFixedPop(NT_MODGEN *modgen, const size_t popSize)
{
    if (popSize == 0)
        return;

    assert(popSize % sizeof(uint32_t) == 0);

    int64_t rem = (int64_t)popSize;
    while (rem > 0)
    {
        const NT_TYPE *poppedType = NULL;
        ntVPop(modgen->stack, &poppedType);
        rem -= poppedType->stackSize;
    }
}

static void emitPartialFixedPop(NT_MODGEN *modgen, const NT_NODE *node, const size_t popSize)
{
    if (popSize == 0)
        return;

    assert(popSize % sizeof(uint32_t) == 0);

    int64_t rem = (int64_t)popSize;
    size_t index = 0;
    while (rem > 0)
    {
        const NT_TYPE *poppedType = NULL;
        ntVPeek(modgen->stack, &poppedType, index++);
        rem -= poppedType->stackSize;
    }

    // value partial popped if false
    assert(rem == 0);

    emit(modgen, node, BC_POP);
    ntWriteModuleVarint(modgen->module, popSize / sizeof(uint32_t), node->token.line);
}

static size_t emitFixedPop(NT_MODGEN *modgen, const NT_NODE *node, const size_t popSize)
{
    if (popSize == 0)
        return 0;

    assert(popSize % sizeof(uint32_t) == 0);

    int64_t rem = (int64_t)popSize;
    size_t sp;
    while (rem > 0)
    {
        const NT_TYPE *poppedType = NULL;
        sp = ntVPop(modgen->stack, &poppedType);
        rem -= poppedType->stackSize;
    }

    // value partial popped if false
    assert(rem == 0);

    emit(modgen, node, BC_POP);
    ntWriteModuleVarint(modgen->module, popSize / sizeof(uint32_t), node->token.line);
    return sp;
}

static void ensureStmt(const NT_NODE *node, NT_NODE_KIND kind)
{
    assert(node->type.class == NC_STMT && node->type.kind == kind &&
           node->type.literalType == LT_NONE);
}

static void beginScope(NT_MODGEN *modgen, NT_SYMBOL_TABLE_TYPE type)
{
    modgen->scope = ntCreateSymbolTable(modgen->scope, type, (void *)modgen->stack->sp);

    if (type & (STT_FUNCTION | STT_METHOD))
        modgen->functionScope = modgen->scope;
}

static bool resolveBranchSymbol(NT_MODGEN *modgen, const NT_NODE *node,
                                NT_SYMBOL_ENTRY *const branchEntry, uint64_t *offset)
{
    assert(modgen);
    assert(node);
    assert(branchEntry);
    assert(offset);

    NT_SYMBOL_ENTRY label;
    // find label
    bool result = ntLookupSymbolCurrent(modgen->functionScope, branchEntry->target_label->chars,
                                        branchEntry->target_label->length, &label);
    if (!result)
    {
        char *labelName =
            ntToCharFixed(branchEntry->target_label->chars, branchEntry->target_label->length);
        ntErrorAtNode(&modgen->report, node, "Label '%s' was not reached", labelName);
        ntFree(labelName);
    }
    assert(result);
    assert((label.type & SYMBOL_TYPE_LABEL) == SYMBOL_TYPE_LABEL);

    const uint64_t boffset = label.data2 - ((size_t)branchEntry->data + *offset);
    const size_t size = ntVarintEncodedSize(ZigZagEncoding(boffset));
    *offset += size;
    if (size != branchEntry->data2)
    {
        branchEntry->data2 = size;
        ntUpdateSymbol(modgen->functionScope, branchEntry);
        return true;
    }
    return false;
}

static bool resolveLabelSymbol(NT_MODGEN *modgen, NT_SYMBOL_ENTRY *const labelSymbol,
                               const uint64_t *offset)
{
    assert(modgen);
    assert(labelSymbol);
    assert(offset);
    assert((labelSymbol->type & SYMBOL_TYPE_LABEL) == SYMBOL_TYPE_LABEL);

    const uint64_t boffset = (size_t)labelSymbol->data + *offset;

    if (boffset != labelSymbol->data2)
    {
        labelSymbol->data2 = boffset;
        ntUpdateSymbol(modgen->functionScope, labelSymbol);
        return true;
    }
    return false;
}

static void resolveLabelAndBranchSymbols(NT_MODGEN *modgen, const NT_NODE *node)
{
    NT_SYMBOL_TABLE *const fscope = modgen->functionScope;
    uint64_t offset;
    bool dirth = false;
    do
    {
        offset = 0;
        dirth = false;

        for (size_t i = 0; i < fscope->table->count; i += sizeof(NT_SYMBOL_ENTRY))
        {
            NT_SYMBOL_ENTRY current;
            const bool result = ntArrayGet(fscope->table, i, &current, sizeof(NT_SYMBOL_ENTRY)) ==
                                sizeof(NT_SYMBOL_ENTRY);
            assert(result);

            switch (current.type)
            {
            case SYMBOL_TYPE_LABEL:
                dirth |= resolveLabelSymbol(modgen, &current, &offset);
                break;
            case SYMBOL_TYPE_BRANCH:
                dirth |= resolveBranchSymbol(modgen, node, &current, &offset);
                break;
            default:
                break;
            }
        }
    } while (dirth);

    offset = 0;
    for (size_t i = 0; i < fscope->table->count; i += sizeof(NT_SYMBOL_ENTRY))
    {
        NT_SYMBOL_ENTRY current;
        const bool result = ntArrayGet(fscope->table, i, &current, sizeof(NT_SYMBOL_ENTRY)) ==
                            sizeof(NT_SYMBOL_ENTRY);
        assert(result);

        switch (current.type)
        {
        case SYMBOL_TYPE_BRANCH: {
            NT_SYMBOL_ENTRY label;
            const bool result =
                ntLookupSymbolCurrent(modgen->functionScope, current.target_label->chars,
                                      current.target_label->length, &label);
            if (!result)
            {
                char *labelName =
                    ntToCharFixed(current.target_label->chars, current.target_label->length);
                ntErrorAtNode(&modgen->report, node, "Label '%s' was not reached", labelName);
                ntFree(labelName);
            }
            assert(result);
            assert((label.type & SYMBOL_TYPE_LABEL) == SYMBOL_TYPE_LABEL);

            const uint64_t boffset = label.data2 - ((size_t)current.data + offset);
            const uint64_t z = ZigZagEncoding(boffset);
            assert(current.data2 == ntVarintEncodedSize(z));

            ntInsertModuleVarint(modgen->module, offset + 1 + (size_t)current.data, boffset);
            offset += current.data2;
        }
        break;
        default:
            break;
        }
    }
}

static void endScope(NT_MODGEN *modgen, const NT_NODE *node, bool emitPop)
{
    const size_t delta = modgen->stack->sp - (size_t)modgen->scope->data;
    if (emitPop)
    {
        emitFixedPop(modgen, node, delta);
    }
    else
    {
        int64_t rem = (int64_t)delta;
        while (rem > 0)
        {
            const NT_TYPE *poppedType = NULL;
            ntVPop(modgen->stack, &poppedType);
            rem -= poppedType->stackSize;
        }
    }

    NT_SYMBOL_TABLE *const oldScope = modgen->scope;
    modgen->scope = oldScope->parent;

    if (oldScope->type & (STT_FUNCTION | STT_METHOD))
    {
        modgen->functionScope = oldScope->parent;
        while (modgen->functionScope != NULL &&
               !(modgen->functionScope->type & (STT_FUNCTION | STT_METHOD)))
        {
            modgen->functionScope = modgen->functionScope->parent;
        }
    }
    ntFreeSymbolTable(oldScope);
}

static void addLocal(NT_MODGEN *modgen, const NT_STRING *name, const NT_TYPE *type)
{
    // TODO: check type in vstack
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_VARIABLE,
        .data = (void *)modgen->stack->sp,
        .exprType = type,
        .weak = false,
    };
    const bool result = ntInsertSymbol(modgen->scope, &entry);
    assert(result);
}

static void addParam(NT_MODGEN *modgen, const NT_STRING *name, const NT_TYPE *type)
{
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_PARAM,
        .data = (void *)modgen->stack->sp,
        .exprType = type,
        .weak = false,
    };
    const bool result = ntInsertSymbol(modgen->scope, &entry);
    assert(result);
}

static void addSymbol(NT_MODGEN *modgen, const NT_STRING *name, NT_SYMBOL_TYPE symbolType,
                      const NT_TYPE *type, void *data)
{
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = symbolType,
        .data = data,
        .exprType = type,
        .weak = false,
    };
    const bool result = ntInsertSymbol(modgen->scope, &entry);
    assert(result);
}

static void addLabel(NT_MODGEN *modgen, const NT_STRING *label)
{
    const size_t pc = modgen->module->code.count;
    ntRefObject((NT_OBJECT *)label);

    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = label,
        .type = SYMBOL_TYPE_LABEL,
        .data = (void *)pc,
        .data2 = pc,
        .exprType = NULL,
        .weak = false,
    };
    const bool result = ntInsertSymbol(modgen->functionScope, &entry);
    assert(result);
}

static const NT_STRING *genString(NT_MODGEN *modgen, const char_t *prefix)
{
    const uint64_t pc = modgen->module->code.count;
    const NT_STRING *str = ntU64Type()->string((NT_OBJECT *)&pc);
    const size_t strlen = ntStrLen(prefix);
    if (strlen == 0)
        return str;

    const NT_STRING *strPrefix = ntCopyString(prefix, strlen);
    const NT_STRING *label = ntConcat((NT_OBJECT *)strPrefix, (NT_OBJECT *)str);

    ntFreeObject((NT_OBJECT *)strPrefix);
    ntFreeObject((NT_OBJECT *)str);

    return label;
}

static const NT_STRING *genLabel(NT_MODGEN *modgen)
{
    const NT_STRING *label = genString(modgen, U":");
    addLabel(modgen, label);
    return label;
}

static void addBranch(NT_MODGEN *modgen, const NT_STRING *label)
{
    const size_t pc = modgen->module->code.count;
    ntRefObject((NT_OBJECT *)label);
    const NT_STRING *branchName = genString(modgen, U"#");
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = branchName,
        .target_label = label,
        .type = SYMBOL_TYPE_BRANCH,
        .data = (void *)pc,
        .data2 = pc,
        .weak = false,
    };
    const bool result = ntInsertSymbol(modgen->functionScope, &entry);
    assert(result);
}

static void emitBranchLabel(NT_MODGEN *modgen, const NT_NODE *node, NT_OPCODE branchOpcode,
                            const NT_STRING *label)
{
    addBranch(modgen, label);
    emit(modgen, node, branchOpcode);
}

static const NT_STRING *emitBranch(NT_MODGEN *modgen, const NT_NODE *node, NT_OPCODE branchOpcode)
{
    const NT_STRING *label = genString(modgen, U":");
    emitBranchLabel(modgen, node, branchOpcode, label);
    ntFreeObject((NT_OBJECT *)label);
    return label;
}

static void addFunction(NT_MODGEN *modgen, const NT_STRING *name, NT_SYMBOL_TYPE symbolType,
                        const NT_DELEGATE_TYPE *delegateType, size_t pc, bool public)
{
    assert(modgen);
    assert(modgen->module);
    assert(name);
    assert(IS_VALID_OBJECT(name));
    assert(delegateType);
    assert(IS_VALID_TYPE(delegateType));
    assert(((symbolType & SYMBOL_TYPE_FUNCTION) == SYMBOL_TYPE_FUNCTION) ||
           ((symbolType & SYMBOL_TYPE_SUBROUTINE) == SYMBOL_TYPE_SUBROUTINE));

    if (modgen->functionScope->parent != &modgen->module->type.fields)
    {
        ntAddModuleFunction(modgen->module, name, delegateType, pc, public);
        return;
    }

    const NT_DELEGATE *delegate =
        ntAddModuleFunction(modgen->module, name, delegateType, pc, public);
    addSymbol(modgen, name, symbolType, (const NT_TYPE *)delegateType, (void *)delegate);
}

static const NT_TYPE *findType(NT_MODGEN *modgen, const NT_NODE *typeNode)
{
    const NT_TOKEN *name = &typeNode->token;

    if (name->type == TK_KEYWORD)
    {
        // primitive
        switch (name->id)
        {
        case KW_TRUE:
        case KW_FALSE:
            return ntBoolType();
        case KW_I32:
            return ntI32Type();
        case KW_I64:
            return ntI64Type();
        case KW_U32:
            return ntU32Type();
        case KW_U64:
            return ntU64Type();
        case KW_F32:
            return ntF32Type();
        case KW_F64:
            return ntF64Type();
        case KW_STRING:
            return ntStringType();
        default: {
            char *typeLex = ntToChar(ntGetKeywordLexeme(typeNode->token.id));
            ntErrorAtNode(&modgen->report, typeNode, "The keyword '%s' is not a type.", typeLex);
            ntFree(typeLex);
            return ntErrorType();
        }
        }
    }
    else
    {
        // object
        NT_SYMBOL_ENTRY entry;
        if (!ntLookupSymbol(modgen->scope, name->lexeme, name->lexemeLength, NULL, &entry))
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            ntErrorAtNode(&modgen->report, typeNode, "The type '%s' don't exist.", lexeme);
            ntFree(lexeme);
            return NULL;
        }

        if (entry.type != SYMBOL_TYPE_TYPE)
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            ntErrorAtNode(&modgen->report, typeNode, "The identifier '%s' is not a type.", lexeme);
            ntFree(lexeme);
            return NULL;
        }
        return entry.exprType;
    }
}

static bool findSymbol(NT_MODGEN *modgen, const char_t *name, const size_t length,
                       NT_SYMBOL_ENTRY *pEntry)
{
    return ntLookupSymbol(modgen->scope, name, length, NULL, pEntry);
}

static void number(NT_MODGEN *modgen, const NT_NODE *node)
{
    char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);

    uint32_t u32;
    uint64_t u64;
    switch (node->token.type)
    {
    case TK_I32:
        sscanf(str, "%i", (int32_t *)&u32);
        break;
    case TK_U32:
        sscanf(str, "%u", &u32);
        break;
    case TK_F32:
        sscanf(str, "%f", (float *)&u32);
        break;
    case TK_I64:
        sscanf(str, "%li", (int64_t *)&u64);
        break;
    case TK_U64:
        sscanf(str, "%lu", &u64);
        break;
    case TK_F64:
        sscanf(str, "%lf", (double *)&u64);
        break;
    default:
        ntErrorAtNode(&modgen->report, node, "Invalid number token type! '%s'", node->token.type);
        break;
    }
    ntFree(str);

    switch (node->token.type)
    {
    case TK_I32:
        emitConstantI32(modgen, node, u32);
        break;
    case TK_U32:
        emitConstantU32(modgen, node, u32);
        break;
    case TK_F32:
        emitConstantF32(modgen, node, u32);
        break;
    case TK_I64:
        emitConstantI64(modgen, node, u64);
        break;
    case TK_U64:
        emitConstantU64(modgen, node, u64);
        break;
    case TK_F64:
        emitConstantF64(modgen, node, u64);
        break;
    default:
        ntErrorAtNode(&modgen->report, node, "Invalid number token type! '%s'", node->token.type);
        break;
    }
}

static void string(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_LITERAL &&
           node->type.literalType == LT_STRING);

    // remove quotes
    const char_t *start = node->token.lexeme + 1;
    size_t length = node->token.lexemeLength - 2;

    char_t *str = ntEscapeString(start, &length);
    const NT_STRING *string = ntTakeString(str, length);
    emitConstantObject(modgen, node, (NT_OBJECT *)string);
}

static void literal(NT_MODGEN *modgen, const NT_NODE *node)
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
            emit(modgen, node, BC_ZERO_32);
            push(modgen, node, ntBoolType());
            break;
        case KW_TRUE:
            emit(modgen, node, BC_ONE_32);
            push(modgen, node, ntBoolType());
            break;
        default: {
            char *lexeme = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            ntErrorAtNode(
                &modgen->report, node,
                "AST invalid format, node id of a bool literal must be TK_TRUE or TK_FALSE "
                "cannot be '%s'!",
                lexeme);
            ntFree(lexeme);
        }
        break;
        }
        break;
    case LT_NONE:
        emit(modgen, node, BC_ZERO_32);
        push(modgen, node, ntU32Type());
        break;
    case LT_STRING:
        string(modgen, node);
        break;
    case LT_I32:
    case LT_I64:
    case LT_U32:
    case LT_U64:
    case LT_F32:
    case LT_F64:
        number(modgen, node);
        break;
    default:
        ntErrorAtNode(&modgen->report, node, "Invalid literal type! '%d'", node->type.literalType);
        break;
    }
}

static void expression(NT_MODGEN *modgen, const NT_NODE *node, const bool needValue);

static void emitDuplicate(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE *type)
{
    switch (type->stackSize)
    {
    case sizeof(uint32_t):
        emit(modgen, node, BC_LOAD_SP_32);
        break;
    case sizeof(uint64_t):
        emit(modgen, node, BC_LOAD_SP_64);
        break;
    }

    ntWriteModuleVarint(modgen->module, 0, node->token.line);
    push(modgen, node, type);
}

static void emitAssign(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE *type,
                       const char_t *variableName, size_t variableNameLen, const char *message);

static void unary(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_UNARY);
    assert(node->token.type == TK_KEYWORD);
    assert((node->right != NULL) ^ (node->left != NULL));

    const NT_TYPE *type;

    if (node->left)
    {
        type = ntEvalExprType(&modgen->report, modgen->scope, node->left);

        const NT_NODE *identifier = node->left;
        assert(identifier);

        expression(modgen, node->left, true);
        emitDuplicate(modgen, node, type);

        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_ONE_32);
            switch (node->token.id)
            {
            case OP_INC:
                emit(modgen, node, BC_ADD_I32);
                break;
            case OP_DEC:
                emit(modgen, node, BC_SUB_I32);
                break;
            default:
                ntErrorAtNode(&modgen->report, node, "Invalid unary operation. %d(%c)",
                              node->token.id, (char)node->token.id);
                break;
            }
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_ONE_64);
            switch (node->token.id)
            {
            case OP_INC:
                emit(modgen, node, BC_ADD_I64);
                break;
            case OP_DEC:
                emit(modgen, node, BC_SUB_I64);
                break;
            default:
                ntErrorAtNode(&modgen->report, node, "Invalid unary operation. %d(%c)",
                              node->token.id, (char)node->token.id);
                break;
            }
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_ONE_F32);
            switch (node->token.id)
            {
            case OP_INC:
                emit(modgen, node, BC_ADD_F32);
                break;
            case OP_DEC:
                emit(modgen, node, BC_SUB_F32);
                break;
            default:
                ntErrorAtNode(&modgen->report, node, "Invalid unary operation. %d(%c)",
                              node->token.id, (char)node->token.id);
                break;
            }
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_ONE_F64);
            switch (node->token.id)
            {
            case OP_INC:
                emit(modgen, node, BC_ADD_F64);
                break;
            case OP_DEC:
                emit(modgen, node, BC_SUB_F64);
                break;
            default:
                ntErrorAtNode(&modgen->report, node, "Invalid unary operation. %d(%c)",
                              node->token.id, (char)node->token.id);
                break;
            }
            break;
        default: {
            char *str = ntToCharFixed(type->typeName->chars, type->typeName->length);
            ntErrorAtNode(&modgen->report, node,
                          "Invalid logical not('%s') operation with type '%s'.",
                          node->token.id == OP_INC ? "++" : "--", str);
            ntFree(str);
        }
        break;
        }

        emitAssign(modgen, node, type, identifier->token.lexeme, identifier->token.lexemeLength,
                   "The variable must be declared");
        emitPop(modgen, node, type);
        return;
    }

    type = ntEvalExprType(&modgen->report, modgen->scope, node->right);
    expression(modgen, node->right, true);
    pop(modgen, node, type);

    switch (node->token.id)
    {
    case '-':
        // negate
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_NEG_I32);
            push(modgen, node, ntI32Type());
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_NEG_I32);
            push(modgen, node, ntU32Type());
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_NEG_I64);
            push(modgen, node, ntI64Type());
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_NEG_I64);
            push(modgen, node, ntU64Type());
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_NEG_F32);
            push(modgen, node, ntF32Type());
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_NEG_F64);
            push(modgen, node, ntF64Type());
            break;
        case NT_OBJECT_STRING:
        case NT_OBJECT_CUSTOM:
        // TODO: call operator.
        default: {
            char *str = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid negate('-') operation with type '%s'.",
                          str);
            ntFree(str);
        }
        break;
        }
        break;
    case '!':
        // logical not
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_IS_ZERO_32);
            push(modgen, node, ntBoolType());
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_IS_ZERO_64);
            push(modgen, node, ntBoolType());
            break;
        default: {
            char *str = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node,
                          "Invalid logical not('!') operation with type '%s'.", str);
            ntFree(str);
            break;
        }
        }
        break;
    case '~':
        // bitwise not
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_NOT_32);
            push(modgen, node, ntI32Type());
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_NOT_32);
            push(modgen, node, ntU32Type());
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_NOT_64);
            push(modgen, node, ntI64Type());
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_NOT_64);
            push(modgen, node, ntU64Type());
            break;
        default: {
            char *str = ntToCharFixed(type->typeName->chars, type->typeName->length);
            ntErrorAtNode(&modgen->report, node,
                          "Invalid logical not('!') operation with type '%s'.", str);
            ntFree(str);
            break;
        }
        }
        break;
    case OP_INC: {
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_ONE_32);
            emit(modgen, node, BC_ADD_I32);
            push(modgen, node, ntI32Type());
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_ONE_32);
            emit(modgen, node, BC_ADD_I32);
            push(modgen, node, ntU32Type());
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_ONE_64);
            emit(modgen, node, BC_ADD_I64);
            push(modgen, node, ntI64Type());
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_ONE_64);
            emit(modgen, node, BC_ADD_I64);
            push(modgen, node, ntU64Type());
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_ONE_F32);
            emit(modgen, node, BC_ADD_F32);
            push(modgen, node, ntF32Type());
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_ONE_F64);
            emit(modgen, node, BC_ADD_F64);
            push(modgen, node, ntF64Type());
            break;
        default: {
            char *str = ntToCharFixed(type->typeName->chars, type->typeName->length);
            ntErrorAtNode(&modgen->report, node,
                          "Invalid logical not('++') operation with type '%s'.", str);
            ntFree(str);
            break;
        }
        }
        const NT_NODE *identifier = node->right;
        emitAssign(modgen, node, type, identifier->token.lexeme, identifier->token.lexemeLength,
                   "The variable must be declared");
        break;
    }
    case OP_DEC: {
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_ONE_32);
            emit(modgen, node, BC_SUB_I32);
            push(modgen, node, ntI32Type());
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_ONE_32);
            emit(modgen, node, BC_SUB_I32);
            push(modgen, node, ntU32Type());
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_ONE_64);
            emit(modgen, node, BC_SUB_I64);
            push(modgen, node, ntI64Type());
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_ONE_64);
            emit(modgen, node, BC_SUB_I64);
            push(modgen, node, ntU64Type());
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_ONE_F32);
            emit(modgen, node, BC_SUB_F32);
            push(modgen, node, ntF32Type());
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_ONE_F64);
            emit(modgen, node, BC_SUB_F64);
            push(modgen, node, ntF64Type());
            break;
        default: {
            char *str = ntToCharFixed(type->typeName->chars, type->typeName->length);
            ntErrorAtNode(&modgen->report, node,
                          "Invalid logical not('++') operation with type '%s'.", str);
            ntFree(str);
            break;
        }
        }

        const NT_NODE *identifier = node->right;
        emitAssign(modgen, node, type, identifier->token.lexeme, identifier->token.lexemeLength,
                   "The variable must be declared");
        break;
    }
    default:
        ntErrorAtNode(&modgen->report, node, "Invalid unary operation. %d(%c)", node->token.id,
                      (char)node->token.id);
        break;
    }
}

static void cast(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE *from, const NT_TYPE *to)
{
    pop(modgen, node, from);

    switch (from->objectType)
    {
    case NT_OBJECT_I32:
        switch (to->objectType)
        {
        case NT_OBJECT_I32:
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_EXTEND_I32);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_CONVERT_F32_I32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_CONVERT_F64_I32);
            break;
        case NT_OBJECT_STRING:
            emit(modgen, node, BC_CONVERT_STR_I32);
            break;
        default:
            goto error;
        }
        break;
    case NT_OBJECT_U32:
        switch (to->objectType)
        {
        case NT_OBJECT_U32:
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_EXTEND_U32);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_CONVERT_F32_U32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_CONVERT_F64_U32);
            break;
        case NT_OBJECT_STRING:
            emit(modgen, node, BC_CONVERT_STR_U32);
            break;
        default:
            goto error;
        }
        break;
    case NT_OBJECT_I64:
        switch (to->objectType)
        {
        case NT_OBJECT_I64:
            break;
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_WRAP_I64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_CONVERT_F32_I64);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_CONVERT_F64_I64);
            break;
        case NT_OBJECT_STRING:
            emit(modgen, node, BC_CONVERT_STR_I64);
            break;
        default:
            goto error;
        }
        break;
    case NT_OBJECT_U64:
        switch (to->objectType)
        {
        case NT_OBJECT_U64:
            break;
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_WRAP_I64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_CONVERT_F32_U64);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_CONVERT_F64_U64);
            break;
        case NT_OBJECT_STRING:
            emit(modgen, node, BC_CONVERT_STR_U64);
            break;
        default:
            goto error;
        }
        break;
    case NT_OBJECT_F32:
        switch (to->objectType)
        {
        case NT_OBJECT_F32:
            break;
        case NT_OBJECT_I32:
            emit(modgen, node, BC_TRUNCATE_I32_F32);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_TRUNCATE_U32_F32);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_TRUNCATE_I64_F32);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_TRUNCATE_U64_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_PROMOTE_F32);
            break;
        case NT_OBJECT_STRING:
            emit(modgen, node, BC_CONVERT_STR_F32);
            break;
        default:
            goto error;
        }
        break;
    case NT_OBJECT_F64:
        switch (to->objectType)
        {
        case NT_OBJECT_F64:
            break;
        case NT_OBJECT_I32:
            emit(modgen, node, BC_TRUNCATE_I32_F64);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_TRUNCATE_U32_F64);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_TRUNCATE_I64_F64);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_TRUNCATE_U64_F64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_DEMOTE_F64);
            break;
        case NT_OBJECT_STRING:
            emit(modgen, node, BC_CONVERT_STR_F64);
            break;
        default:
            goto error;
        }
        break;
    case NT_OBJECT_STRING:
        switch (to->objectType)
        {
        case NT_OBJECT_STRING:
            break;
        case NT_OBJECT_I32:
            emit(modgen, node, BC_CONVERT_I32_STR);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_CONVERT_U32_STR);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_CONVERT_I64_STR);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_CONVERT_U64_STR);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_CONVERT_F32_STR);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_CONVERT_F64_STR);
            break;
        default:
            goto error;
        }
        break;
    case NT_OBJECT_TYPE_TYPE:
    case NT_OBJECT_ASSEMBLY:
    case NT_OBJECT_MODULE:
    case NT_OBJECT_OBJECT:
    case NT_OBJECT_DELEGATE:
    case NT_OBJECT_CUSTOM:
        // TODO
    default:
        goto error;
    }

    push(modgen, node, to);
    return;
error : {

    // TODO: object cast operator?
    char *fromStr = ntToChar(from->typeName->chars);
    char *dstStr = ntToChar(to->typeName->chars);
    ntErrorAtNode(&modgen->report, node, "Invalid cast from '%s' to '%s'.", fromStr, dstStr);
    ntFree(fromStr);
    ntFree(dstStr);

    push(modgen, node, to);
}
}

static void binary(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_BINARY);
    assert(node->left != NULL && node->right != NULL);

    const NT_TYPE *leftType = ntEvalExprType(&modgen->report, modgen->scope, node->left);
    const NT_TYPE *rightType = ntEvalExprType(&modgen->report, modgen->scope, node->right);
    const NT_TYPE *type = leftType->objectType < rightType->objectType ? leftType : rightType;
    const bool isConcat = type == ntStringType();

    expression(modgen, node->left, true);

    // only cast if is not concat operation, because CONCAT instruction handles any object type
    if (!isConcat || !ntTypeIsAssignableFrom(ntObjectType(), leftType))
        cast(modgen, node->left, leftType, type);

    expression(modgen, node->right, true);
    // only cast if is not concat operation, because CONCAT instruction handles any object type
    if (!isConcat || !ntTypeIsAssignableFrom(ntObjectType(), rightType))
        cast(modgen, node->right, rightType, type);

    pop(modgen, node, type);
    pop(modgen, node, type);

    switch (node->token.id)
    {
    case OP_NE:
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_NE_32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_NE_64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_NE_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_NE_F64);
            break;
        case NT_OBJECT_STRING:
            switch (sizeof(NT_REF))
            {
            case sizeof(uint32_t):
                emit(modgen, node, BC_NE_32);
                break;
            case sizeof(uint64_t):
                emit(modgen, node, BC_NE_64);
                break;
            default:
                ntErrorAtNode(&modgen->report, node,
                              "Sometime are wrong with NT_REF inside compiler");
                break;
            }
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid != operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, ntBoolType());
        break;
    case OP_EQ:
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_EQ_32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_EQ_64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_EQ_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_EQ_F64);
            break;
        case NT_OBJECT_STRING:
            switch (sizeof(NT_REF))
            {
            case sizeof(uint32_t):
                emit(modgen, node, BC_EQ_32);
                break;
            case sizeof(uint64_t):
                emit(modgen, node, BC_EQ_64);
                break;
            default:
                ntErrorAtNode(&modgen->report, node,
                              "Sometime are wrong with NT_REF inside compiler");
                break;
            }
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid == operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, ntBoolType());
        break;
    case '>':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_GT_I32);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_GT_U32);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_GT_I64);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_GT_U64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_GT_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_GT_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid > operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, ntBoolType());
        break;
    case OP_GE:
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_GE_I32);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_GE_U32);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_GE_I64);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_GE_U64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_GE_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_GE_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid <= operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, ntBoolType());
        break;
    case '<':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_LT_I32);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_LT_U32);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_LT_I64);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_LT_U64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_LT_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_LT_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid < operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, ntBoolType());
        break;
    case OP_LE:
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_LE_I32);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_LE_U32);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_LE_I64);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_LE_U64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_LE_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_LE_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid < operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, ntBoolType());
        break;

    case '+':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_ADD_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_ADD_I64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_ADD_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_ADD_F64);
            break;
        case NT_OBJECT_TYPE_TYPE:
        case NT_OBJECT_STRING:
        case NT_OBJECT_MODULE:
        case NT_OBJECT_ASSEMBLY:
        case NT_OBJECT_DELEGATE:
        case NT_OBJECT_OBJECT:
        case NT_OBJECT_CUSTOM:
            // TODO: operator overloading
            emit(modgen, node, BC_CONCAT);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid + operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, type);
        break;
    case '-':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_SUB_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_SUB_I64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_SUB_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_SUB_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid - operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, type);
        break;
    case '*':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_MUL_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_MUL_I64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_MUL_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_MUL_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid * operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, type);
        break;
    case '/':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_DIV_I32);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_DIV_U32);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_DIV_I64);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_DIV_U64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_DIV_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_DIV_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid / operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, type);
        break;
    case '%':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(modgen, node, BC_REM_I32);
            break;
        case NT_OBJECT_U32:
            emit(modgen, node, BC_REM_U32);
            break;
        case NT_OBJECT_I64:
            emit(modgen, node, BC_REM_I64);
            break;
        case NT_OBJECT_U64:
            emit(modgen, node, BC_REM_U64);
            break;
        case NT_OBJECT_F32:
            emit(modgen, node, BC_REM_F32);
            break;
        case NT_OBJECT_F64:
            emit(modgen, node, BC_REM_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid / operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, type);
        break;
    case '|':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_OR_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_OR_I64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid | operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, type);
        break;
    case '&':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_AND_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_AND_I64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid & operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, type);
        break;
    case '^':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(modgen, node, BC_XOR_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(modgen, node, BC_XOR_I64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            ntErrorAtNode(&modgen->report, node, "Invalid ^ operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(modgen, node, type);
        break;
    default:
        ntErrorAtNode(&modgen->report, node, "Invalid binary operation. %d", node->token.id);
        break;
    }
}

static void variable(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_VARIABLE);

    NT_SYMBOL_ENTRY entry;
    if (!findSymbol(modgen, node->token.lexeme, node->token.lexemeLength, &entry))
    {
        ntErrorAtNode(&modgen->report, node, "The symbol must be declared.");
        return;
    }
    if (((entry.type & SYMBOL_TYPE_FUNCTION) == SYMBOL_TYPE_FUNCTION) ||
        ((entry.type & SYMBOL_TYPE_SUBROUTINE) == SYMBOL_TYPE_SUBROUTINE))
    {
        emitConstantObject(modgen, node, (NT_OBJECT *)entry.data);
        return;
    }
    assert((entry.type & SYMBOL_TYPE_VARIABLE) == SYMBOL_TYPE_VARIABLE ||
           (entry.type & SYMBOL_TYPE_PARAM) == SYMBOL_TYPE_PARAM);

    const NT_TYPE *type = entry.exprType;
    const uint64_t delta = modgen->stack->sp - (size_t)entry.data;

    switch (type->stackSize)
    {
    case sizeof(uint32_t):
        emit(modgen, node, BC_LOAD_SP_32);
        break;
    case sizeof(uint64_t):
        emit(modgen, node, BC_LOAD_SP_64);
        break;
    default:
        ntErrorAtNode(&modgen->report, node,
                      "The symbol value has a stack size incompatible with BC_LOAD_SP operation.");
        return;
    }
    ntWriteModuleVarint(modgen->module, delta, node->token.line);
    push(modgen, node, entry.exprType);
}

static void emitAssign32(NT_MODGEN *modgen, const NT_NODE *node, const char_t *variableName,
                         size_t variableNameLen, const char *message, ...)
{
    NT_SYMBOL_TABLE *table = NULL;
    NT_SYMBOL_ENTRY entry;
    if (!ntLookupSymbol(modgen->scope, variableName, variableNameLen, &table, &entry))
    {
        va_list vl;
        va_start(vl, message);
        ntVErrorAtNode(&modgen->report, node, message, vl);
        va_end(vl);
    }

    if (table->type & STT_TYPE)
    {
        NT_MODULE *module = (NT_MODULE *)table->data;

        assert(module);
        assert(IS_VALID_OBJECT(module));
        assert(module->type.objectType == NT_OBJECT_MODULE);

        // module variable
        emitConstantObject(modgen, node, (NT_OBJECT *)module);

        // TODO:
        assert(false);
    }
    else
    {
        // local variable
        const uint64_t delta = modgen->stack->sp - (size_t)entry.data;
        emit(modgen, node, BC_STORE_SP_32);
        const size_t offset = ntWriteModuleVarint(modgen->module, delta, node->token.line);
        assert(offset);
    }
}

static void emitAssign64(NT_MODGEN *modgen, const NT_NODE *node, const char_t *variableName,
                         size_t variableNameLen, const char *message, ...)
{
    NT_SYMBOL_ENTRY entry;
    if (!ntLookupSymbol(modgen->scope, variableName, variableNameLen, NULL, &entry))
    {
        va_list vl;
        va_start(vl, message);
        ntVErrorAtNode(&modgen->report, node, message, vl);
        va_end(vl);
    }

    const uint64_t delta = modgen->stack->sp - (size_t)entry.data;
    emit(modgen, node, BC_STORE_SP_64);
    const size_t offset = ntWriteModuleVarint(modgen->module, delta, node->token.line);
    assert(offset);
}

static void emitAssign(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE *type,
                       const char_t *variableName, size_t variableNameLen, const char *message)
{
    switch (type->stackSize)
    {
    case sizeof(uint32_t):
        emitAssign32(modgen, node, variableName, variableNameLen, message);
        break;
    case sizeof(uint64_t):
        emitAssign64(modgen, node, variableName, variableNameLen, message);
        break;
    default:
        ntErrorAtNode(&modgen->report, node, "INTERNAL ERROR: Invalid objectType field! %d",
                      type->objectType);
        break;
    }
}

static void assign(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(node != NULL);
    assert(modgen != NULL);
    assert(node->type.class == NC_EXPR && node->type.kind == NK_ASSIGN);
    assert(node->left->type.class == NC_EXPR && node->left->type.kind == NK_VARIABLE);

    const NT_TYPE *rightType = ntEvalExprType(&modgen->report, modgen->scope, node->right);

    expression(modgen, node->right, true);

    const NT_NODE *identifier = node->left;
    assert(identifier);

    NT_SYMBOL_ENTRY entry;
    if (!findSymbol(modgen, identifier->token.lexeme, identifier->token.lexemeLength, &entry))
    {
        ntErrorAtNode(&modgen->report, node, "The symbol must be declared.");
        return;
    }

    if (entry.exprType != rightType)
    {
        char *rightTypeName = ntToChar(rightType->typeName->chars);
        char *variableTypeName = ntToChar(entry.exprType->typeName->chars);
        ntErrorAtNode(&modgen->report, node,
                      "The variable type '%s' is incompatible with expression type '%s'.",
                      variableTypeName, rightTypeName);
        ntFree(rightTypeName);
        ntFree(variableTypeName);
        return;
    }

    emitAssign(modgen, node, rightType, node->left->token.lexeme, node->left->token.lexemeLength,
               "The symbol must be declared.");
}

static void typeToBool(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE *type)
{
    switch (type->objectType)
    {
    case NT_OBJECT_I32:
    case NT_OBJECT_U32:
        break;
    case NT_OBJECT_F64:
        emit(modgen, node, BC_IS_NOT_ZERO_F64);
        break;
    case NT_OBJECT_F32:
        emit(modgen, node, BC_IS_NOT_ZERO_F32);
        break;
    case NT_OBJECT_I64:
    case NT_OBJECT_U64:
        emit(modgen, node, BC_IS_NOT_ZERO_64);
        break;
    default: {
        char *typeStr = ntToChar(type->typeName->chars);
        ntErrorAtNode(&modgen->report, node, "Invalid implicit cast from type '%s' to 'bool'.",
                      typeStr);
        ntFree(typeStr);
        break;
    }
    }
    pop(modgen, node, type);
    push(modgen, node, ntBoolType());
}

static void logicalAnd(NT_MODGEN *modgen, const NT_NODE *node)
{
    // branch when left value is false
    const NT_STRING *falseBranch = emitBranch(modgen, node, BC_BRANCH_Z_32);

    // consume left 'true' value and check right value
    emitPop(modgen, node, ntBoolType());

    const NT_TYPE *right = ntEvalExprType(&modgen->report, modgen->scope, node->right);
    expression(modgen, node->right, true);
    typeToBool(modgen, node, right);

    addLabel(modgen, falseBranch);
}

static void logicalOr(NT_MODGEN *modgen, const NT_NODE *node)
{
    // branch to end, when first value is true
    const NT_STRING *trueBranch = emitBranch(modgen, node, BC_BRANCH_NZ_32);

    // consume left 'false' value
    emitPop(modgen, node, ntBoolType());

    // eval right
    const NT_TYPE *right = ntEvalExprType(&modgen->report, modgen->scope, node->right);
    expression(modgen, node->right, true);

    // logical is not zero
    switch (right->objectType)
    {
    case NT_OBJECT_I32:
    case NT_OBJECT_U32:
        break;
    case NT_OBJECT_I64:
    case NT_OBJECT_U64:
        emit(modgen, node, BC_IS_NOT_ZERO_64);
        push(modgen, node, ntBoolType());
        break;
    default:
        ntErrorAtNode(&modgen->report, node, "Invalid argument cast to bool.");
        break;
    }

    // true:
    addLabel(modgen, trueBranch);
}

static void logical(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(modgen);
    assert(node);
    assert(node->type.class == NC_EXPR && node->type.kind == NK_LOGICAL);

    const NT_TYPE *type = ntEvalExprType(&modgen->report, modgen->scope, node->left);
    expression(modgen, node->left, true);
    typeToBool(modgen, node, type);

    switch (node->token.id)
    {
    case OP_LOGAND:
        logicalAnd(modgen, node);
        break;
    case OP_LOGOR:
        logicalOr(modgen, node);
        break;
    default: {
        char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        ntErrorAtNode(&modgen->report, node, "Invalid logical operation with ID '%d'('%s').",
                      node->token.id, str);
        ntFree(str);
        break;
    }
    }
}

static void get(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(modgen);
    assert(node);
    assert(node->type.class == NC_EXPR);
    assert(node->type.kind == NK_GET);

    NT_NODE *current = node->left;
    NT_OBJECT *constantObject = NULL;

    NT_SYMBOL_ENTRY entry;
    entry.type = SYMBOL_TYPE_NONE;
    do
    {
        const bool result = ntLookupSymbol(modgen->scope, current->token.lexeme,
                                           current->token.lexemeLength, NULL, &entry);

        if (result && (entry.type &
                       (SYMBOL_TYPE_MODULE | SYMBOL_TYPE_FUNCTION | SYMBOL_TYPE_SUBROUTINE)) != 0)
        {
            constantObject = (NT_OBJECT *)entry.data;
            current = current->left;
        }
        else
            break;
    } while (current);

    if (entry.type == SYMBOL_TYPE_NONE)
    {
        ntErrorAtNode(&modgen->report, node->left, "Undeclared symbol");
        return;
    }

    if (constantObject->type->objectType == NT_OBJECT_TYPE_TYPE)
    {
        NT_TYPE *type = (NT_TYPE *)constantObject;
        const bool result = ntLookupSymbol(&type->fields, node->token.lexeme,
                                           node->token.lexemeLength, NULL, &entry);

        if (result && (entry.type &
                       (SYMBOL_TYPE_MODULE | SYMBOL_TYPE_FUNCTION | SYMBOL_TYPE_SUBROUTINE)) != 0)
        {
            constantObject = (NT_OBJECT *)entry.data;
            emitConstantObject(modgen, node, constantObject);
            return;
        }
    }

    if (constantObject)
    {
        emitConstantObject(modgen, node, constantObject);
    }

    // dynamic
    // TODO:
    assert(0);
}

static void call(NT_MODGEN *modgen, const NT_NODE *node, const bool needValue)
{
    assert(modgen);
    assert(node);

    const NT_TYPE *type = ntEvalExprType(&modgen->report, modgen->scope, node->left);

    switch (type->objectType)
    {
    case NT_OBJECT_I32:
    case NT_OBJECT_I64:
    case NT_OBJECT_U32:
    case NT_OBJECT_U64:
    case NT_OBJECT_F32:
    case NT_OBJECT_F64:
    case NT_OBJECT_STRING: {
        const size_t callArgsCount = ntListLen(node->data);
        if (callArgsCount > 1)
        {
            ntErrorAtNode(&modgen->report, node, "Cast operator accepts only one argument");
            return;
        }
        else if (callArgsCount == 0)
        {
            ntErrorAtNode(&modgen->report, node, "Cast operator needs one argument");
            return;
        }

        const NT_NODE *arg = (const NT_NODE *)ntListGet(node->data, 0);
        const NT_TYPE *fromType = ntEvalExprType(&modgen->report, modgen->scope, (NT_NODE *)arg);
        expression(modgen, arg, true);

        cast(modgen, node, fromType, type);
        return;
    }
    case NT_OBJECT_DELEGATE:
        break;
    default:
        ntErrorAtNode(&modgen->report, node,
                      "Call only can be perform with a delegate or a type with cast support");
        return;
    }

    assert(type->objectType == NT_OBJECT_DELEGATE);
    const NT_DELEGATE_TYPE *delegateType = (const NT_DELEGATE_TYPE *)type;
    const bool hasReturn = delegateType->returnType != NULL;

    // if is subroutine and need a value as result, is a abstract tree error.
    if (needValue && !hasReturn)
    {
        char *name = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        ntErrorAtNode(&modgen->report, node, "A subroutine('%s') cannot return a value.", name);
        ntFree(name);
        return;
    }

    const uint32_t sp = modgen->stack->sp;

    const size_t callArgsCount = ntListLen(node->data);
    if (callArgsCount != delegateType->paramCount)
    {
        char *name = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        ntErrorAtNode(&modgen->report, node,
                      "The '%s' call has wrong number of parameters, expect number is "
                      "%d, not %d.",
                      name, delegateType->paramCount, callArgsCount);
        ntFree(name);
        return;
    }

    bool paramError = false;
    for (size_t i = 0; i < callArgsCount; ++i)
    {
        const NT_NODE *arg = (const NT_NODE *)ntListGet(node->data, i);
        const NT_TYPE *paramType = ntEvalExprType(&modgen->report, modgen->scope, (NT_NODE *)arg);
        const NT_TYPE *expectType = delegateType->params[i].type;

        expression(modgen, arg, true);

        if (!ntTypeIsAssignableFrom(expectType, paramType))
        {
            char *expectTypeName =
                ntToCharFixed(expectType->typeName->chars, expectType->typeName->length);
            char *paramTypeName =
                ntToCharFixed(paramType->typeName->chars, paramType->typeName->length);
            char *paramName = ntToCharFixed(delegateType->params[i].name->chars,
                                            delegateType->params[i].name->length);

            ntErrorAtNode(&modgen->report, arg,
                          "The argument('%s', %d) expect a value of type '%s', not '%s'.",
                          paramName, i, expectTypeName, paramTypeName);

            ntFree(expectTypeName);
            ntFree(paramTypeName);
            ntFree(paramName);
            paramError = true;
        }
    }

    if (paramError)
        return;

    // emit function name
    expression(modgen, node->left, needValue);
    emit(modgen, node, BC_CALL);
    pop(modgen, node, (const NT_TYPE *)delegateType);
    if (delegateType->returnType)
        push(modgen, node, delegateType->returnType);

    size_t delta = modgen->stack->sp - sp;

    if (delta > 0 && delegateType->returnType != NULL)
    {
        // ensure that dont virtual pops the arg0, because is the return value.
        delta -= delegateType->returnType->stackSize;
    }
    vFixedPop(modgen, delta);
}

static void expression(NT_MODGEN *modgen, const NT_NODE *node, const bool needValue)
{
    assert(node->type.class == NC_EXPR);

    switch (node->type.kind)
    {
    case NK_LITERAL:
        literal(modgen, node);
        break;
    case NK_UNARY:
        unary(modgen, node);
        break;
    case NK_BINARY:
        binary(modgen, node);
        break;
    case NK_VARIABLE:
        variable(modgen, node);
        break;
    case NK_ASSIGN:
        assign(modgen, node);
        break;
    case NK_LOGICAL:
        logical(modgen, node);
        break;
    case NK_GET:
        get(modgen, node);
        break;
    case NK_CALL:
        call(modgen, node, needValue);
        break;
    default: {
        char *lexeme = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        ntErrorAtNode(&modgen->report, node, "CODEGEN unrecognized expression. (Lexeme: %s)",
                      lexeme);
        ntFree(lexeme);
        break;
    }
    }
}

static void expressionStatement(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(node);
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_EXPR);
    assert(node->left);
    assert(node->left->type.class == NC_EXPR);

    expression(modgen, node->left, false);
    if (node->left->expressionType)
        emitPop(modgen, node, node->left->expressionType);
}

static void statement(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE **returnType);

static const NT_STRING *emitCondition(NT_MODGEN *modgen, const NT_NODE *node, bool isZero,
                                      const NT_TYPE **pConditionType)
{
    assert(modgen);
    assert(node);
    assert(pConditionType);

    // emit condition and branch
    *pConditionType = ntEvalExprType(&modgen->report, modgen->scope, node->condition);
    expression(modgen, node->condition, true);

    switch ((*pConditionType)->objectType)
    {
    case NT_OBJECT_I32:
    case NT_OBJECT_U32:
        return emitBranch(modgen, node, isZero ? BC_BRANCH_Z_32 : BC_BRANCH_NZ_32);
    case NT_OBJECT_I64:
    case NT_OBJECT_U64:
        return emitBranch(modgen, node, isZero ? BC_BRANCH_Z_64 : BC_BRANCH_NZ_64);
    default:
        ntErrorAtNode(&modgen->report, node,
                      "Invalid expression, must evaluate to basic types like int, long, etc.");
        return NULL;
    }
}

static void ifStatement(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE **returnType)
{
    const bool hasElse = node->right != NULL;

    // emit condition and branch
    const NT_TYPE *conditionType;
    const NT_STRING *elseBranch = emitCondition(modgen, node, true, &conditionType);
    assert(conditionType);

    // if has else body, each body pops the condition from stack, otherwise, pop in end
    if (hasElse)
        emitPop(modgen, node, conditionType);

    const NT_TYPE *thenReturnType = NULL;
    const NT_TYPE *elseReturnType = NULL;

    // emit then body
    statement(modgen, node->left, &thenReturnType);

    // emit else body if exist
    if (hasElse)
    {
        // skip else body, when then body has taken
        const NT_STRING *skipElse = emitBranch(modgen, node, BC_BRANCH);

        // elseBranch:
        addLabel(modgen, elseBranch);

        // push in vstack for else branch
        push(modgen, node, conditionType);
        // each body pops the condition from stack
        emitPop(modgen, node, conditionType);

        // emit else body
        statement(modgen, node->right, &elseReturnType);
        if (*returnType == NULL)
            *returnType = elseReturnType;
        else if (elseReturnType && elseReturnType != *returnType)
        {
            char *expect =
                ntToCharFixed((*returnType)->typeName->chars, (*returnType)->typeName->length);
            char *current =
                ntToCharFixed(elseReturnType->typeName->chars, elseReturnType->typeName->length);
            ntErrorAtNode(&modgen->report, node,
                          "The else branch expect '%s' type as return, but is '%s'.", expect,
                          current);
            ntFree(expect);
            ntFree(current);
        }

        // skipElse:
        addLabel(modgen, skipElse);
    }
    else
    {
        // elseBranch:
        addLabel(modgen, elseBranch);

        // pop condition when no has else
        emitPop(modgen, node, conditionType);
    }

    if (thenReturnType && elseReturnType && !*returnType)
        *returnType = thenReturnType;
}

static void blockStatment(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE **returnType)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_BLOCK);

    const NT_TYPE *blockReturnType = NULL;
    beginScope(modgen, STT_NONE);
    const NT_SYMBOL_TABLE *const scope = modgen->scope;
    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        const NT_NODE *stmt = ntListGet(node->data, i);
        statement(modgen, stmt, &blockReturnType);
    }
    if (*returnType == NULL)
        *returnType = blockReturnType;

    if (scope == modgen->scope)
        endScope(modgen, node, true);
}

static void conditionalLoopStatement(NT_MODGEN *modgen, const NT_NODE *node, bool isZero,
                                     const NT_TYPE **returnType)
{
    beginScope(modgen, STT_BREAKABLE);

    // loop:
    const NT_STRING *loopLabel = genLabel(modgen);
    modgen->scope->loopLabel = loopLabel;

    const NT_STRING *breakLabel = genString(modgen, U"$");
    modgen->scope->breakLabel = breakLabel;

    // check condition
    const NT_TYPE *conditionType;
    const NT_STRING *exitLabel = emitCondition(modgen, node, isZero, &conditionType);
    assert(conditionType);

    // code block
    emitPop(modgen, node, conditionType);
    statement(modgen, node->left, returnType);

    // loop to start
    emitBranchLabel(modgen, node, BC_BRANCH, loopLabel);

    // exit:
    addLabel(modgen, exitLabel);

    // pop condition value from stach when false
    push(modgen, node, conditionType);
    emitPop(modgen, node, conditionType);

    // break:
    addLabel(modgen, breakLabel);

    // free label
    ntFreeObject((NT_OBJECT *)exitLabel);

    endScope(modgen, node, true);
}

static void untilStatment(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_UNTIL);

    const NT_TYPE *returnType = NULL;
    conditionalLoopStatement(modgen, node, false, &returnType);
}

static void whileStatment(NT_MODGEN *modgen, const NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_WHILE);

    const NT_TYPE *returnType = NULL;
    conditionalLoopStatement(modgen, node, true, &returnType);
}

static void declareVariable(NT_MODGEN *modgen, const NT_NODE *node)
{
    const NT_TYPE *type = NULL;
    if (node->left != NULL)
    {
        type = findType(modgen, node->left);
        if (node->right)
        {
            const NT_TYPE *initType = ntEvalExprType(&modgen->report, modgen->scope, node->right);
            if (type != initType)
            {
                ntErrorAtNode(&modgen->report, node, "Invalid initalizer type. Incompatible type!");
                return;
            }
        }
    }
    else
    {
        if (node->right == NULL)
        {
            ntErrorAtNode(&modgen->report, node,
                          "Variable declarations must has a type or initializer.");
            return;
        }
        type = ntEvalExprType(&modgen->report, modgen->scope, node->right);
    }

    if (node->right)
    {
        expression(modgen, node->right, true);
    }
    else
    {
        switch (type->stackSize)
        {
        case sizeof(uint32_t):
            emit(modgen, node, BC_ZERO_32);
            break;
        case sizeof(uint64_t):
            emit(modgen, node, BC_ZERO_64);
            break;
        default:
            ntErrorAtNode(&modgen->report, node, "CODEGEN invalid stackSize!");
            return;
        }
    }

    const NT_STRING *varName = ntCopyString(node->token.lexeme, node->token.lexemeLength);
    addLocal(modgen, varName, type);
}

static void varStatement(NT_MODGEN *modgen, const NT_NODE *node)
{
    ensureStmt(node, NK_VAR);
    declareVariable(modgen, node);
}

const char_t *const returnVariable = U"@return";

static void endFunctionScope(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE **returnType,
                             bool isEndScope)
{
    const NT_SYMBOL_TABLE *functionScope = modgen->scope;
    while (!(functionScope->type & (STT_FUNCTION | STT_METHOD)))
    {
        functionScope = functionScope->parent;
    }

    if (functionScope->type & STT_FUNCTION)
    {
        if (node->left == NULL)
        {
            ntErrorAtNode(&modgen->report, node,
                          "Critical modgen error! The scope need a return value.");
        }
        assert(node->left);

        const NT_TYPE *type = ntEvalExprType(&modgen->report, modgen->scope, node->left);
        if (modgen->scope->scopeReturnType == NULL ||
            modgen->scope->scopeReturnType == ntUndefinedType())
            modgen->scope->scopeReturnType = type;

        assert(modgen->scope->scopeReturnType == type);

        expression(modgen, node->left, true);
        emitAssign(modgen, node, type, returnVariable, ntStrLen(returnVariable),
                   "Critical modgen error! The variable for return must be declared.");

        const size_t scopeSize = modgen->stack->sp - (size_t)functionScope->data;
        assert(scopeSize >= type->stackSize);

        const size_t delta = scopeSize - type->stackSize;
        emitPartialFixedPop(modgen, node, delta);

        if (returnType && *returnType == NULL)
            *returnType = type;
    }
    else
    {
        const size_t scopeSize = modgen->stack->sp - (size_t)functionScope->data;
        emitPartialFixedPop(modgen, node, scopeSize);
    }

    if (isEndScope)
    {
        while (!(modgen->scope->type & (STT_FUNCTION | STT_METHOD)))
        {
            NT_SYMBOL_TABLE *oldScope = modgen->scope;
            modgen->scope = oldScope->parent;
            ntFreeSymbolTable(oldScope);
        }
        modgen->scope = functionScope->parent;
    }
}

static void declareFunction(NT_MODGEN *modgen, const NT_NODE *node, const bool returnValue)
{
    const size_t startPc = modgen->module->code.count;
    const char_t *name = node->token.lexeme;
    const size_t nameLen = node->token.lexemeLength;

    const NT_SYMBOL_TYPE symbolType = returnValue ? SYMBOL_TYPE_FUNCTION : SYMBOL_TYPE_SUBROUTINE;
    bool hasReturn = false;

    beginScope(modgen, returnValue ? STT_FUNCTION : STT_METHOD);

    const NT_LIST params = node->data;
    const size_t paramCount = ntListLen(node->data);

    NT_ARRAY paramsArray;
    ntInitArray(&paramsArray);
    const NT_TYPE *returnType = NULL;

    if (returnValue)
    {
        if (node->left)
            returnType = findType(modgen, node->left);
        else
            returnType = ntEvalBlockReturnType(&modgen->report, modgen->scope, node->right);

        push(modgen, node, returnType);
        addParam(modgen, ntCopyString(returnVariable, ntStrLen(returnVariable)), returnType);

        if (paramCount >= 1)
            pop(modgen, node, returnType);
        else
        {
            switch (returnType->stackSize)
            {
            case sizeof(uint32_t):
                emit(modgen, node, BC_ZERO_32);
                break;
            case sizeof(uint64_t):
                emit(modgen, node, BC_ZERO_64);
                break;
            default:
                ntErrorAtNode(&modgen->report, node, "CODEGEN invalid stackSize for return type!");
                return;
            }
        }
    }
    else
        returnType = ntVoidType();

    for (size_t i = 0; i < paramCount; ++i)
    {
        const NT_NODE *paramNode = ntListGet(params, i);
        const NT_NODE *typeNode = paramNode->left;
        const NT_TYPE *type = findType(modgen, typeNode);
        const NT_STRING *paramName =
            ntCopyString(paramNode->token.lexeme, paramNode->token.lexemeLength);

        push(modgen, paramNode, type);
        addParam(modgen, paramName, type);

        const NT_PARAM param = {
            .name = paramName,
            .type = type,
        };
        ntArrayAdd(&paramsArray, &param, sizeof(NT_PARAM));
    }

    const NT_DELEGATE_TYPE *delegateType = ntTakeDelegateType(
        modgen->codegen->assembly, returnType, paramCount, (NT_PARAM *)paramsArray.data);
    const NT_STRING *funcName = ntCopyString(name, nameLen);

    addFunction(modgen, funcName, symbolType, delegateType, startPc, modgen->public);

    for (size_t i = 0; i < ntListLen(node->right->data) && !hasReturn; ++i)
    {
        const NT_NODE *stmt = (NT_NODE *)ntListGet(node->right->data, i);
        const NT_TYPE *statmentReturn = NULL;
        statement(modgen, stmt, &statmentReturn);
        if (statmentReturn != NULL)
            hasReturn |= true;
    }

    ntDeinitArray(&paramsArray);

    // resolve branchs and labels
    resolveLabelAndBranchSymbols(modgen, node);

    if (returnValue)
    {
        if (!hasReturn)
        {
            char *lname = ntToCharFixed(name, nameLen);
            ntErrorAtNode(&modgen->report, node,
                          "Function '%s' doesn't  return a value on all code paths.", lname);
            ntFree(lname);

            assert(modgen->scope->type != STT_METHOD);
            endScope(modgen, node, true);
            emit(modgen, node, BC_RETURN);
        }
        else
        {
            endScope(modgen, node, false);
        }
    }
    else
    {
        endFunctionScope(modgen, node, NULL, true);
        emit(modgen, node, BC_RETURN);
    }
}

static void defStatement(NT_MODGEN *modgen, const NT_NODE *node)
{
    ensureStmt(node, NK_DEF);
    declareFunction(modgen, node, true);
}

static void subStatement(NT_MODGEN *modgen, const NT_NODE *node)
{
    ensureStmt(node, NK_SUB);
    declareFunction(modgen, node, false);
}

static void declaration(NT_MODGEN *modgen, const NT_NODE *node)
{
    switch (node->type.kind)
    {
    case NK_DEF:
        defStatement(modgen, node);
        break;
    case NK_SUB:
        subStatement(modgen, node);
        break;
    case NK_VAR:
        varStatement(modgen, node);
        break;
    case NK_IMPORT:
        break;
    default:
        ntErrorAtNode(&modgen->report, node, "Expect a declaration");
        break;
    }
}

static void returnStatement(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE **returnType)
{
    ensureStmt(node, NK_RETURN);
    endFunctionScope(modgen, node, returnType, false);
    emit(modgen, node, BC_RETURN);
}

static void breakStatement(NT_MODGEN *modgen, const NT_NODE *node)
{
    ensureStmt(node, NK_BREAK);

    const NT_SYMBOL_TABLE *breakScope = modgen->scope;
    while (breakScope && !(breakScope->type & STT_BREAKABLE))
    {
        breakScope = breakScope->parent;
    }

    if (!breakScope)
    {
        ntErrorAtNode(&modgen->report, node,
                      "Invalid break statement, break is not in a breakable scope!");
        return;
    }

    assert(breakScope->breakLabel);

    modgen->scope->breaked = true;
    const size_t scopeSize = modgen->stack->sp - (size_t)breakScope->data;
    emitPartialFixedPop(modgen, node, scopeSize);

    emitBranchLabel(modgen, node, BC_BRANCH, breakScope->breakLabel);
}

static void continueStatement(NT_MODGEN *modgen, const NT_NODE *node)
{
    ensureStmt(node, NK_CONTINUE);

    const NT_SYMBOL_TABLE *continueScope = modgen->scope;
    while (continueScope && !(continueScope->type & STT_BREAKABLE))
    {
        continueScope = continueScope->parent;
    }

    if (!continueScope)
    {
        ntErrorAtNode(&modgen->report, node,
                      "Invalid break statement, break is not in a breakable scope!");
        return;
    }

    assert(continueScope->loopLabel);

    modgen->scope->continued = true;
    const size_t scopeSize = modgen->stack->sp - (size_t)continueScope->data;
    emitPartialFixedPop(modgen, node, scopeSize);

    emitBranchLabel(modgen, node, BC_BRANCH, continueScope->loopLabel);
}

static void statement(NT_MODGEN *modgen, const NT_NODE *node, const NT_TYPE **returnType)
{
    *returnType = NULL;
    if (node->type.class != NC_STMT)
    {
        ntErrorAtNode(&modgen->report, node, "Invalid node, the node must be a statment!");
        return;
    }

    switch (node->type.kind)
    {
    case NK_EXPR:
        expressionStatement(modgen, node);
        break;
    case NK_IF:
        ifStatement(modgen, node, returnType);
        break;
    case NK_BLOCK:
        blockStatment(modgen, node, returnType);
        break;
    case NK_UNTIL:
        untilStatment(modgen, node);
        break;
    case NK_WHILE:
        whileStatment(modgen, node);
        break;
    case NK_VAR:
        varStatement(modgen, node);
        break;
    case NK_RETURN:
        returnStatement(modgen, node, returnType);
        break;
    case NK_BREAK:
        breakStatement(modgen, node);
        break;
    case NK_CONTINUE:
        continueStatement(modgen, node);
        break;
    default: {
        const char *const label = ntGetKindLabel(node->type.kind);
        ntErrorAtNode(&modgen->report, node,
                      "Invalid statment. The statement with kind '%s' is invalid.", label);
        break;
    }
    }
}

static void module(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(codegen);
    assert(node);
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_MODULE);

    NT_MODULE *module;
    assert(node->userdata != NULL);
    if (node->userdata == NULL)
    {
        printf("Something are wrong, field userdata of module node AST must has a module "
               "instance.\n");
        return;
    }

    module = (NT_MODULE *)node->userdata;
    assert(IS_VALID_OBJECT(module));
    assert(module->type.objectType == NT_OBJECT_MODULE);

    NT_MODGEN *modgen = ntCreateModgen(codegen, module);

    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        const NT_NODE *stmt = ntListGet(node->data, i);
        if (stmt->type.class == NC_STMT && stmt->type.kind == NK_PUBLIC)
            modgen->public = true;
        else if (stmt->type.class == NC_STMT && stmt->type.kind == NK_PRIVATE)
            modgen->public = false;
        declaration(modgen, stmt);
    }

    codegen->had_error |= modgen->report.had_error;
    ntFreeModgen(modgen);
    ntAddConstantObject(codegen->assembly, (NT_OBJECT *)module);
}

bool ntGen(NT_CODEGEN *codegen, size_t count, const NT_NODE **moduleNodes)
{
    // for (size_t i = 0; i < ntListLen(block->data); ++i)
    // {
    //     const NT_NODE *stmt = (NT_NODE *)ntListGet(block->data, i);
    //     if (stmt->type.)
    // }

    for (size_t i = 0; i < count; ++i)
    {
        const NT_NODE *stmt = moduleNodes[i];
        module(codegen, stmt);
    }

    // if (entryPointName)
    // {
    //     NT_SYMBOL_ENTRY symbolEntry;
    //     const size_t entryPointLength = ntStrLen(entryPointName);
    //     if (ntLookupSymbol(codegen->scope, entryPointName, entryPointLength, &symbolEntry) &&
    //         entryPoint)
    //     {
    //         const NT_DELEGATE *delegate = ntGetConstantDelegate(codegen->module,
    //         symbolEntry.data); assert(delegate); assert(delegate->object.type->objectType ==
    //         NT_OBJECT_DELEGATE); *entryPoint = delegate;
    //     }
    // }

    return !codegen->had_error;
}
