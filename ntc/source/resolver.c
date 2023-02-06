#include "resolver.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct
{
    NT_ASSEMBLY *assembly;
    NT_MODULE *globalModule;
    NT_MODULE *module;
    NT_SYMBOL_TABLE *global;
    NT_SYMBOL_TABLE *scope;
    NT_SYMBOL_TABLE *functionScope;
    bool public;
    bool had_error;
} RESOLVER;

static void addWeakSymbol(RESOLVER *r, const NT_STRING *name, NT_SYMBOL_TYPE symbolType,
                          const NT_TYPE *type, void *data)
{
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = symbolType,
        .data = data,
        .exprType = type,
        .weak = true,
    };
    const bool result = ntInsertSymbol(r->scope, &entry);
    assert(result);
}

static void addWeakLocal(RESOLVER *r, const NT_STRING *name, const NT_TYPE *type)
{
    // TODO: check type in vstack
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_VARIABLE,
        .data = 0,
        .exprType = type,
        .weak = true,
    };
    const bool result = ntInsertSymbol(r->scope, &entry);
    assert(result);
}

static void addWeakParam(RESOLVER *r, const NT_STRING *name, const NT_TYPE *type)
{
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = SYMBOL_TYPE_PARAM,
        .data = 0,
        .exprType = type,
        .weak = true,
    };
    const bool result = ntInsertSymbol(r->scope, &entry);
    assert(result);
}

static void vErrorAt(RESOLVER *r, const NT_NODE *node, const char *message, va_list args)
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
    r->had_error = true;
}

static void vWarningAt(const NT_NODE *node, const char *message, va_list args)
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
}

static void errorAt(RESOLVER *r, const NT_NODE *node, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    vErrorAt(r, node, message, vl);
    va_end(vl);
}

static void warningAt(const NT_NODE *node, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    vWarningAt(node, message, vl);
    va_end(vl);
}

static const NT_TYPE *findType(RESOLVER *r, NT_NODE *typeNode)
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
            errorAt(r, typeNode, "The keyword '%s' is not a type.", typeLex);
            ntFree(typeLex);
            return ntErrorType();
        }
        }
    }
    else
    {
        // object
        NT_SYMBOL_ENTRY entry;
        if (!ntLookupSymbol(r->scope, name->lexeme, name->lexemeLength, NULL, &entry))
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            errorAt(r, typeNode, "The type '%s' don't exist.", lexeme);
            ntFree(lexeme);
            return NULL;
        }

        if (entry.type != SYMBOL_TYPE_TYPE)
        {
            char *lexeme = ntToCharFixed(name->lexeme, name->lexemeLength);
            errorAt(r, typeNode, "The identifier '%s' is not a type.", lexeme);
            ntFree(lexeme);
            return NULL;
        }
        return entry.exprType;
    }
}

static bool findSymbol(RESOLVER *r, const char_t *name, const size_t length,
                       NT_SYMBOL_ENTRY *pEntry)
{
    return ntLookupSymbol(r->scope, name, length, NULL, pEntry);
}

static const NT_TYPE *evalExprType(RESOLVER *r, NT_NODE *node)
{
    assert(node->type.class == NC_EXPR);

    if (node->expressionType)
        return node->expressionType;

    const NT_TYPE *left = NULL;
    const NT_TYPE *right = NULL;

    if (node->left != NULL)
    {
        left = evalExprType(r, node->left);
        assert(left);
    }

    if (node->right != NULL)
    {
        right = evalExprType(r, node->right);
        assert(right);
    }

    switch (node->type.kind)
    {
    case NK_LITERAL:
        switch (node->type.literalType)
        {
        case LT_BOOL:
            assert(node->token.type == TK_KEYWORD);
            node->expressionType = ntBoolType();
            break;
        case LT_NONE:
            node->expressionType = ntI32Type();
            break;
        case LT_STRING:
            node->expressionType = ntStringType();
            break;
        case LT_I32:
            node->expressionType = ntI32Type();
            break;
        case LT_I64:
            node->expressionType = ntI64Type();
            break;
        case LT_U32:
            node->expressionType = ntU32Type();
            break;
        case LT_U64:
            node->expressionType = ntU64Type();
            break;
        case LT_F32:
            node->expressionType = ntF32Type();
            break;
        case LT_F64:
            node->expressionType = ntF64Type();
            break;
        default:
            errorAt(r, node, "Invalid literal type! '%d'", node->type.literalType);
            break;
        }
        break;
    case NK_UNARY:
        switch (node->token.id)
        {
        case '-':
        case OP_DEC:
        case OP_INC:
            node->expressionType = left == NULL ? right : left;
            break;
        case '!':
            node->expressionType = ntBoolType();
            break;
        case '~':
            if (right->objectType == NT_OBJECT_I32 || right->objectType == NT_OBJECT_I64 ||
                right->objectType == NT_OBJECT_U32 || right->objectType == NT_OBJECT_U64)
                node->expressionType = right;
            else
            {
                errorAt(r, node,
                        "Invalid type for '~' operation! Must be a integer(i32, i64, u32 or u64).");
                return NULL;
            }
            break;
        default:
            errorAt(r, node, "Invalid unary operator!");
            return NULL;
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
            node->expressionType = ntBoolType();
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '|':
        case '&':
        case '^':
            left = evalExprType(r, node->left);
            right = evalExprType(r, node->right);

            if (left->objectType == NT_OBJECT_CUSTOM || right->objectType == NT_OBJECT_CUSTOM)
            {
                // TODO: add operators support to objects.
                errorAt(r, node, "Invalid math operation with custom object.");
                return NULL;
            }

            if (left->objectType < right->objectType)
                node->expressionType = left;
            else
                node->expressionType = right;
            break;
        default:
            errorAt(r, node, "Invalid binary operation. %d", node->token.id);
            return NULL;
        }
        break;
    case NK_LOGICAL:
        switch (node->token.id)
        {
        case OP_LOGOR:
        case OP_LOGAND:
            node->expressionType = ntBoolType();
            break;
        default:
            errorAt(r, node, "Invalid logical operation. %d", node->token.id);
            return NULL;
        }
    case NK_GET: {
        const NT_TYPE *type = evalExprType(r, node->left);
        assert(type);
        assert(IS_VALID_TYPE(type));

        NT_SYMBOL_ENTRY entry;
        const bool result = ntLookupSymbol(&type->fields, node->token.lexeme,
                                           node->token.lexemeLength, NULL, &entry);
        assert(result);
        node->expressionType = entry.exprType;

        break;
    }
    case NK_CALL: {
        switch (left->objectType)
        {
        case NT_OBJECT_DELEGATE:
            for (size_t i = 0; i < ntListLen(node->data); ++i)
            {
                NT_NODE *arg = (NT_NODE *)ntListGet(node->data, i);
                evalExprType(r, arg);
            }
            node->expressionType = ((const NT_DELEGATE_TYPE *)left)->returnType;
            break;
        default: {
            char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            errorAt(r, node, "The function or method '%s' must be declareed.", str);
            ntFree(str);
            return NULL;
        }
        }
        break;
    }
    case NK_VARIABLE: {
        NT_SYMBOL_ENTRY entry;
        if (!findSymbol(r, node->token.lexeme, node->token.lexemeLength, &entry))
        {
            errorAt(r, node, "The variable must be declared.");
            break;
        }

        if ((entry.type &
             (SYMBOL_TYPE_VARIABLE | SYMBOL_TYPE_CONSTANT | SYMBOL_TYPE_PARAM | SYMBOL_TYPE_TYPE |
              SYMBOL_TYPE_FUNCTION | SYMBOL_TYPE_SUBROUTINE | SYMBOL_TYPE_MODULE)) == 0)
        {
            char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            errorAt(r, node,
                    "The symbol '%s' is not a constant, parameter, variable, method or function!",
                    str);
            ntFree(str);
            break;
        }
        node->expressionType = entry.exprType;
        break;
    }
    break;
    case NK_ASSIGN: {
        if (left != right)
        {
            char *leftName = ntToCharFixed(left->typeName->chars, left->typeName->length);
            char *rightName = ntToCharFixed(right->typeName->chars, right->typeName->length);
            errorAt(r, node,
                    "Invalid type, variable is of type %s, but the value expression to assign is "
                    "%s.",
                    leftName, rightName);
            ntFree(leftName);
            ntFree(rightName);
        }
        node->expressionType = left;
        break;
    }
    break;
    default:
        errorAt(r, node, "AST invalid format, node kind cannot be %s!",
                ntGetKindLabel(node->type.kind));
        return NULL;
    }

    return node->expressionType;
}

static const NT_TYPE *evalBlockReturnType(RESOLVER *r, NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_BLOCK);

    if (node->expressionType)
        return node->expressionType;

    const NT_TYPE *blockReturnType = NULL;

    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        NT_NODE *stmt = (NT_NODE *)ntListGet(node->data, i);
        const NT_TYPE *tmp = NULL;
        assert(stmt->type.class == NC_STMT);

        switch (stmt->type.kind)
        {
        case NK_RETURN:
            tmp = evalExprType(r, stmt->left);
            break;
        case NK_BLOCK:
            tmp = evalBlockReturnType(r, stmt);
            break;
        case NK_IF: {
            assert(stmt->left->type.class == NC_STMT);
            switch (stmt->left->type.kind)
            {
            case NK_BLOCK:
                tmp = evalBlockReturnType(r, stmt->left);
                break;
            case NK_RETURN:
                assert(stmt->left->left);
                assert(stmt->left->left->type.class == NC_EXPR);
                tmp = evalExprType(r, stmt->left->left);
                break;
            default:
                break;
            }

            if (stmt->right) // else branch
            {
                const NT_TYPE *elseTmp = NULL;
                switch (stmt->right->type.kind)
                {
                case NK_BLOCK:
                    elseTmp = evalBlockReturnType(r, stmt->right);
                    break;
                case NK_RETURN:
                    assert(stmt->right->left);
                    assert(stmt->right->left->type.class == NC_EXPR);
                    elseTmp = evalExprType(r, stmt->right->left);
                    break;
                default:
                    break;
                }

                if (elseTmp != NULL && tmp != NULL && elseTmp != tmp)
                {
                    char *expectTypeName =
                        ntToCharFixed(tmp->typeName->chars, tmp->typeName->length);
                    char *currentTypeName =
                        ntToCharFixed(elseTmp->typeName->chars, elseTmp->typeName->length);
                    // more than one type as return
                    errorAt(r, stmt,
                            "The same type must be used in all return statements of if branches, "
                            "expect type is %s, not %s",
                            expectTypeName, currentTypeName);
                    ntFree(expectTypeName);
                    ntFree(currentTypeName);
                }
                else if (elseTmp != NULL && tmp == NULL)
                    tmp = elseTmp;
            }
        }
        break;
        case NK_WHILE:
        case NK_UNTIL:
            assert(stmt->left->type.class == NC_STMT);
            switch (stmt->left->type.kind)
            {
            case NK_BLOCK:
                tmp = evalBlockReturnType(r, stmt->left);
                break;
            case NK_RETURN:
                assert(stmt->left->left);
                assert(stmt->left->left->type.class == NC_EXPR);
                tmp = evalExprType(r, stmt->left->left);
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

        if (blockReturnType == NULL && tmp != NULL)
            blockReturnType = tmp;
        else if (blockReturnType != NULL && tmp != NULL && tmp != blockReturnType)
        {
            char *expectTypeName =
                ntToCharFixed(blockReturnType->typeName->chars, blockReturnType->typeName->length);
            char *currentTypeName = ntToCharFixed(tmp->typeName->chars, tmp->typeName->length);
            // more than one type as return
            errorAt(r, stmt,
                    "The same type must be used in all return statements, expect type is %s, "
                    "not %s",
                    expectTypeName, currentTypeName);
            ntFree(expectTypeName);
            ntFree(currentTypeName);
        }
    }

    node->expressionType = blockReturnType;
    return blockReturnType;
}

static void statement(RESOLVER *r, NT_NODE *node, const NT_TYPE **returnType);

static void ifStatement(RESOLVER *r, const NT_NODE *node, const NT_TYPE **returnType)
{
    const bool hasElse = node->right != NULL;

    const NT_TYPE *thenReturnType = NULL;
    const NT_TYPE *elseReturnType = NULL;

    // condition
    evalExprType(r, node->condition);

    // then body
    statement(r, node->left, &thenReturnType);

    // else body if exist
    if (hasElse)
    {
        // else body
        statement(r, node->right, &elseReturnType);
        if (*returnType == NULL)
            *returnType = elseReturnType;
        else if (elseReturnType && elseReturnType != *returnType)
        {
            char *expect =
                ntToCharFixed((*returnType)->typeName->chars, (*returnType)->typeName->length);
            char *current =
                ntToCharFixed(elseReturnType->typeName->chars, elseReturnType->typeName->length);
            errorAt(r, node, "The else branch expect '%s' type as return, but is '%s'.", expect,
                    current);
            ntFree(expect);
            ntFree(current);
        }
    }

    if (thenReturnType && elseReturnType && !*returnType)
        *returnType = thenReturnType;
}

static NT_SYMBOL_TABLE *beginScope(RESOLVER *r, NT_SYMBOL_TABLE_TYPE type)
{
    r->scope = ntCreateSymbolTable(r->scope, type, NULL);

    if (type == STT_FUNCTION || type == STT_METHOD)
        r->functionScope = r->scope;

    return r->scope;
}

static void endScope(RESOLVER *r)
{
    NT_SYMBOL_TABLE *const oldScope = r->scope;
    r->scope = oldScope->parent;
}

static void blockStatment(RESOLVER *r, NT_NODE *node, const NT_TYPE **returnType)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_BLOCK);

    const NT_TYPE *blockReturnType = NULL;

    NT_SYMBOL_TABLE *const scope = beginScope(r, STT_NONE);
    node->userdata = scope;
    for (size_t i = 0; i < ntListLen(node->data); ++i)
    {
        NT_NODE *stmt = ntListGet(node->data, i);
        statement(r, stmt, &blockReturnType);
    }
    if (*returnType == NULL)
        *returnType = blockReturnType;

    endScope(r);
}

static void loopStatment(RESOLVER *r, const NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_UNTIL || node->type.kind == NK_WHILE);

    const NT_TYPE *returnType = NULL;

    evalExprType(r, node->condition);

    beginScope(r, STT_BREAKABLE);
    // body
    statement(r, node->left, &returnType);
    endScope(r);
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
            const NT_TYPE *initType = evalExprType(r, node->right);
            if (type != initType)
            {
                errorAt(r, node, "Invalid initalizer type. Incompatible type!");
                return;
            }
        }
    }
    else
    {
        if (node->right == NULL)
        {
            errorAt(r, node, "Variable must has a type or initializer.");
            return;
        }
        type = evalExprType(r, node->right);
    }

    const NT_STRING *varName = ntCopyString(node->token.lexeme, node->token.lexemeLength);
    addWeakLocal(r, varName, type);
}

static void endFunctionScope(RESOLVER *r, const NT_NODE *node, const NT_TYPE **returnType,
                             bool isEndScope)
{
    const NT_SYMBOL_TABLE *functionScope = r->scope;
    while (functionScope->type != STT_FUNCTION && functionScope->type != STT_METHOD)
    {
        functionScope = functionScope->parent;
    }

    if (functionScope->type == STT_FUNCTION)
    {
        if (node->left == NULL)
        {
            errorAt(r, node, "Critical modgen error! The scope need a return value.");
        }
        assert(node->left);

        const NT_TYPE *type = evalExprType(r, node->left);
        if (r->scope->scopeReturnType == NULL)
            r->scope->scopeReturnType = type;

        assert(r->scope->scopeReturnType == type);

        if (returnType && *returnType == NULL)
            *returnType = type;
    }

    if (isEndScope)
    {
        r->scope = functionScope->parent;
    }
}

static void returnStatement(RESOLVER *r, const NT_NODE *node, const NT_TYPE **returnType)
{
    assert(node->type.class == NC_STMT && node->type.kind == NK_RETURN);
    endFunctionScope(r, node, returnType, false);
}

static void expressionStatement(RESOLVER *r, NT_NODE *node)
{
    assert(node->type.class == NC_STMT);
    assert(node->type.kind == NK_EXPR);
    evalExprType(r, node->left);
}

static void statement(RESOLVER *r, NT_NODE *node, const NT_TYPE **returnType)
{
    *returnType = NULL;
    if (node->type.class != NC_STMT)
    {
        errorAt(r, node, "Invalid node, the node must be a statment!");
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
    default: {
        const char *const label = ntGetKindLabel(node->type.kind);
        errorAt(r, node, "Invalid statment. The statement with kind '%s' is invalid.", label);
        break;
    }
    }
}

static void addWeakFunction(RESOLVER *r, const NT_STRING *name, NT_SYMBOL_TYPE symbolType,
                            const NT_DELEGATE_TYPE *delegateType, bool public)
{
    assert(r);
    assert(r->module);
    assert(name);
    assert(IS_VALID_OBJECT(name));
    assert(delegateType);
    assert(IS_VALID_TYPE(delegateType));
    assert(((symbolType & SYMBOL_TYPE_FUNCTION) == SYMBOL_TYPE_FUNCTION) ||
           ((symbolType & SYMBOL_TYPE_SUBROUTINE) == SYMBOL_TYPE_SUBROUTINE));

    if (r->functionScope->parent == &r->module->type.fields)
    {
        ntAddModuleWeakFunction(r->module, name, delegateType, public);
        return;
    }

    assert(public == false);
    ntAddModuleWeakFunction(r->module, name, delegateType, public);
    addWeakSymbol(r, name, symbolType, (const NT_TYPE *)delegateType, NULL);
}

static void declareWeakFunction(RESOLVER *r, NT_NODE *node, const bool returnValue)
{
    assert(r);

    const char_t *name = node->token.lexeme;
    const size_t nameLen = node->token.lexemeLength;

    const NT_SYMBOL_TYPE symbolType = returnValue ? SYMBOL_TYPE_FUNCTION : SYMBOL_TYPE_SUBROUTINE;
    bool hasReturn = false;

    beginScope(r, returnValue ? STT_FUNCTION : STT_METHOD);

    const NT_LIST params = node->data;
    const size_t paramCount = ntListLen(node->data);

    NT_ARRAY paramsArray;
    ntInitArray(&paramsArray);
    const NT_TYPE *returnType = NULL;

    if (returnValue)
    {
        if (node->left)
            returnType = findType(r, node->left);
        else
            returnType = evalBlockReturnType(r, node->right);
    }

    for (size_t i = 0; i < paramCount; ++i)
    {
        const NT_NODE *paramNode = ntListGet(params, i);
        NT_NODE *typeNode = paramNode->left;
        const NT_TYPE *type = findType(r, typeNode);
        const NT_STRING *paramName =
            ntCopyString(paramNode->token.lexeme, paramNode->token.lexemeLength);

        addWeakParam(r, paramName, type);

        const NT_PARAM param = {
            .name = paramName,
            .type = type,
        };
        ntArrayAdd(&paramsArray, &param, sizeof(NT_PARAM));
    }

    for (size_t i = 0; i < ntListLen(node->right->data) && !hasReturn; ++i)
    {
        NT_NODE *stmt = (NT_NODE *)ntListGet(node->right->data, i);
        const NT_TYPE *statmentReturn = NULL;
        statement(r, stmt, &statmentReturn);
        if (statmentReturn != NULL)
            hasReturn |= true;
    }

    const NT_DELEGATE_TYPE *delegateType =
        ntTakeDelegateType(r->assembly, returnType, paramCount, (NT_PARAM *)paramsArray.data);
    const NT_STRING *funcName = ntCopyString(name, nameLen);

    ntDeinitArray(&paramsArray);

    addWeakFunction(r, funcName, symbolType, delegateType, r->public);

    if (returnValue)
    {
        if (!hasReturn)
        {
            char *lname = ntToCharFixed(name, nameLen);
            errorAt(r, node, "Function '%s' doesn't  return a value on all code paths.", lname);
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

    NT_SYMBOL_TABLE *table = r->global;
    NT_MODULE *importedModule = NULL;
    assert(table);

    NT_SYMBOL_ENTRY entry;

    bool result = false;
    do
    {
        result = ntLookupSymbolCurrent(table, current->token.lexeme, current->token.lexemeLength,
                                       &entry);
        assert(result);

        if (!result)
        {
            errorAt(r, node, "Cannot resolver the import symbol");
            break;
        }

        if ((entry.type & SYMBOL_TYPE_MODULE) != SYMBOL_TYPE_MODULE)
        {
            errorAt(r, node, "Symbol must be a module");
            break;
        }

        importedModule = (NT_MODULE *)entry.data;

        assert(importedModule);
        assert(IS_VALID_OBJECT(importedModule));
        assert(((NT_OBJECT *)importedModule)->type->objectType == NT_OBJECT_TYPE_TYPE);
        assert(((NT_TYPE *)importedModule)->objectType == NT_OBJECT_MODULE);

        table = &importedModule->type.fields;
        current = current->right;
    } while (current != NULL);

    if (result)
        ntInsertSymbol(&r->module->type.fields, &entry);

    if (r->module != r->globalModule)
        warningAt(node, "Consider only use imports on file module");
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
        errorAt(r, node, "Expect a declaration");
        break;
    }
}

static void module(RESOLVER *r, NT_NODE *node)
{
    assert(r);
    assert(node);
    assert(node->type.class == NC_STMT && node->type.kind == NK_MODULE);

    NT_MODULE *const module = (NT_MODULE *)node->userdata;
    assert(module);
    assert(IS_VALID_OBJECT(module));
    assert(module->type.objectType == NT_OBJECT_MODULE);

    const bool savePublic = r->public;
    NT_MODULE *const saveModule = r->module;

    r->module = module;

    const size_t count = ntListLen(node->data);
    for (size_t i = 0; i < count; ++i)
    {
        NT_NODE *stmt = (NT_NODE *)ntListGet(node->data, i);

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

bool ntResolve(NT_ASSEMBLY *assembly, NT_SYMBOL_TABLE *globalTable, size_t moduleNodeCount,
               NT_NODE **moduleNodes)
{
    assert(assembly != NULL);
    assert(moduleNodeCount > 0);
    assert(moduleNodes != NULL);

    RESOLVER resolver = {
        .assembly = assembly,
        .module = NULL,
        .globalModule = NULL,
        .global = globalTable,
        .scope = globalTable,
        .functionScope = NULL,
        .public = false,
        .had_error = false,
    };

    for (size_t i = 0; i < moduleNodeCount; ++i)
    {
        NT_NODE *const current = moduleNodes[i];
        resolver.globalModule = (NT_MODULE *)current->userdata;
        module(&resolver, current);
    }

    return !resolver.had_error;
}
