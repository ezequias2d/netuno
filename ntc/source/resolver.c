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
#include "resolver.h"
#include "modules/numbers.h"
#include "netuno/nir/context.h"
#include "report.h"
#include "scope.h"
#include "type.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct
{
    NT_REPORT report;
    NIR_CONTEXT *context;

    NT_TYPE *globalModule;
    NT_TYPE *module;
    NT_SCOPE *global;
    NT_SCOPE *scope;
    NT_SCOPE *functionScope;
    bool public;
} RESOLVER;

// static void addWeakSymbol(RESOLVER *r, NT_STRING *name, NT_SYMBOL_TYPE symbolType,
//                           const NT_TYPE *type, void *data)
// {
//     assert(r);
//     assert(name);
//     assert(!type || (IS_VALID_TYPE(type) && type->objectType != NT_TYPE_UNDEFINED));

//     const NT_SYMBOL entry = {
//         .symbol_name = name,
//         .type = symbolType,
//         .data = data,
//         .exprType = type,
//         .weak = true,
//     };
//     const bool result = ntInsertSymbol(r->scope, &entry);
//     assert(result);
// }

static void addWeakLocal(RESOLVER *r, NT_STRING *name, const NT_TYPE *type)
{
    assert(r);
    assert(name);
    assert(type);
    assert(type->objectType != NT_TYPE_UNDEFINED);
    assert(type->objectType != NT_TYPE_VOID);

    // TODO: check type in vstack
    const NT_SYMBOL entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_VARIABLE,
        .exprType = type,
        .weak = true,
    };
    const bool result = ntInsertSymbol(r->scope, &entry);
    assert(result);
}

static void addWeakParam(RESOLVER *r, NT_STRING *name, const NT_TYPE *type)
{
    assert(r);
    assert(name);
    assert(type);
    assert(type->objectType != NT_TYPE_UNDEFINED);
    assert(type->objectType != NT_TYPE_VOID);

    const NT_SYMBOL entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_PARAM,
        .exprType = type,
        .weak = true,
    };
    const bool result = ntInsertSymbol(r->scope, &entry);
    assert(result);
}

static const NT_TYPE *findType(RESOLVER *r, NT_NODE *typeNode)
{
    const NT_TOKEN *name = &typeNode->token;

    if (name->type == TK_KEYWORD)
    {
        // primitive
        switch (name->id)
        {
        case KW_BOOL:
            return ntBoolType(r->context);
        case KW_I32:
            return ntI32Type(r->context);
        case KW_I64:
            return ntI64Type(r->context);
        case KW_U32:
            return ntU32Type(r->context);
        case KW_U64:
            return ntU64Type(r->context);
        case KW_F32:
            return ntF32Type(r->context);
        case KW_F64:
            return ntF64Type(r->context);
        case KW_STRING:
            return ntStringType(r->context);
        default: {
            char *typeLex = ntToChar(ntGetKeywordLexeme(typeNode->token.id));
            ntErrorAtNode(&r->report, typeNode, "The keyword '%s' is not a type.", typeLex);
            ntFree(typeLex);
            return ntErrorType();
        }
        }
    }
    else
    {
        // object
        NT_SYMBOL entry;
        if (!ntLookupSymbol2(r->scope, name->lexeme, name->lexemeLength, NULL, &entry))
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            ntErrorAtNode(&r->report, typeNode, "The type '%s' don't exist.", lexeme);
            ntFree(lexeme);
            return ntErrorType();
        }

        if (entry.type != SYMBOL_TYPE_TYPE)
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            ntErrorAtNode(&r->report, typeNode, "The identifier '%s' is not a type.", lexeme);
            ntFree(lexeme);
            return ntErrorType();
        }
        return entry.exprType;
    }
}

const NT_TYPE *ntEvalExprType(NIR_CONTEXT *context, NT_REPORT *report, NT_SCOPE *table,
                              NT_NODE *node)
{
    assert(report);
    assert(node);
    assert(node->type.class == NC_EXPR);

    if (node->expressionType)
        return node->expressionType;
    assert(table);

    const NT_TYPE *left, *right;
    left = right = ntUndefinedType();

    if (node->left != NULL)
    {
        left = ntEvalExprType(context, report, table, node->left);
        if (!left)
            left = ntEvalExprType(context, report, table, node->left);
        assert(left);
    }

    if (node->right != NULL)
    {
        right = ntEvalExprType(context, report, table, node->right);
        if (!right)
            right = ntEvalExprType(context, report, table, node->right);
        assert(right);
    }

    switch (node->type.kind)
    {
    case NK_LITERAL:
        switch (node->type.literalType)
        {
        case LT_BOOL:
            assert(node->token.type == TK_KEYWORD);
            node->expressionType = ntBoolType(context);
            break;
        case LT_NONE:
            node->expressionType = ntObjectType();
            break;
        case LT_STRING:
            node->expressionType = ntStringType(context);
            break;
        case LT_I32:
            node->expressionType = ntI32Type(context);
            break;
        case LT_I64:
            node->expressionType = ntI64Type(context);
            break;
        case LT_U32:
            node->expressionType = ntU32Type(context);
            break;
        case LT_U64:
            node->expressionType = ntU64Type(context);
            break;
        case LT_F32:
            node->expressionType = ntF32Type(context);
            break;
        case LT_F64:
            node->expressionType = ntF64Type(context);
            break;
        default:
            ntErrorAtNode(report, node, "Invalid literal type! '%d'", node->type.literalType);
            node->expressionType = ntErrorType();
            break;
        }
        break;
    case NK_UNARY:
        switch (node->token.id)
        {
        case '-':
        case OP_DEC:
        case OP_INC:
            node->expressionType = left->objectType == NT_TYPE_UNDEFINED ? right : left;
            assert(node->expressionType);
            assert(node->expressionType->objectType != NT_TYPE_UNDEFINED);
            break;
        case '!':
            node->expressionType = ntBoolType(context);
            break;
        case '~':
            if (right->objectType == NT_TYPE_I32 || right->objectType == NT_TYPE_I64 ||
                right->objectType == NT_TYPE_U32 || right->objectType == NT_TYPE_U64)
                node->expressionType = right;
            else
            {
                ntErrorAtNode(
                    report, node,
                    "Invalid type for '~' operation! Must be a integer(i32, i64, u32 or u64).");
                node->expressionType = ntErrorType();
                break;
            }
            break;
        default:
            ntErrorAtNode(report, node, "Invalid unary operator!");
            node->expressionType = ntErrorType();
            break;
        }
        break;
    case NK_BINARY:
        switch (node->token.id)
        {
        case OP_NE:
        case OP_EQ:
        case '>':
        case OP_GE:
        case '<':
        case OP_LE:
            node->expressionType = ntBoolType(context);
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '|':
        case '&':
        case '^':
            if (left->objectType == NT_TYPE_CUSTOM || right->objectType == NT_TYPE_CUSTOM)
            {
                // TODO: add operators support to objects.
                ntErrorAtNode(report, node, "Invalid math operation with custom object.");
                node->expressionType = ntErrorType();
                break;
            }

            if (left->objectType < right->objectType)
                node->expressionType = left;
            else
                node->expressionType = right;
            break;
        default:
            ntErrorAtNode(report, node, "Invalid binary operation. %d", node->token.id);
            node->expressionType = ntErrorType();
            break;
        }
        break;
    case NK_LOGICAL:
        switch (node->token.id)
        {
        case OP_LOGOR:
        case OP_LOGAND:
            node->expressionType = ntBoolType(context);
            break;
        default:
            ntErrorAtNode(report, node, "Invalid logical operation. %d", node->token.id);
            node->expressionType = ntErrorType();
            break;
        }
        break;
    case NK_GET: {
        const NT_TYPE *type = ntEvalExprType(context, report, table, node->left);
        assert(type);
        assert(IS_VALID_TYPE(type));

        if (type->objectType != NT_TYPE_ERROR)
        {
            NT_SYMBOL entry;
            const bool result = ntLookupSymbol2(&type->fields, node->token.lexeme,
                                                node->token.lexemeLength, NULL, &entry);
            assert(result);
            node->expressionType = entry.exprType;
        }
        else
            node->expressionType = type;
        break;
    }
    case NK_CALL: {
        switch (left->objectType)
        {
        case NT_TYPE_I32:
        case NT_TYPE_U32:
        case NT_TYPE_F32:
        case NT_TYPE_I64:
        case NT_TYPE_U64:
        case NT_TYPE_F64:
        case NT_TYPE_STRING:
            node->expressionType = left;
            break;
        case NT_TYPE_DELEGATE:
            for (size_t i = 0; i < ntListLen(node->data); ++i)
            {
                NT_NODE *arg;
                const bool result = ntListGet(node->data, i, (void **)&arg);
                assert(result);
                ntEvalExprType(context, report, table, arg);
            }
            node->expressionType = left->delegate.returnType;
            break;
        default: {
            // char *str = ntToCharFixed(node->left->token.lexeme, node->left->token.lexemeLength);
            // ntErrorAtNode(report, node->left, "The function or method '%s' must be declareed.",
            //               str);
            // ntFree(str);
            node->expressionType = ntErrorType();
            break;
        }
        }
        break;
    }
    case NK_VARIABLE: {
        NT_SYMBOL entry;
        if (!ntLookupSymbol2(table, node->token.lexeme, node->token.lexemeLength, NULL, &entry))
        {
            ntErrorAtNode(report, node, "The symbol must be declared.");
            node->expressionType = ntErrorType();
            break;
        }

        if ((entry.type &
             (SYMBOL_TYPE_VARIABLE | SYMBOL_TYPE_CONSTANT | SYMBOL_TYPE_PARAM | SYMBOL_TYPE_TYPE |
              SYMBOL_TYPE_FUNCTION | SYMBOL_TYPE_SUBROUTINE | SYMBOL_TYPE_MODULE)) == 0)
        {
            char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            ntErrorAtNode(
                report, node,
                "The symbol '%s' is not a constant, parameter, variable, method or function!", str);
            ntFree(str);
            node->expressionType = ntErrorType();
            break;
        }
        node->expressionType = entry.exprType;
        break;
    }
    case NK_ASSIGN: {
        if (left != right)
        {
            char *leftName = ntStringToChar(left->typeName);
            char *rightName = ntStringToChar(right->typeName);
            ntErrorAtNode(
                report, node,
                "Invalid type, variable is of type %s, but the value expression to assign is "
                "%s.",
                leftName, rightName);
            ntFree(leftName);
            ntFree(rightName);
            node->expressionType = ntErrorType();
            break;
        }
        node->expressionType = left;
        break;
    }
    default:
        ntErrorAtNode(report, node, "AST invalid format, node kind cannot be %s!",
                      ntGetKindLabel(node->type.kind));
        node->expressionType = ntErrorType();
        break;
    }

    return node->expressionType;
}

static NT_SCOPE *beginScope(RESOLVER *r, NT_SCOPE_TYPE type)
{
    r->scope = ntCreateSymbolTable(r->scope, type, NULL);

    if (type == STT_FUNCTION || type == STT_METHOD)
        r->functionScope = r->scope;

    return r->scope;
}

static void endScope(RESOLVER *r)
{
    NT_SCOPE *const oldScope = r->scope;
    r->scope = (NT_SCOPE *)oldScope->parent;
}

static const NT_TYPE *evalIfReturnType(NIR_CONTEXT *context, NT_REPORT *report, NT_SCOPE *table,
                                       NT_NODE *node)
{
    NT_NODE *const thenBranch = node->left;
    assert(thenBranch->type.class == NC_STMT);

    const NT_TYPE *type = ntUndefinedType();

    switch (thenBranch->type.kind)
    {
    case NK_BLOCK:
        type = ntEvalBlockReturnType(context, report, NULL, thenBranch);
        break;
    case NK_RETURN:
        assert(thenBranch->left);
        assert(thenBranch->left->type.class == NC_EXPR);
        type = ntEvalExprType(context, report, table, thenBranch->left);
        break;
    default:
        break;
    }

    NT_NODE *const elseBranch = node->right;

    if (elseBranch)
    {
        const NT_TYPE *elseType = NULL;
        switch (elseBranch->type.kind)
        {
        case NK_IF:
            elseType = evalIfReturnType(context, report, table, elseBranch);
            break;
        case NK_BLOCK:
            elseType = ntEvalBlockReturnType(context, report, NULL, elseBranch);
            break;
        case NK_RETURN:
            assert(elseBranch->left);
            assert(elseBranch->left->type.class == NC_EXPR);
            elseType = ntEvalExprType(context, report, table, elseBranch->left);
            break;
        default:
            break;
        }

        if (elseType->objectType != NT_TYPE_UNDEFINED && type->objectType != NT_TYPE_UNDEFINED &&
            elseType != type)
        {
            char *expectTypeName = ntStringToChar(type->typeName);
            char *currentTypeName = ntStringToChar(elseType->typeName);
            // more than one type as return
            ntErrorAtNode(report, node,
                          "The same type must be used in all return statements of if branches, "
                          "expect type is %s, not %s",
                          expectTypeName, currentTypeName);
            ntFree(expectTypeName);
            ntFree(currentTypeName);
            type = ntErrorType();
        }
        else if (elseType->objectType != NT_TYPE_UNDEFINED && type->objectType != NT_TYPE_UNDEFINED)
            type = elseType;
    }

    return type;
}

const NT_TYPE *ntEvalBlockReturnType(NIR_CONTEXT *context, NT_REPORT *report, NT_SCOPE *table,
                                     NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_BLOCK);

    if (node->expressionType)
        return node->expressionType;

    assert(table);

    const NT_TYPE *blockReturnType = ntUndefinedType();

    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        NT_NODE *stmt;
        const bool result = ntListGet(node->data, i, (void **)&stmt);
        assert(result);

        const NT_TYPE *tmp = ntUndefinedType();
        assert(stmt->type.class == NC_STMT);

        switch (stmt->type.kind)
        {
        case NK_RETURN:
            tmp = ntEvalExprType(context, report, table, stmt->left);
            break;
        case NK_BLOCK:
            tmp = ntEvalBlockReturnType(context, report, NULL, stmt);
            break;
        case NK_IF:
            tmp = evalIfReturnType(context, report, table, stmt);
            break;
        case NK_WHILE:
        case NK_UNTIL:
            assert(stmt->left->type.class == NC_STMT);
            switch (stmt->left->type.kind)
            {
            case NK_BLOCK:
                tmp = ntEvalBlockReturnType(context, report, NULL, stmt->left);
                break;
            case NK_RETURN:
                assert(stmt->left->left);
                assert(stmt->left->left->type.class == NC_EXPR);
                tmp = ntEvalExprType(context, report, table, stmt->left->left);
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

        if (blockReturnType->objectType == NT_TYPE_UNDEFINED &&
            tmp->objectType != NT_TYPE_UNDEFINED)
            blockReturnType = tmp;
        else if (blockReturnType->objectType != NT_TYPE_UNDEFINED &&
                 tmp->objectType != NT_TYPE_UNDEFINED && tmp != blockReturnType)
        {
            char *expectTypeName = ntStringToChar(blockReturnType->typeName);
            char *currentTypeName = ntStringToChar(tmp->typeName);
            // more than one type as return
            ntErrorAtNode(report, stmt,
                          "The same type must be used in all return statements, expect type is %s, "
                          "not %s",
                          expectTypeName, currentTypeName);
            ntFree(expectTypeName);
            ntFree(currentTypeName);
            blockReturnType = ntErrorType();
        }
    }

    node->expressionType = blockReturnType;
    return blockReturnType;
}

static void statement(RESOLVER *r, NT_NODE *node, const NT_TYPE **returnType);

static void ifStatement(RESOLVER *r, const NT_NODE *node, const NT_TYPE **returnType)
{
    const bool hasElse = node->right != NULL;

    const NT_TYPE *thenReturnType = ntUndefinedType();
    const NT_TYPE *elseReturnType = ntUndefinedType();

    // condition
    ntEvalExprType(r->context, &r->report, r->scope, node->condition);

    // then body
    statement(r, node->left, &thenReturnType);

    // else body if exist
    if (hasElse)
    {
        // else body
        statement(r, node->right, &elseReturnType);
        if ((*returnType)->objectType == NT_TYPE_UNDEFINED)
            *returnType = elseReturnType;
        else if (elseReturnType->objectType != NT_TYPE_UNDEFINED && elseReturnType != *returnType)
        {
            char *expect = ntStringToChar((*returnType)->typeName);
            char *current = ntStringToChar(elseReturnType->typeName);
            ntErrorAtNode(&r->report, node,
                          "The else branch expect '%s' type as return, but is '%s'.", expect,
                          current);
            ntFree(expect);
            ntFree(current);
            elseReturnType = thenReturnType = ntErrorType();
        }
    }

    if (thenReturnType->objectType != NT_TYPE_UNDEFINED &&
        elseReturnType->objectType != NT_TYPE_UNDEFINED &&
        (*returnType)->objectType != NT_TYPE_UNDEFINED)
        *returnType = thenReturnType;
}

static void blockStatment(RESOLVER *r, NT_NODE *node, const NT_TYPE **returnType)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_BLOCK);

    const NT_TYPE *blockReturnType = ntUndefinedType();

    NT_SCOPE *const scope = beginScope(r, STT_NONE);
    node->userdata = scope;
    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        NT_NODE *stmt;
        const bool result = ntListGet(node->data, i, (void **)&stmt);
        assert(result);

        statement(r, stmt, &blockReturnType);
    }
    if ((*returnType)->objectType == NT_TYPE_UNDEFINED)
        *returnType = blockReturnType;

    ntEvalBlockReturnType(r->context, &r->report, r->scope, node);

    endScope(r);
}

static void loopStatment(RESOLVER *r, const NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_UNTIL || node->type.kind == NK_WHILE);

    const NT_TYPE *returnType = ntUndefinedType();

    beginScope(r, STT_BREAKABLE);
    // body
    statement(r, node->left, &returnType);
    ntEvalBlockReturnType(r->context, &r->report, r->scope, node->left);
    endScope(r);

    ntEvalExprType(r->context, &r->report, r->scope, node->condition);
}

static void varStatement(RESOLVER *r, const NT_NODE *node)
{
    assert(node->type.class == NC_STMT && node->type.kind == NK_VAR);

    const NT_TYPE *type = NULL;
    if (node->left != NULL)
    {
        type = findType(r, node->left);
        if (node->right)
        {
            const NT_TYPE *initType = ntEvalExprType(r->context, &r->report, r->scope, node->right);
            if (type != initType)
            {
                ntErrorAtNode(&r->report, node, "Invalid initalizer type. Incompatible type!");
                return;
            }
        }
    }
    else
    {
        if (node->right == NULL)
        {
            ntErrorAtNode(&r->report, node,
                          "Variable declarations must has a type or initializer.");
            return;
        }
        type = ntEvalExprType(r->context, &r->report, r->scope, node->right);
    }

    addWeakLocal(r, ntCopyString(node->token.lexeme, node->token.lexemeLength), type);
}

static void endFunctionScope(RESOLVER *r, const NT_NODE *node, const NT_TYPE **returnType,
                             bool isEndScope)
{
    const NT_SCOPE *functionScope = r->scope;
    while (functionScope->type != STT_FUNCTION && functionScope->type != STT_METHOD)
    {
        functionScope = functionScope->parent;
    }

    if (functionScope->type == STT_FUNCTION)
    {
        if (node->left == NULL)
        {
            ntErrorAtNode(&r->report, node,
                          "Critical modgen error! The scope need a return value.");
        }
        assert(node->left);

        const NT_TYPE *type = ntEvalExprType(r->context, &r->report, r->scope, node->left);
        if (r->scope->scopeReturnType == NULL)
            r->scope->scopeReturnType = type;

        assert(r->scope->scopeReturnType == type);

        if (returnType && *returnType == NULL)
            *returnType = type;
    }

    if (isEndScope)
    {
        r->scope = (NT_SCOPE *)functionScope->parent;
    }
}

static void returnStatement(RESOLVER *r, const NT_NODE *node, const NT_TYPE **returnType)
{
    assert(node->type.class == NC_STMT && node->type.kind == NK_RETURN);

    if (node->left)
    {
        const NT_TYPE *const type = ntEvalExprType(r->context, &r->report, r->scope, node->left);
        assert(type);
        assert(IS_VALID_TYPE(type));

        if (type->objectType == NT_TYPE_UNDEFINED)
            ntErrorAtNode(&r->report, node, "Return statement cannot has undefined type value.");

        if (type->objectType == NT_TYPE_VOID)
            ntErrorAtNode(&r->report, node, "Return statement need a expresion with value.");

        *returnType = type;
    }

    endFunctionScope(r, node, returnType, false);
}

static void expressionStatement(RESOLVER *r, NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_EXPR);
    ntEvalExprType(r->context, &r->report, r->scope, node->left);
}

static void statement(RESOLVER *r, NT_NODE *node, const NT_TYPE **returnType)
{
    if (node->type.class != NC_STMT)
    {
        ntErrorAtNode(&r->report, node, "Invalid node, the node must be a statment!");
        return;
    }

    switch (node->type.kind)
    {
    case NK_EXPR:
        expressionStatement(r, node);
        break;
    case NK_IF:
        ifStatement(r, node, returnType);
        break;
    case NK_BLOCK:
        blockStatment(r, node, returnType);
        break;
    case NK_UNTIL:
    case NK_WHILE:
        loopStatment(r, node);
        break;
    case NK_VAR:
        varStatement(r, node);
        break;
    case NK_RETURN:
        returnStatement(r, node, returnType);
        break;
    case NK_BREAK:
    case NK_CONTINUE:
        break;
    default: {
        const char *const label = ntGetKindLabel(node->type.kind);
        ntErrorAtNode(&r->report, node,
                      "Invalid statment. The statement with kind '%s' is invalid.", label);
        break;
    }
    }
}

static void addWeakFunction(NT_SCOPE *scope, NT_STRING *name, const NT_TYPE *delegateType,
                            bool public)
{
    assert(scope);
    assert(name);
    assert(delegateType);
    assert(delegateType->objectType == NT_TYPE_DELEGATE);
    assert(IS_VALID_TYPE(delegateType));

    const NT_SYMBOL_TYPE symbolType = ((delegateType->delegate.returnType != ntUndefinedType() &&
                                        delegateType->delegate.returnType != ntVoidType())
                                           ? SYMBOL_TYPE_FUNCTION
                                           : SYMBOL_TYPE_SUBROUTINE) |
                                      (public ? SYMBOL_TYPE_PUBLIC : SYMBOL_TYPE_PRIVATE);

    const NT_SYMBOL entry = {
        .symbol_name = name,
        .type = symbolType,
        .exprType = delegateType,
        .weak = true,
    };
    bool result = ntInsertSymbol(scope, &entry);
    if (!result)
        result = ntUpdateSymbol(scope, &entry);
    assert(result);
}

static void declareWeakFunction(RESOLVER *r, NT_NODE *node, const bool returnValue)
{
    assert(r);

    const char_t *name = node->token.lexeme;
    const size_t nameLen = node->token.lexemeLength;

    bool hasReturn = false;

    NT_SCOPE *const scope = beginScope(r, returnValue ? STT_FUNCTION : STT_METHOD);
    node->userdata = scope;

    const NT_LIST params = node->data;
    const size_t paramCount = ntListLen(node->data);

    NT_ARRAY paramsArray;
    ntInitArray(&paramsArray);

    for (size_t i = 0; i < paramCount; ++i)
    {
        NT_NODE *paramNode;
        const bool result = ntListGet(params, i, (void **)&paramNode);
        assert(result);

        NT_NODE *typeNode = paramNode->left;
        const NT_TYPE *type = findType(r, typeNode);

        NT_STRING *paramName = ntCopyString(paramNode->token.lexeme, paramNode->token.lexemeLength);
        addWeakParam(r, paramName, type);

        const NT_PARAM param = {
            .name = ntRefString(paramName),
            .type = type,
        };
        ntArrayAdd(&paramsArray, &param, sizeof(NT_PARAM));
    }

    const NT_TYPE *returnType;

    if (returnValue)
    {
        returnType = ntUndefinedType();
        if (node->left)
        {
            returnType = findType(r, node->left);

            // add function before statements (recursive functions is compatible)
            const NT_TYPE *delegateType =
                ntTakeDelegateType(returnType, paramCount, (NT_PARAM *)paramsArray.data);
            NT_STRING *funcName = ntCopyString(name, nameLen);
            addWeakFunction((NT_SCOPE *)r->scope->parent, funcName, delegateType, r->public);
        }
    }
    else
    {
        returnType = ntVoidType();

        // add sub procedure (recursive sub procedure is compatible)
        const NT_TYPE *delegateType =
            ntTakeDelegateType(returnType, paramCount, (NT_PARAM *)paramsArray.data);
        NT_STRING *funcName = ntCopyString(name, nameLen);
        addWeakFunction((NT_SCOPE *)r->scope->parent, funcName, delegateType, r->public);
    }

    const NT_TYPE *statmentReturn = ntUndefinedType();
    for (size_t i = 0; i < ntListLen(node->right->data); ++i)
    {
        NT_NODE *stmt;
        const bool result = (NT_NODE *)ntListGet(node->right->data, i, (void **)&stmt);
        assert(result);

        statement(r, stmt, &statmentReturn);
        if (statmentReturn->objectType != NT_TYPE_UNDEFINED)
            hasReturn |= true;
    }

    if (returnValue)
    {
        returnType = ntEvalBlockReturnType(r->context, &r->report, r->scope, node->right);
        // add function after statements (recursive functions is not compatible)
        const NT_TYPE *delegateType =
            ntTakeDelegateType(returnType, paramCount, (NT_PARAM *)paramsArray.data);
        NT_STRING *funcName = ntCopyString(name, nameLen);
        addWeakFunction((NT_SCOPE *)r->scope->parent, funcName, delegateType, r->public);
    }

    ntDeinitArray(&paramsArray);

    if (returnValue)
    {
        if (!hasReturn)
        {
            char *lname = ntToCharFixed(name, nameLen);
            ntErrorAtNode(&r->report, node,
                          "Function '%s' doesn't  return a value on all code paths.", lname);
            ntFree(lname);
            assert(r->scope->type != STT_METHOD);
        }
    }
    else
    {
        endFunctionScope(r, node, NULL, true);
    }
}

static void defStatement(RESOLVER *r, NT_NODE *node)
{
    assert(r);
    assert(node->type.class == NC_STMT && node->type.kind == NK_DEF);
    declareWeakFunction(r, node, true);
}

static void subStatement(RESOLVER *r, NT_NODE *node)
{
    assert(r);
    assert(node->type.class == NC_STMT && node->type.kind == NK_SUB);
    declareWeakFunction(r, node, false);
}

static void importStatement(RESOLVER *r, NT_NODE *node)
{
    assert(r);
    assert(node->type.class == NC_STMT && node->type.kind == NK_IMPORT);

    NT_NODE *current = node->left;
    assert(current);
    assert(current->type.class == NC_EXPR && current->type.kind == NK_GET);

    const NT_SCOPE *table = r->global;
    const NT_TYPE *importedModule = NULL;
    assert(table);

    NT_SYMBOL entry;

    bool result = false;
    do
    {
        result = ntLookupSymbolCurrent2(table, current->token.lexeme, current->token.lexemeLength,
                                        &entry);

        if (!result)
        {
            ntErrorAtNode(&r->report, node, "Cannot resolver the import symbol");
            break;
        }

        if ((entry.type & SYMBOL_TYPE_MODULE) != SYMBOL_TYPE_MODULE)
        {
            ntErrorAtNode(&r->report, node, "Symbol must be a module");
            break;
        }

        importedModule = entry.exprType;
        assert(importedModule);
        assert(importedModule->objectType == NT_TYPE_MODULE);

        table = &importedModule->fields;
        current = current->right;
    } while (current != NULL);

    if (result)
        ntInsertSymbol(&r->module->fields, &entry);

    // if (r->module != r->globalModule)
    //     ntWarningAtNode(node, "Consider only use imports on file module");
}

static void declaration(RESOLVER *r, NT_NODE *node)
{
    assert(node);
    assert(node->type.class == NC_STMT);

    switch (node->type.kind)
    {
    case NK_DEF:
        defStatement(r, node);
        break;
    case NK_SUB:
        subStatement(r, node);
        break;
    case NK_VAR:
        varStatement(r, node);
        break;
    case NK_IMPORT:
        importStatement(r, node);
        break;
    default:
        break;
    }
}

static void addType(RESOLVER *r, const NT_TYPE *type)
{
    NT_SYMBOL entry = {
        .symbol_name = type->typeName,
        .type = SYMBOL_TYPE_TYPE,
        .exprType = type,
        .weak = false,
    };
    ntInsertSymbol(r->scope, &entry);
}

static void module(RESOLVER *r, NT_NODE *node)
{
    assert(r);
    assert(node);
    assert(node->type.class == NC_STMT && node->type.kind == NK_MODULE);

    NT_TYPE *const module = ntTakeType(
        NT_TYPE_MODULE, ntCopyString(node->token.lexeme, node->token.lexemeLength), ntObjectType());
    assert(module);
    assert(module->objectType == NT_TYPE_MODULE);
    node->expressionType = module;

    const bool savePublic = r->public;
    NT_TYPE *const saveModule = r->module;

    r->module = module;

    addType(r, ntI32Type(r->context));
    addType(r, ntI64Type(r->context));
    addType(r, ntU32Type(r->context));
    addType(r, ntU64Type(r->context));
    addType(r, ntF32Type(r->context));
    addType(r, ntF64Type(r->context));
    addType(r, ntStringType(NULL));

    const size_t count = ntListLen(node->data);
    for (size_t i = 0; i < count; ++i)
    {
        NT_NODE *stmt;
        const bool result = ntListGet(node->data, i, (void **)&stmt);
        assert(result);
        assert(stmt->type.class == NC_STMT);

        switch (stmt->type.kind)
        {
        case NK_PUBLIC:
            r->public = true;
            break;
        case NK_PRIVATE:
            r->public = false;
            break;
        default:
            declaration(r, stmt);
            break;
        }
    }

    r->module = saveModule;
    r->public = savePublic;
}

bool ntResolve(NIR_CONTEXT *context, NT_SCOPE *globalTable, size_t moduleNodeCount,
               NT_NODE **moduleNodes)
{
    assert(moduleNodeCount > 0);
    assert(moduleNodes != NULL);

    RESOLVER resolver = {
        .module = NULL,
        .globalModule = NULL,
        .global = globalTable,
        .scope = globalTable,
        .functionScope = NULL,
        .public = false,
        .report.had_error = false,
        .context = context,
    };

    for (size_t i = 0; i < moduleNodeCount; ++i)
    {
        NT_NODE *const current = moduleNodes[i];
        resolver.globalModule = (NT_TYPE *)current->userdata;

        module(&resolver, current);
    }

    return !resolver.report.had_error;
}
