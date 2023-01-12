#include "codegen.h"
#include "parser.h"
#include "scanner.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/nto.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/symbol.h>
#include <netuno/varint.h>
#include <stdarg.h>
#include <stdio.h>

static void addType(NT_CODEGEN *codegen, const NT_TYPE *type)
{
    NT_SYMBOL_ENTRY entry = {
        .symbol_name = type->typeName,
        .type = SYMBOL_TYPE_TYPE,
        .data = 0,
        .exprType = type,
    };
    ntInsertSymbol(codegen->scope, &entry);
}

NT_CODEGEN *ntCreateCodegen(NT_ASSEMBLY *assembly, NT_CHUNK *chunk)
{
    NT_CODEGEN *codegen = (NT_CODEGEN *)ntMalloc(sizeof(NT_CODEGEN));

    codegen->chunk = chunk;
    codegen->scope = ntCreateSymbolTable(NULL, STT_NONE, 0);
    codegen->stack = ntCreateVStack();
    codegen->had_error = false;
    codegen->assembly = assembly;

    addType(codegen, ntI32Type());
    addType(codegen, ntI64Type());
    addType(codegen, ntU32Type());
    addType(codegen, ntU64Type());
    addType(codegen, ntF32Type());
    addType(codegen, ntF64Type());
    addType(codegen, ntStringType());

    return codegen;
}

void ntFreeCodegen(NT_CODEGEN *codegen)
{
    if (codegen)
    {
        ntFreeSymbolTable(codegen->scope);
        ntFreeVStack(codegen->stack);
        ntFree(codegen);
    }
}

static size_t emit(NT_CODEGEN *codegen, const NT_NODE *node, uint8_t value)
{
    return ntWriteChunk(codegen->chunk, value, node->token.line);
}

static void vErrorAt(NT_CODEGEN *codegen, const NT_NODE *node, const char *message, va_list args)
{
    const NT_TOKEN token = node->token;

    printf("[line %d] Error", token.line);

    if (token.type == TK_EOF)
        printf(" at end");
    else if (token.type == TK_ERROR)
    {
    }
    else
    {
        char *str = ntToCharFixed(token.lexeme, token.lexemeLength);
        printf(" at '%s'", str);
        ntFree(str);
    }

    printf(": ");
    vprintf(message, args);
    printf("\n");
    codegen->had_error = true;
}

static void vWarningAt(NT_CODEGEN *codegen, const NT_NODE *node, const char *message, va_list args)
{
    const NT_TOKEN token = node->token;

    printf("[line %d] Warming", token.line);

    if (token.type == TK_EOF)
        printf(" at end");
    else if (token.type == TK_ERROR)
    {
    }
    else
    {
        char *str = ntToCharFixed(token.lexeme, token.lexemeLength);
        printf(" at '%s'", str);
        ntFree(str);
    }

    printf(": ");
    vprintf(message, args);
    printf("\n");
    codegen->had_error = true;
}

static void errorAt(NT_CODEGEN *codegen, const NT_NODE *node, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    vErrorAt(codegen, node, message, vl);
    va_end(vl);
}

static void warningAt(NT_CODEGEN *codegen, const NT_NODE *node, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    vWarningAt(codegen, node, message, vl);
    va_end(vl);
}

static size_t push(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE *type)
{
    assert(type);
    assert(node);
    return ntVPush(codegen->stack, type);
}

static size_t pop(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE *type)
{
    const NT_TYPE *popType;
    const size_t result = ntVPop(codegen->stack, &popType);

    if (popType != type)
    {
        errorAt(
            codegen, node,
            "Sometime are wrong in codegen, the popped type dont match with pop type are wrong.");
    }

    return result;
}

static void emitConstantI32(NT_CODEGEN *codegen, const NT_NODE *node, uint32_t value)
{
    emit(codegen, node, BC_CONST_32);
    const uint64_t index = ntAddConstant32(codegen->chunk, value);
    ntWriteChunkVarint(codegen->chunk, index, node->token.line);
    push(codegen, node, ntI32Type());
}

static void emitConstantI64(NT_CODEGEN *codegen, const NT_NODE *node, uint64_t value)
{
    emit(codegen, node, BC_CONST_64);
    const uint64_t index = ntAddConstant64(codegen->chunk, value);
    ntWriteChunkVarint(codegen->chunk, index, node->token.line);
    push(codegen, node, ntI64Type());
}

static void emitConstantU32(NT_CODEGEN *codegen, const NT_NODE *node, uint32_t value)
{
    emit(codegen, node, BC_CONST_32);
    const uint64_t index = ntAddConstant32(codegen->chunk, value);
    ntWriteChunkVarint(codegen->chunk, index, node->token.line);
    push(codegen, node, ntU32Type());
}

static void emitConstantU64(NT_CODEGEN *codegen, const NT_NODE *node, uint64_t value)
{
    emit(codegen, node, BC_CONST_64);
    const uint64_t index = ntAddConstant64(codegen->chunk, value);
    ntWriteChunkVarint(codegen->chunk, index, node->token.line);
    push(codegen, node, ntU64Type());
}

static void emitConstantF32(NT_CODEGEN *codegen, const NT_NODE *node, uint32_t value)
{
    emit(codegen, node, BC_CONST_32);
    const uint64_t index = ntAddConstant32(codegen->chunk, value);
    ntWriteChunkVarint(codegen->chunk, index, node->token.line);
    push(codegen, node, ntF32Type());
}

static void emitConstantF64(NT_CODEGEN *codegen, const NT_NODE *node, uint64_t value)
{
    emit(codegen, node, BC_CONST_64);
    const uint64_t index = ntAddConstant64(codegen->chunk, value);
    ntWriteChunkVarint(codegen->chunk, index, node->token.line);
    push(codegen, node, ntF64Type());
}

static void emitConstantString(NT_CODEGEN *codegen, const NT_NODE *node, const char_t *chars,
                               const size_t length)
{
    emit(codegen, node, BC_CONST_STRING);
    const uint64_t index = ntAddConstantString(codegen->chunk, chars, length);
    ntWriteChunkVarint(codegen->chunk, index, node->token.line);
    push(codegen, node, ntStringType());
}

static size_t emitPop(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE *type)
{
    switch (type->objectType)
    {
    case NT_OBJECT_STRING:
    case NT_OBJECT_CUSTOM:
    case NT_OBJECT_I32:
    case NT_OBJECT_U32:
    case NT_OBJECT_F32:
        emit(codegen, node, BC_POP_32);
        break;
    case NT_OBJECT_F64:
    case NT_OBJECT_U64:
    case NT_OBJECT_I64:
        emit(codegen, node, BC_POP_64);
        break;
    default:
        warningAt(codegen, node, "Invalid objectType pop.");
        return 0;
    }
    return pop(codegen, node, type);
}

static size_t emitFixedPop(NT_CODEGEN *codegen, const NT_NODE *node, const size_t popSize)
{
    if (popSize == 0)
        return 0;

    assert(popSize % sizeof(uint32_t) == 0);

    int64_t rem = (int64_t)popSize;
    size_t sp = 0;
    while (rem > 0)
    {
        const NT_TYPE *poppedType = NULL;
        sp = ntVPop(codegen->stack, &poppedType);
        rem -= poppedType->stackSize;
    }

    // value partial popped if false
    assert(rem == 0);

    emit(codegen, node, BC_POP);
    ntWriteChunkVarint(codegen->chunk, popSize / sizeof(uint32_t), node->token.line);
    return sp;
}

static void ensureStmt(const NT_NODE *node, NT_NODE_KIND kind)
{
    assert(node->type.class == NC_STMT && node->type.kind == kind &&
           node->type.literalType == LT_NONE);
}

static void beginScope(NT_CODEGEN *codegen, NT_SYMBOL_TABLE_TYPE type)
{
    codegen->scope = ntCreateSymbolTable(codegen->scope, type, codegen->stack->sp);
}

static void endScopeRemaining(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE *type)
{
    const size_t delta = codegen->stack->sp - (codegen->scope->data + type->stackSize);
    emitFixedPop(codegen, node, delta);
    codegen->scope = codegen->scope->parent;
}

static void endScope(NT_CODEGEN *codegen, const NT_NODE *node)
{
    const size_t delta = codegen->stack->sp - codegen->scope->data;
    emitFixedPop(codegen, node, delta);

    NT_SYMBOL_TABLE *oldScope = codegen->scope;
    codegen->scope = oldScope->parent;
    ntFreeSymbolTable(oldScope);
}

static void addLocal(NT_CODEGEN *codegen, const NT_STRING *name, const NT_TYPE *type)
{
    // TODO: check type in vstack
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_VARIABLE,
        .data = codegen->stack->sp,
        .exprType = type,
    };
    const bool result = ntInsertSymbol(codegen->scope, &entry);
    assert(result);
}

static void addParam(NT_CODEGEN *codegen, const NT_STRING *name, const NT_TYPE *type)
{
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_PARAM,
        .data = codegen->stack->sp,
        .exprType = type,
    };
    const bool result = ntInsertSymbol(codegen->scope, &entry);
    assert(result);
}

static void addSymbol(NT_CODEGEN *codegen, const NT_STRING *name, NT_SYMBOL_TYPE symbolType,
                      const NT_TYPE *type, size_t data)
{
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = symbolType,
        .data = data,
        .exprType = type,
    };
    const bool result = ntInsertSymbol(codegen->scope, &entry);
    assert(result);
}

static const NT_TYPE *findType(NT_CODEGEN *codegen, const NT_NODE *typeNode)
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
        default: {
            char *typeLex = ntToChar(ntGetKeywordLexeme(typeNode->token.id));
            errorAt(codegen, typeNode, "The keyword '%s' is not a type.", typeLex);
            ntFree(typeLex);
            return ntErrorType();
        }
        }
    }
    else
    {
        // object
        NT_SYMBOL_ENTRY entry;
        if (!ntLookupSymbol(codegen->scope, name->lexeme, name->lexemeLength, &entry))
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            errorAt(codegen, typeNode, "The type '%s' don't exist.", lexeme);
            ntFree(lexeme);
            return NULL;
        }

        if (entry.type != SYMBOL_TYPE_TYPE)
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            errorAt(codegen, typeNode, "The identifier '%s' is not a type.", lexeme);
            ntFree(lexeme);
            return NULL;
        }
        return entry.exprType;
    }
}

static bool findSymbol(NT_CODEGEN *codegen, const char_t *name, const size_t length,
                       NT_SYMBOL_ENTRY *pEntry)
{
    return ntLookupSymbol(codegen->scope, name, length, pEntry);
}

static const NT_TYPE *evalExprType(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR);

    const NT_TYPE *left = NULL;
    const NT_TYPE *right = NULL;

    if (node->left != NULL)
    {
        left = evalExprType(codegen, node->left);
        assert(left);
    }

    if (node->right != NULL)
    {
        right = evalExprType(codegen, node->right);
        assert(right);
    }

    switch (node->type.kind)
    {
    case NK_LITERAL:
        switch (node->type.literalType)
        {
        case LT_BOOL:
            assert(node->token.type == TK_KEYWORD);
            return ntBoolType();
        case LT_NONE:
            return ntI32Type();
        case LT_STRING:
            return ntStringType();
        case LT_I32:
            return ntI32Type();
        case LT_I64:
            return ntI64Type();
        case LT_U32:
            return ntU32Type();
        case LT_U64:
            return ntU64Type();
        case LT_F32:
            return ntF32Type();
        case LT_F64:
            return ntF64Type();
        default:
            errorAt(codegen, node, "Invalid literal type! '%d'", node->type.literalType);
            break;
        }
        break;
    case NK_UNARY:
        switch (node->token.id)
        {
        case '-':
        case OP_DEC:
        case OP_INC:
            return left == NULL ? right : left;
        case '!':
            return ntBoolType();
        case '~':
            if (right->objectType == NT_OBJECT_I32 || right->objectType == NT_OBJECT_I64 ||
                right->objectType == NT_OBJECT_U32 || right->objectType == NT_OBJECT_U64)
                return right;
            else
            {
                errorAt(codegen, node,
                        "Invalid type for '~' operation! Must be a integer(i32, i64, u32 or u64).");
                return NULL;
            }
        default:
            errorAt(codegen, node, "Invalid unary operator!");
            return NULL;
        }
    case NK_BINARY:
        switch (node->token.id)
        {
        case OP_NE:
        case OP_EQ:
        case '>':
        case OP_GE:
        case '<':
        case OP_LE:
            return ntBoolType();
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '|':
        case '&':
        case '^':
            left = evalExprType(codegen, node->left);
            right = evalExprType(codegen, node->right);

            if (left->objectType == NT_OBJECT_CUSTOM || right->objectType == NT_OBJECT_CUSTOM)
            {
                // TODO: add operators support to objects.
                errorAt(codegen, node, "Invalid math operation with custom object.");
                return NULL;
            }

            if (left->objectType < right->objectType)
                return left;
            return right;
        default:
            errorAt(codegen, node, "Invalid binary operation. %d", node->token.id);
            return NULL;
        }
        break;
    case NK_LOGICAL:
        switch (node->token.id)
        {
        case OP_LOGOR:
        case OP_LOGAND:
            return ntBoolType();
        default:
            errorAt(codegen, node, "Invalid logical operation. %d", node->token.id);
            return NULL;
        }
    case NK_CALL: {
        switch (left->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
        case NT_OBJECT_F32:
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
        case NT_OBJECT_F64:
        case NT_OBJECT_FUNCTION:
            return left;
        default: {
            char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            errorAt(codegen, node, "The function or method '%s' must be declareed.", str);
            ntFree(str);
            return NULL;
        }
        }
        break;
    }
    case NK_VARIABLE: {
        NT_SYMBOL_ENTRY entry;
        if (!findSymbol(codegen, node->token.lexeme, node->token.lexemeLength, &entry))
        {
            errorAt(codegen, node, "The variable must be declared.");
            break;
        }
        if (entry.type != SYMBOL_TYPE_VARIABLE && entry.type != SYMBOL_TYPE_CONSTANT &&
            entry.type != SYMBOL_TYPE_PARAM && entry.type != SYMBOL_TYPE_TYPE)
        {
            char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            errorAt(codegen, node, "The symbol '%s' is not a constant, parameter or variable!",
                    str);
            ntFree(str);
            break;
        }
        return entry.exprType;
    }
    break;
    case NK_ASSIGN: {
        if (left != right)
        {
            char *leftName = ntToCharFixed(left->typeName->chars, left->typeName->length);
            char *rightName = ntToCharFixed(right->typeName->chars, right->typeName->length);
            errorAt(codegen, node,
                    "Invalid type, variable is of type %s, but the value expression to assign is "
                    "%s.",
                    leftName, rightName);
            ntFree(leftName);
            ntFree(rightName);
        }
        return left;
    }
    break;
    default:
        errorAt(codegen, node, "AST invalid format, node kind cannot be %s!",
                ntGetKindLabel(node->type.kind));
        break;
    }

    errorAt(codegen, node, "Unkown expr.");
    return NULL;
}

static void number(NT_CODEGEN *codegen, const NT_NODE *node)
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
        errorAt(codegen, node, "Invalid number token type! '%s'", node->token.type);
        break;
    }
    ntFree(str);

    switch (node->token.type)
    {
    case TK_I32:
        emitConstantI32(codegen, node, u32);
        break;
    case TK_U32:
        emitConstantU32(codegen, node, u32);
        break;
    case TK_F32:
        emitConstantF32(codegen, node, u32);
        break;
    case TK_I64:
        emitConstantI64(codegen, node, u64);
        break;
    case TK_U64:
        emitConstantU64(codegen, node, u64);
        break;
    case TK_F64:
        emitConstantF64(codegen, node, u64);
        break;
    default:
        errorAt(codegen, node, "Invalid number token type! '%s'", node->token.type);
        break;
    }
}

static void string(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_LITERAL &&
           node->type.literalType == LT_STRING);

    // remove quotes
    const char_t *start = node->token.lexeme + 1;
    const size_t length = node->token.lexemeLength - 2;

    emitConstantString(codegen, node, start, length);
}

static void literal(NT_CODEGEN *codegen, const NT_NODE *node)
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
            emit(codegen, node, BC_ZERO_32);
            push(codegen, node, ntBoolType());
            break;
        case KW_TRUE:
            emit(codegen, node, BC_ONE_32);
            push(codegen, node, ntBoolType());
            break;
        default: {
            char *lexeme = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            errorAt(codegen, node,
                    "AST invalid format, node id of a bool literal must be TK_TRUE or TK_FALSE "
                    "cannot be '%s'!",
                    lexeme);
            ntFree(lexeme);
        }
        break;
        }
        break;
    case LT_NONE:
        emit(codegen, node, BC_ZERO_32);
        push(codegen, node, ntU32Type());
        break;
    case LT_STRING:
        string(codegen, node);
        break;
    case LT_I32:
    case LT_I64:
    case LT_U32:
    case LT_U64:
    case LT_F32:
    case LT_F64:
        number(codegen, node);
        break;
    default:
        errorAt(codegen, node, "Invalid literal type! '%d'", node->type.literalType);
        break;
    }
}

static void expression(NT_CODEGEN *codegen, const NT_NODE *node, const bool needValue);

static void unary(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_UNARY);
    assert(node->token.type == TK_KEYWORD);

    const NT_TYPE *type = evalExprType(codegen, node->right);
    expression(codegen, node->right, true);
    pop(codegen, node, type);

    switch (node->token.id)
    {
    case '-':
        // negate
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(codegen, node, BC_NEG_I32);
            push(codegen, node, ntI32Type());
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_NEG_I32);
            push(codegen, node, ntU32Type());
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_NEG_I64);
            push(codegen, node, ntI64Type());
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_NEG_I64);
            push(codegen, node, ntU64Type());
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_NEG_F32);
            push(codegen, node, ntF32Type());
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_NEG_F64);
            push(codegen, node, ntF64Type());
            break;
        case NT_OBJECT_STRING:
        case NT_OBJECT_CUSTOM:
        // TODO: call operator.
        default: {
            char *str = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid negate('-') operation with type '%s'.", str);
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
            emit(codegen, node, BC_IS_ZERO_32);
            push(codegen, node, ntBoolType());
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_IS_ZERO_64);
            push(codegen, node, ntBoolType());
            break;
        default: {
            char *str = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid logical not('!') operation with type '%s'.", str);
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
            emit(codegen, node, BC_NOT_32);
            push(codegen, node, ntI32Type());
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_NOT_32);
            push(codegen, node, ntU32Type());
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_NOT_64);
            push(codegen, node, ntI64Type());
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_NOT_64);
            push(codegen, node, ntU64Type());
            break;
        default: {
            char *str = ntToCharFixed(type->typeName->chars, type->typeName->length);
            errorAt(codegen, node, "Invalid logical not('!') operation with type '%s'.", str);
            ntFree(str);
            break;
        }
        }
        break;
    default:
        errorAt(codegen, node, "Invalid unary operation. %d(%c)", node->token.id,
                (char)node->token.id);
        break;
    }
}

static void cast(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE *from, const NT_TYPE *to)
{
    pop(codegen, node, from);

    switch (from->objectType)
    {
    case NT_OBJECT_I32:
        switch (to->objectType)
        {
        case NT_OBJECT_I32:
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_EXTEND_I32);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_CONVERT_F32_I32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_CONVERT_F64_I32);
            break;
        case NT_OBJECT_STRING:
            emit(codegen, node, BC_CONVERT_STR_I32);
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
            emit(codegen, node, BC_EXTEND_U32);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_CONVERT_F32_U32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_CONVERT_F64_U32);
            break;
        case NT_OBJECT_STRING:
            emit(codegen, node, BC_CONVERT_STR_U32);
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
            emit(codegen, node, BC_WRAP_I64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_CONVERT_F32_I64);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_CONVERT_F64_I64);
            break;
        case NT_OBJECT_STRING:
            emit(codegen, node, BC_CONVERT_STR_I64);
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
            emit(codegen, node, BC_WRAP_I64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_CONVERT_F32_U64);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_CONVERT_F64_U64);
            break;
        case NT_OBJECT_STRING:
            emit(codegen, node, BC_CONVERT_STR_U64);
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
            emit(codegen, node, BC_TRUNCATE_I32_F32);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_TRUNCATE_U32_F32);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_TRUNCATE_I64_F32);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_TRUNCATE_U64_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_PROMOTE_F32);
            break;
        case NT_OBJECT_STRING:
            emit(codegen, node, BC_CONVERT_STR_F32);
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
            emit(codegen, node, BC_TRUNCATE_I32_F64);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_TRUNCATE_U32_F64);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_TRUNCATE_I64_F64);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_TRUNCATE_U64_F64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_DEMOTE_F64);
            break;
        case NT_OBJECT_STRING:
            emit(codegen, node, BC_CONVERT_STR_F64);
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
            emit(codegen, node, BC_CONVERT_I32_STR);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_CONVERT_U32_STR);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_CONVERT_I64_STR);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_CONVERT_U64_STR);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_CONVERT_F32_STR);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_CONVERT_F64_STR);
            break;
        default:
            goto error;
        }
        break;
    default:
        goto error;
    }

    push(codegen, node, to);
    return;
error : {

    // TODO: object cast operator?
    char *fromStr = ntToChar(from->typeName->chars);
    char *dstStr = ntToChar(to->typeName->chars);
    errorAt(codegen, node, "Invalid cast from '%s' to '%s'.", fromStr, dstStr);
    ntFree(fromStr);
    ntFree(dstStr);

    push(codegen, node, to);
}
}

static void binary(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_BINARY);
    assert(node->left != NULL && node->right != NULL);

    const NT_TYPE *leftType = evalExprType(codegen, node->left);
    const NT_TYPE *rightType = evalExprType(codegen, node->right);
    const NT_TYPE *type = leftType->objectType < rightType->objectType ? leftType : rightType;

    expression(codegen, node->left, true);
    cast(codegen, node->left, leftType, type);

    expression(codegen, node->right, true);
    cast(codegen, node->right, leftType, type);

    pop(codegen, node, type);
    pop(codegen, node, type);

    switch (node->token.id)
    {
    case OP_NE:
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(codegen, node, BC_NE_32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_NE_64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_NE_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_NE_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid != operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, ntBoolType());
        break;
    case OP_EQ:
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(codegen, node, BC_EQ_32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_EQ_64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_EQ_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_EQ_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid == operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, ntBoolType());
        break;
    case '>':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(codegen, node, BC_GT_I32);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_GT_U32);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_GT_I64);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_GT_U64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_GT_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_GT_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid > operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, ntBoolType());
        break;
    case OP_GE:
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(codegen, node, BC_GE_I32);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_GE_U32);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_GE_I64);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_GE_U64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_GE_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_GE_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid <= operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, ntBoolType());
        break;
    case '<':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(codegen, node, BC_LT_I32);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_LT_U32);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_LT_I64);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_LT_U64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_LT_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_LT_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid < operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, ntBoolType());
        break;
    case OP_LE:
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(codegen, node, BC_LE_I32);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_LE_U32);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_LE_I64);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_LE_U64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_LE_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_LE_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid < operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, ntBoolType());
        break;

    case '+':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(codegen, node, BC_ADD_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_ADD_I64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_ADD_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_ADD_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid + operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, type);
        break;
    case '-':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(codegen, node, BC_SUB_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_SUB_I64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_SUB_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_SUB_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid + operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, type);
        break;
    case '*':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(codegen, node, BC_MUL_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_MUL_I64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_MUL_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_MUL_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid * operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, type);
        break;
    case '/':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(codegen, node, BC_DIV_I32);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_DIV_U32);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_DIV_I64);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_DIV_U64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_DIV_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_DIV_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid / operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, type);
        break;
    case '%':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
            emit(codegen, node, BC_REM_I32);
            break;
        case NT_OBJECT_U32:
            emit(codegen, node, BC_REM_U32);
            break;
        case NT_OBJECT_I64:
            emit(codegen, node, BC_REM_I64);
            break;
        case NT_OBJECT_U64:
            emit(codegen, node, BC_REM_U64);
            break;
        case NT_OBJECT_F32:
            emit(codegen, node, BC_REM_F32);
            break;
        case NT_OBJECT_F64:
            emit(codegen, node, BC_REM_F64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid / operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, type);
        break;
    case '|':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(codegen, node, BC_OR_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_OR_I64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid | operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, type);
        break;
    case '&':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(codegen, node, BC_AND_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_AND_I64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid & operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, type);
        break;
    case '^':
        switch (type->objectType)
        {
        case NT_OBJECT_I32:
        case NT_OBJECT_U32:
            emit(codegen, node, BC_XOR_I32);
            break;
        case NT_OBJECT_I64:
        case NT_OBJECT_U64:
            emit(codegen, node, BC_XOR_I64);
            break;
        default: {
            char *typeStr = ntToChar(type->typeName->chars);
            errorAt(codegen, node, "Invalid ^ operation for type '%s'.", typeStr);
            ntFree(typeStr);
            break;
        }
        }
        push(codegen, node, type);
        break;
    default:
        errorAt(codegen, node, "Invalid binary operation. %d", node->token.id);
        break;
    }
}

static void variable(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_EXPR && node->type.kind == NK_VARIABLE);

    NT_SYMBOL_ENTRY entry;
    if (!findSymbol(codegen, node->token.lexeme, node->token.lexemeLength, &entry))
    {
        errorAt(codegen, node, "The variable must be declared.");
        return;
    }

    const NT_TYPE *type = entry.exprType;
    const uint32_t delta = codegen->stack->sp - entry.data - type->stackSize;

    switch (type->stackSize)
    {
    case sizeof(uint32_t):
        emit(codegen, node, BC_LOAD_SP_32);
        break;
    case sizeof(uint64_t):
        emit(codegen, node, BC_LOAD_SP_64);
        break;
    default:
        errorAt(codegen, node,
                "The variable has a stack size incompatible with BC_LOAD_SP operation.");
        return;
    }
    ntWriteChunkVarint(codegen->chunk, delta, node->token.line);
    push(codegen, node, entry.exprType);
}

static void emitAssign32(NT_CODEGEN *codegen, const NT_NODE *node, const char_t *variableName,
                         size_t variableNameLen, const char *message, ...)
{
    NT_SYMBOL_ENTRY entry;
    if (!ntLookupSymbol(codegen->scope, variableName, variableNameLen, &entry))
    {
        va_list vl;
        va_start(vl, message);
        vErrorAt(codegen, node, message, vl);
        va_end(vl);
    }

    const size_t delta = codegen->stack->sp - entry.data;
    emit(codegen, node, BC_STORE_SP_32);
    const size_t offset = ntWriteChunkVarint(codegen->chunk, delta, node->token.line);
    assert(offset);
}

static void emitAssign64(NT_CODEGEN *codegen, const NT_NODE *node, const char_t *variableName,
                         size_t variableNameLen, const char *message, ...)
{
    NT_SYMBOL_ENTRY entry;
    if (!ntLookupSymbol(codegen->scope, variableName, variableNameLen, &entry))
    {
        va_list vl;
        va_start(vl, message);
        vErrorAt(codegen, node, message, vl);
        va_end(vl);
    }

    const size_t delta = codegen->stack->sp - entry.data;
    emit(codegen, node, BC_STORE_SP_64);
    const size_t offset = ntWriteChunkVarint(codegen->chunk, delta, node->token.line);
    assert(offset);
}

static void emitAssign(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE *type,
                       const char_t *variableName, size_t variableNameLen, const char *message)
{
    switch (type->stackSize)
    {
    case sizeof(uint32_t):
        emitAssign32(codegen, node, variableName, variableNameLen, message);
        break;
    case sizeof(uint64_t):
        emitAssign64(codegen, node, variableName, variableNameLen, message);
        break;
    default:
        errorAt(codegen, node, "INTERNAL ERROR: Invalid objectType field! %d", type->objectType);
        break;
    }
}

static void assign(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node != NULL);
    assert(codegen != NULL);
    assert(node->type.class == NC_EXPR && node->type.kind == NK_ASSIGN);
    assert(node->left->type.class == NC_EXPR && node->left->type.kind == NK_VARIABLE);

    const NT_TYPE *rightType = evalExprType(codegen, node->right);
    expression(codegen, node->right, true);

    const NT_NODE *identifier = node->left;
    assert(identifier);

    NT_SYMBOL_ENTRY entry;
    if (!findSymbol(codegen, identifier->token.lexeme, identifier->token.lexemeLength, &entry))
    {
        errorAt(codegen, node, "The variable must be declared.");
        return;
    }

    if (entry.exprType != rightType)
    {
        char *rightTypeName = ntToChar(rightType->typeName->chars);
        char *variableTypeName = ntToChar(entry.exprType->typeName->chars);
        errorAt(codegen, node, "The variable type '%s' is incompatible with expression type '%s'.",
                variableTypeName, rightTypeName);
        ntFree(rightTypeName);
        ntFree(variableTypeName);
        return;
    }

    emitAssign(codegen, node, rightType, node->left->token.lexeme, node->left->token.lexemeLength,
               "The variable must be declared.");
}

static void typeToBool(NT_CODEGEN *codegen, const NT_NODE *node, const NT_TYPE *type)
{
    switch (type->objectType)
    {
    case NT_OBJECT_F64:
        emit(codegen, node, BC_IS_NOT_ZERO_F64);
        break;
    case NT_OBJECT_F32:
        emit(codegen, node, BC_IS_NOT_ZERO_F32);
        break;
    case NT_OBJECT_I64:
    case NT_OBJECT_U64:
        emit(codegen, node, BC_IS_NOT_ZERO_64);
        break;
    default: {
        char *typeStr = ntToChar(type->typeName->chars);
        errorAt(codegen, node, "Invalid implicit cast from type '%s' to 'bool'.", typeStr);
        ntFree(typeStr);
        break;
    }
    }
    pop(codegen, node, type);
    push(codegen, node, ntBoolType());
}

static void logicalAnd(NT_CODEGEN *codegen, const NT_NODE *node)
{
    // branch when left value is false
    const size_t falseBranch = emit(codegen, node, BC_BRANCH_Z_32);

    // consume left 'true' value and check right value
    emitPop(codegen, node, ntBoolType());

    const NT_TYPE *right = evalExprType(codegen, node->right);
    expression(codegen, node->right, true);
    typeToBool(codegen, node, right);

    ntInsertChunkVarint(codegen->chunk, falseBranch + 1, codegen->chunk->code.count - falseBranch);
}

static void logicalOr(NT_CODEGEN *codegen, const NT_NODE *node)
{
    // branch to right value when left value is false
    const size_t elseBranch = emit(codegen, node, BC_BRANCH_Z_32);

    // branch to end, because left value is true
    const size_t trueBranch = emit(codegen, node, BC_BRANCH);

    // consume left 'false' value
    const size_t elseLocation = emitPop(codegen, node, ntBoolType());

    // eval right
    const NT_TYPE *right = evalExprType(codegen, node->right);
    expression(codegen, node->right, true);

    // logical not
    switch (right->objectType)
    {
    case NT_OBJECT_I32:
    case NT_OBJECT_U32:
        break;
    case NT_OBJECT_I64:
    case NT_OBJECT_U64:
        emit(codegen, node, BC_IS_NOT_ZERO_64);
        push(codegen, node, ntBoolType());
        break;
    default:
        errorAt(codegen, node, "Invalid argument cast to bool.");
        break;
    }
    ntInsertChunkVarint(codegen->chunk, trueBranch + 1, codegen->chunk->code.count - trueBranch);
    ntInsertChunkVarint(codegen->chunk, elseBranch + 1, elseLocation - elseBranch);
}

static void logical(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(codegen);
    assert(node);
    assert(node->type.class == NC_EXPR && node->type.kind == NK_LOGICAL);

    const NT_TYPE *type = evalExprType(codegen, node->left);
    expression(codegen, node->left, true);
    typeToBool(codegen, node, type);

    switch (node->token.id)
    {
    case OP_LOGAND:
        logicalAnd(codegen, node);
        break;
    case OP_LOGOR:
        logicalOr(codegen, node);
        break;
    default: {
        char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        errorAt(codegen, node, "Invalid logical operation with ID '%d'('%s').", node->token.id,
                str);
        ntFree(str);
        break;
    }
    }
}

static void call(NT_CODEGEN *codegen, const NT_NODE *node, const bool needValue)
{
    assert(codegen);
    assert(node);
    assert(node->type.class == NC_EXPR && node->type.kind == NK_CALL);

    const NT_TOKEN *callName = &node->left->token;
    NT_SYMBOL_ENTRY entry;
    if (!ntLookupSymbol(codegen->scope, callName->lexeme, callName->lexemeLength, &entry))
    {
        char *name = ntToCharFixed(callName->lexeme, callName->lexemeLength);
        errorAt(codegen, node->left, "The function or subroutine '%s' must be declared.", name);
        ntFree(name);
        return;
    }

    if (callName->type == TK_KEYWORD && ntListLen(node->data) == 1)
    {
        // cast operator
        const NT_NODE *expr = ntListGet(node->data, 0);
        const NT_TYPE *exprType = evalExprType(codegen, expr);
        const NT_TYPE *dstType;

        expression(codegen, expr, true);

        switch (callName->id)
        {
        case KW_I32:
            dstType = ntI32Type();
            break;
        case KW_U32:
            dstType = ntU32Type();
            break;
        case KW_F32:
            dstType = ntI32Type();
            break;
        case KW_I64:
            dstType = ntI32Type();
            break;
        case KW_U64:
            dstType = ntU32Type();
            break;
        case KW_F64:
            dstType = ntI32Type();
            break;
        case KW_STRING:
            dstType = ntStringType();
            break;
        default: {
            char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            errorAt(codegen, node, "The keyword '%s' cannot be used to cast to.", str);
            ntFree(str);
            return;
        }
        }

        cast(codegen, node, exprType, dstType);
        return;
    }
    else if (entry.type != SYMBOL_TYPE_FUNCTION && entry.type != SYMBOL_TYPE_SUBROUTINE)
    {
        char *name = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        errorAt(codegen, node, "A symbol '%s' is not a function or subroutine.", name);
        ntFree(name);
        return;
    }

    if (needValue && entry.type == SYMBOL_TYPE_SUBROUTINE)
    {
        char *name = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        errorAt(codegen, node, "A subroutine('%s') cannot return a value.", name);
        ntFree(name);
        return;
    }

    const uint32_t sp = codegen->stack->sp;

    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        const NT_NODE *arg = (const NT_NODE *)ntListGet(node->data, i);
        expression(codegen, arg, true);
    }

    emitConstantString(codegen, node, node->token.lexeme, node->token.lexemeLength);
    emit(codegen, node, BC_CALL);

    size_t delta = codegen->stack->sp - sp;

    if (delta > 0 && entry.type == SYMBOL_TYPE_FUNCTION)
    {
        // ensure that dont pops the arg0, because is the return value.
        switch (entry.exprType->objectType)
        {
        case NT_OBJECT_STRING:
        case NT_OBJECT_CUSTOM:
        case NT_OBJECT_F32:
        case NT_OBJECT_U32:
        case NT_OBJECT_I32:
            delta -= sizeof(uint32_t);
            break;
        case NT_OBJECT_F64:
        case NT_OBJECT_U64:
        case NT_OBJECT_I64:
            delta -= sizeof(uint64_t);
            break;
        default:
            break;
        }
    }

    emitFixedPop(codegen, node, delta);
}

static void expression(NT_CODEGEN *codegen, const NT_NODE *node, const bool needValue)
{
    assert(node->type.class == NC_EXPR);

    switch (node->type.kind)
    {
    case NK_LITERAL:
        literal(codegen, node);
        break;
    case NK_UNARY:
        unary(codegen, node);
        break;
    case NK_BINARY:
        binary(codegen, node);
        break;
    case NK_VARIABLE:
        variable(codegen, node);
        break;
    case NK_ASSIGN:
        assign(codegen, node);
        break;
    case NK_LOGICAL:
        logical(codegen, node);
        break;
    case NK_CALL:
        call(codegen, node, needValue);
        break;
    default: {
        char *lexeme = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
        errorAt(codegen, node, "CODEGEN unrecognized expression. (Lexeme: %s)", lexeme);
        ntFree(lexeme);
        break;
    }
    }
}

static void printStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    const NT_TYPE *leftType = evalExprType(codegen, node->left);
    expression(codegen, node->left, true);

    if (leftType->objectType != NT_OBJECT_STRING)
        cast(codegen, node, leftType, ntStringType());

    pop(codegen, node, ntStringType());
    emit(codegen, node, BC_PRINT);
}

static void expressionStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_EXPR);

    const NT_TYPE *leftType = evalExprType(codegen, node->left);
    expression(codegen, node->left, false);
    emitPop(codegen, node, leftType);
}

static void statement(NT_CODEGEN *codegen, const NT_NODE *node, bool *hasReturn);

static size_t emitCondition(NT_CODEGEN *codegen, const NT_NODE *node, bool isZero,
                            const NT_TYPE **pConditionType)
{
    assert(codegen);
    assert(node);
    assert(pConditionType);

    // emit condition and branch
    *pConditionType = evalExprType(codegen, node->condition);
    expression(codegen, node->condition, true);

    switch ((*pConditionType)->objectType)
    {
    case NT_OBJECT_I32:
    case NT_OBJECT_U32:
        return emit(codegen, node, isZero ? BC_BRANCH_Z_32 : BC_BRANCH_NZ_32);
    case NT_OBJECT_I64:
    case NT_OBJECT_U64:
        return emit(codegen, node, isZero ? BC_BRANCH_Z_64 : BC_BRANCH_NZ_64);
    default:
        errorAt(codegen, node,
                "Invalid expression, must evaluate to basic types like int, long, etc.");
        return 0;
    }
}

static void ifStatement(NT_CODEGEN *codegen, const NT_NODE *node, bool *hasReturn)
{
    const bool hasElse = node->right != NULL;

    // emit condition and branch
    const NT_TYPE *conditionType;
    const size_t elseBranch = emitCondition(codegen, node, true, &conditionType);
    assert(conditionType);

    // if has else body, each body pops the condition from stack, otherwise, pop in end
    if (hasElse)
        emitPop(codegen, node, conditionType);

    // emit then body
    statement(codegen, node->left, hasReturn);

    // start of else branch
    size_t startElse = codegen->chunk->code.count;

    // emit else body if exist
    if (hasElse)
    {
        // push in vstack for else branch
        push(codegen, node, conditionType);

        // skip else body, when then body has taken
        const size_t skipElse = emit(codegen, node, BC_BRANCH);
        emitPop(codegen, node, conditionType);

        // emit else body
        statement(codegen, node->right, hasReturn);

        // add offset as skipElse param
        ntInsertChunkVarint(codegen->chunk, skipElse + 1, codegen->chunk->code.count - skipElse);

        // increment startElse, to not skip skipElse branch.
        startElse += 1 + ntReadVariant(codegen->chunk, skipElse + 1, NULL);
    }
    else
    {
        // pop condition when no has else
        emitPop(codegen, node, conditionType);
    }

    const size_t delta = startElse - elseBranch;
    ntInsertChunkVarint(codegen->chunk, elseBranch + 1, delta);
}

static void blockStatment(NT_CODEGEN *codegen, const NT_NODE *node, bool *hasReturn)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_BLOCK);

    beginScope(codegen, STT_NONE);
    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        const NT_NODE *stmt = ntListGet(node->data, i);
        statement(codegen, stmt, hasReturn);
    }
    endScope(codegen, node);
}

static void conditionalLoopStatement(NT_CODEGEN *codegen, const NT_NODE *node, bool isZero,
                                     bool *hasReturn)
{
    const size_t loopStart = codegen->chunk->code.count;

    // start
    const NT_TYPE *conditionType;
    const size_t exit = emitCondition(codegen, node, isZero, &conditionType);
    assert(conditionType);

    emitPop(codegen, node, conditionType);
    statement(codegen, node->left, hasReturn);

    // loop to start
    const size_t loopBranch = emit(codegen, node, BC_BRANCH);
    const size_t loopOffset = loopStart - loopBranch;

    const size_t exitOffset = codegen->chunk->code.count - exit;

    size_t correctedExitOffset = exitOffset;
    size_t correctedLoopOffset = loopOffset;

    do
    {
        correctedExitOffset = exitOffset + ntVarintEncodedSize(ZigZagEncoding(correctedLoopOffset));
        correctedLoopOffset = loopOffset -
                              ntVarintEncodedSize(ZigZagEncoding(correctedExitOffset)) -
                              ntVarintEncodedSize(ZigZagEncoding(correctedLoopOffset));
    } while (correctedExitOffset !=
                 exitOffset + ntVarintEncodedSize(ZigZagEncoding(correctedLoopOffset)) ||
             correctedLoopOffset != loopOffset -
                                        ntVarintEncodedSize(ZigZagEncoding(correctedExitOffset)) -
                                        ntVarintEncodedSize(ZigZagEncoding(correctedLoopOffset)));

    ntInsertChunkVarint(codegen->chunk, loopBranch + 1, correctedLoopOffset);
    ntInsertChunkVarint(codegen->chunk, exit + 1, correctedExitOffset);

    // pop condition value from stach when false
    push(codegen, node, conditionType);
    emitPop(codegen, node, conditionType);
}

static void untilStatment(NT_CODEGEN *codegen, const NT_NODE *node, bool *hasReturn)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_UNTIL);

    beginScope(codegen, STT_BREAKABLE);
    conditionalLoopStatement(codegen, node, false, hasReturn);
    endScope(codegen, node);
}

static void whileStatment(NT_CODEGEN *codegen, const NT_NODE *node, bool *hasReturn)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_WHILE);

    beginScope(codegen, STT_BREAKABLE);
    conditionalLoopStatement(codegen, node, true, hasReturn);
    endScope(codegen, node);
}

static void declareVariable(NT_CODEGEN *codegen, const NT_NODE *node)
{
    const NT_TYPE *type = NULL;
    if (node->left != NULL)
    {
        type = findType(codegen, node->left);
        if (node->right)
        {
            const NT_TYPE *initType = evalExprType(codegen, node->right);
            if (type != initType)
            {
                errorAt(codegen, node, "Invalid initalizer type. Incompatible type!");
                return;
            }
        }
    }
    else
    {
        if (node->right == NULL)
        {
            errorAt(codegen, node, "Variable must has a type or initializer.");
            return;
        }
        type = evalExprType(codegen, node->right);
    }

    if (node->right)
    {
        expression(codegen, node->right, true);
    }
    else
    {
        switch (type->stackSize)
        {
        case sizeof(uint32_t):
            emit(codegen, node, BC_ZERO_32);
            break;
        case sizeof(uint64_t):
            emit(codegen, node, BC_ZERO_64);
            break;
        default:
            errorAt(codegen, node, "CODEGEN invalid stackSize!");
            return;
        }
    }

    const NT_STRING *varName = ntCopyString(node->token.lexeme, node->token.lexemeLength);
    addLocal(codegen, varName, type);
}

static void varStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    ensureStmt(node, NK_VAR);
    declareVariable(codegen, node);
}

const char_t *const returnVariable = U"@return";

static void endFunctionScope(NT_CODEGEN *codegen, const NT_NODE *node)
{
    while (codegen->scope->type != STT_FUNCTION && codegen->scope->type != STT_METHOD)
    {
        endScope(codegen, node);
    }

    if (codegen->scope->type == STT_FUNCTION)
    {
        if (node->left == NULL)
        {
            errorAt(codegen, node, "Critical codegen error! The scope need a return value.");
        }
        assert(node->left);

        const NT_TYPE *type = evalExprType(codegen, node->left);
        expression(codegen, node->left, true);
        emitAssign(codegen, node, type, returnVariable, ntStrLen(returnVariable),
                   "Critical codegen error! The variable for return must be declared.");

        endScopeRemaining(codegen, node, type);
    }
    else
    {
        endScope(codegen, node);
    }
}

static void declareFunction(NT_CODEGEN *codegen, const NT_NODE *node, const bool returnValue)
{
    const size_t startPc = codegen->chunk->code.count;
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
        returnType = findType(codegen, node->left);

        addParam(codegen, ntCopyString(returnVariable, ntStrLen(returnVariable)), returnType);

        if (paramCount < 1)
            switch (returnType->stackSize)
            {
            case sizeof(uint32_t):
                emit(codegen, node, BC_ZERO_32);
                break;
            case sizeof(uint64_t):
                emit(codegen, node, BC_ZERO_64);
                break;
            default:
                errorAt(codegen, node, "CODEGEN invalid stackSize for return type!");
                return;
            }
    }

    for (size_t i = 0; i < paramCount; ++i)
    {
        const NT_NODE *paramNode = ntListGet(params, i);
        const NT_NODE *typeNode = paramNode->left;
        const NT_TYPE *type = findType(codegen, typeNode);
        const NT_STRING *paramName =
            ntCopyString(paramNode->token.lexeme, paramNode->token.lexemeLength);

        addParam(codegen, paramName, type);

        const NT_PARAM param = {
            .name = paramName,
            .type = type,
        };
        ntArrayAdd(&paramsArray, &param, sizeof(NT_PARAM));
    }

    for (size_t i = 0; i < ntListLen(node->right->data) && !hasReturn; ++i)
    {
        const NT_NODE *stmt = (NT_NODE *)ntListGet(node->right->data, i);
        bool statmentReturn = false;
        statement(codegen, stmt, &statmentReturn);
        hasReturn |= statmentReturn;
    }

    if (returnValue)
    {
        if (!hasReturn)
        {
            char *lname = ntToCharFixed(name, nameLen);
            errorAt(codegen, node, "Function '%s' doesn't  return a value on all code paths.",
                    lname);
            ntFree(lname);

            while (codegen->scope->type != STT_FUNCTION && codegen->scope->type != STT_METHOD)
            {
                endScope(codegen, node);
            }
            assert(codegen->scope->type != STT_METHOD);
            endScope(codegen, node);
            emit(codegen, node, BC_RETURN);
        }
    }
    else
    {
        endFunctionScope(codegen, node);
        emit(codegen, node, BC_RETURN);
    }

    const NT_TYPE *delegateType =
        ntDelegateType(codegen->assembly, returnType, paramCount, (NT_PARAM *)paramsArray.data);
    const NT_STRING *funcName = ntCopyString(name, nameLen);
    addSymbol(codegen, funcName, symbolType, delegateType, startPc);
    ntDeinitArray(&paramsArray);
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
    default:
        break;
    }
}

static void returnStatement(NT_CODEGEN *codegen, const NT_NODE *node)
{
    ensureStmt(node, NK_RETURN);
    endFunctionScope(codegen, node);
    emit(codegen, node, BC_RETURN);
}

static void statement(NT_CODEGEN *codegen, const NT_NODE *node, bool *hasReturn)
{
    *hasReturn |= false;
    if (node->type.class != NC_STMT)
    {
        errorAt(codegen, node, "Invalid node, the node must be a statment!");
        return;
    }

    switch (node->type.kind)
    {
    case NK_PRINT:
        printStatement(codegen, node);
        break;
    case NK_EXPR:
        expressionStatement(codegen, node);
        break;
    case NK_IF:
        ifStatement(codegen, node, hasReturn);
        break;
    case NK_BLOCK:
        blockStatment(codegen, node, hasReturn);
        break;
    case NK_UNTIL:
        untilStatment(codegen, node, hasReturn);
        break;
    case NK_WHILE:
        whileStatment(codegen, node, hasReturn);
        break;
    // case NK_DEF:
    //     defStatement(codegen, node);
    //     break;
    // case NK_SUB:
    //     subStatement(codegen, node);
    //     break;
    case NK_VAR:
        varStatement(codegen, node);
        break;
    case NK_RETURN:
        *hasReturn = true;
        returnStatement(codegen, node);
        break;
    default: {
        const char *const label = ntGetKindLabel(node->type.kind);
        errorAt(codegen, node, "Invalid statment. The statement with kind '%s' is invalid.", label);
        break;
    }
    }
}

bool ntGen(NT_CODEGEN *codegen, const NT_NODE **block, size_t count)
{
    codegen->had_error = false;

    // for (size_t i = 0; i < ntListLen(block->data); ++i)
    // {
    //     const NT_NODE *stmt = (NT_NODE *)ntListGet(block->data, i);
    //     if (stmt->type.)
    // }

    for (size_t i = 0; i < count; ++i)
    {
        const NT_NODE *stmt = block[i];
        declaration(codegen, stmt);
    }

    return !codegen->had_error;
}
