#include "parser.h"
#include "list.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <stdarg.h>
#include <stdio.h>

static NT_NODE *block(NT_PARSER *parser, NT_TK_ID end, const bool returnValue);
static NT_NODE *statement(NT_PARSER *parser, const bool returnValue);
static NT_NODE *declaration(NT_PARSER *parser, const bool returnValue);

NT_PARSER *ntParserCreate(NT_SCANNER *scanner)
{
    NT_PARSER *parser = (NT_PARSER *)ntMalloc(sizeof(NT_PARSER));

    parser->scanner = scanner;
    parser->current = parser->previous =
        (NT_TOKEN){.type = TK_NONE, .lexeme = NULL, .lexemeLength = 0, .line = -1};
    parser->hadError = false;

    return parser;
}

void ntParserDestroy(NT_PARSER *parser)
{
    ntFree(parser);
}

static void vErrorAt(NT_TOKEN token, const char *message, va_list args)
{

    const char *atEnd = (token.type == TK_EOF) ? " at end" : " at ";
    char *lexeme = "";
    bool freeLexeme = false;
    if (token.type != TK_EOF)
    {
        lexeme = ntToCharFixed(token.lexeme, token.lexemeLength);
        freeLexeme = true;
    }

    printf("[line %d] Error%s%s: ", token.line, atEnd, lexeme);
    vprintf(message, args);
    printf("\n");

    if (freeLexeme)
        ntFree(lexeme);
    assert(false);
}

static void errorAt(NT_TOKEN token, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    vErrorAt(token, message, vl);
    va_end(vl);
}

static void vErrorAtCurret(NT_PARSER *parser, const char *message, va_list args)
{
    vErrorAt(parser->current, message, args);
}

static void errorAtCurret(NT_PARSER *parser, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    vErrorAtCurret(parser, message, vl);
    va_end(vl);
}

static void advance(NT_PARSER *parser)
{
    parser->previous = parser->current;

    while (true)
    {
        ntScanToken(parser->scanner, &parser->current);

        if (parser->current.type != TK_ERROR)
            break;

        char *lexeme = ntToCharFixed(parser->current.lexeme, parser->current.lexemeLength);
        errorAtCurret(parser, lexeme);
        ntFree(lexeme);
    }
}

static bool check(const NT_PARSER *parser, NT_TK_TYPE type)
{
    return parser->current.type == type;
}

static bool checkId(const NT_PARSER *parser, NT_TK_TYPE type, NT_TK_ID id)
{
    return parser->current.type == type && parser->current.id == id;
}

static void consume(NT_PARSER *parser, NT_TK_TYPE type, const char *message, ...)
{
    if (parser->current.type == type)
    {
        advance(parser);
        return;
    }

    va_list vl;
    va_start(vl, message);
    vErrorAtCurret(parser, message, vl);
    va_end(vl);
}

static void consumeId(NT_PARSER *parser, NT_TK_TYPE type, NT_TK_ID id, const char *message, ...)
{
    if (parser->current.type == type && parser->current.id == id)
    {
        advance(parser);
        return;
    }
    va_list vl;
    va_start(vl, message);
    vErrorAtCurret(parser, message, vl);
    va_end(vl);
}

static void consumeId2(NT_PARSER *parser, NT_TK_TYPE type1, NT_TK_ID id1, NT_TK_TYPE type2,
                       NT_TK_ID id2, const char *message, ...)
{
    if ((parser->current.type == type1 && parser->current.id == id1) ||
        (parser->current.type == type2 && parser->current.id == id2))
    {
        advance(parser);
        return;
    }
    va_list vl;
    va_start(vl, message);
    vErrorAtCurret(parser, message, vl);
    va_end(vl);
}

static bool match(NT_PARSER *parser, NT_TK_TYPE type)
{
    if (check(parser, type))
    {
        advance(parser);
        return true;
    }
    return false;
}

static bool matchId(NT_PARSER *parser, NT_TK_TYPE type, NT_TK_ID id)
{
    if (checkId(parser, type, id))
    {
        advance(parser);
        return true;
    }
    return false;
}

static NT_NODE *makeNode(NT_NODE_CLASS class, NT_NODE_KIND kind, NT_TOKEN token, NT_NODE *left,
                         NT_NODE *right)
{
    NT_NODE *node = (NT_NODE *)ntMalloc(sizeof(NT_NODE));
    *node = (NT_NODE){
        .type = {class, kind, LT_NONE},
        .token = token,
        .left = left,
        .right = right,
        .condition = NULL,
        .data = NULL,
    };
    return node;
}

void ntDestroyNode(NT_NODE *node)
{
    if (node->right)
        ntDestroyNode(node->right);
    if (node->left)
        ntDestroyNode(node->left);
    if (node->condition)
        ntDestroyNode(node->condition);
    if (node->data)
    {
        for (size_t i = 0; i < ntListLen(node->data); ++i)
            ntDestroyNode((NT_NODE *)ntListGet(node->data, i));
        ntFreeList(node->data);
    }
    ntFree(node);
}

static NT_NODE *makeBlock(const NT_TOKEN token, const NT_TOKEN end, NT_LIST statements)
{
    NT_NODE *node = makeNode(NC_STMT, NK_BLOCK, token, NULL, NULL);
    node->data = statements;
    node->token2 = end;
    return node;
}

static NT_NODE *makeIf(const NT_TOKEN token, NT_NODE *condition, NT_NODE *thenBranch,
                       NT_NODE *elseBranch)
{
    NT_NODE *node = makeNode(NC_STMT, NK_IF, token, thenBranch, elseBranch);
    node->condition = condition;
    return node;
}

static NT_NODE *makeWhile(const NT_TOKEN token, NT_NODE *condition, NT_NODE *body)
{
    NT_NODE *node = makeNode(NC_STMT, NK_WHILE, token, body, NULL);
    node->condition = condition;
    return node;
}

static NT_NODE *makeUntil(const NT_TOKEN token, NT_NODE *condition, NT_NODE *body)
{
    NT_NODE *node = makeNode(NC_STMT, NK_UNTIL, token, body, NULL);
    node->condition = condition;
    return node;
}

static NT_NODE *makeFunction(NT_NODE_KIND kind, NT_TOKEN name, NT_LIST parameters,
                             NT_NODE *returnType, NT_NODE *body)
{
    NT_NODE *node = makeNode(NC_STMT, kind, name, returnType, body);
    node->data = parameters;
    return node;
}

static NT_NODE *makeVar(NT_TOKEN name, NT_NODE *type, NT_NODE *initializer)
{
    return makeNode(NC_STMT, NK_VAR, name, type, initializer);
}

static NT_NODE *makeVariable(NT_TOKEN name)
{
    return makeNode(NC_EXPR, NK_VARIABLE, name, NULL, NULL);
}

static NT_NODE *makeAssign(NT_TOKEN equal, NT_NODE *variable, NT_NODE *value)
{
    return makeNode(NC_EXPR, NK_ASSIGN, equal, variable, value);
}

static NT_NODE *makeLogical(NT_TOKEN op, NT_NODE *left, NT_NODE *right)
{
    return makeNode(NC_EXPR, NK_LOGICAL, op, left, right);
}

static NT_NODE *makeBinary(NT_TOKEN op, NT_NODE *left, NT_NODE *right)
{
    return makeNode(NC_EXPR, NK_BINARY, op, left, right);
}

static NT_NODE *makeCall(NT_TOKEN token, NT_NODE *callee, NT_LIST arguments)
{
    NT_NODE *node = makeNode(NC_EXPR, NK_CALL, token, callee, NULL);
    node->data = arguments;
    return node;
}

static NT_NODE *makeLiteral(const NT_TOKEN token, NT_LITERAL_TYPE literalType)
{
    NT_NODE *node = makeNode(NC_EXPR, NK_LITERAL, token, NULL, NULL);
    node->type.literalType = literalType;
    return node;
}

static NT_NODE *makeLabel(const NT_TOKEN name)
{
    return makeNode(NC_STMT, NK_LABEL, name, NULL, NULL);
}

static NT_NODE *labelDeclaration(NT_PARSER *parser)
{
    const NT_TOKEN name = parser->previous;
    consumeId(parser, TK_KEYWORD, ':', "Expect a colon before a label identifier");

    return makeLabel(name);
}

static NT_NODE *expression(NT_PARSER *parser);

static NT_NODE *primary(NT_PARSER *parser)
{
    if (matchId(parser, TK_KEYWORD, KW_TRUE) || matchId(parser, TK_KEYWORD, KW_FALSE))
        return makeLiteral(parser->previous, LT_BOOL);

    if (matchId(parser, TK_KEYWORD, KW_NONE))
        return makeLiteral(parser->previous, LT_NONE);

    if (match(parser, TK_I32))
        return makeLiteral(parser->previous, LT_I32);
    if (match(parser, TK_I64))
        return makeLiteral(parser->previous, LT_I64);

    if (match(parser, TK_U32))
        return makeLiteral(parser->previous, LT_U32);
    if (match(parser, TK_U64))
        return makeLiteral(parser->previous, LT_U64);

    if (match(parser, TK_F32))
        return makeLiteral(parser->previous, LT_F32);
    if (match(parser, TK_F64))
        return makeLiteral(parser->previous, LT_F64);

    if (match(parser, TK_STRING))
    {
        NT_TOKEN token = parser->previous;
        // remove quotes
        // token.lexeme++;
        // token.lexemeLength -= 2;
        return makeLiteral(token, LT_STRING);
    }

    if (match(parser, TK_IDENT))
        return makeVariable(parser->previous);

    if (matchId(parser, TK_KEYWORD, '('))
    {
        NT_NODE *expr = expression(parser);
        consumeId(parser, TK_KEYWORD, ')', "Expect ')' after expression.");
        return expr;
    }

    if (match(parser, TK_KEYWORD))
        return makeVariable(parser->previous);

    errorAt(parser->current, "Expect expression.");
    return makeNode(NC_NONE, NK_NONE, parser->current, NULL, NULL);
}

static NT_NODE *finishCall(NT_PARSER *parser, NT_NODE *callee)
{
    NT_LIST arguments = ntCreateList();

    if (!checkId(parser, TK_KEYWORD, ')'))
    {
        do
        {
            if (ntListLen(arguments) >= 255)
                errorAtCurret(parser, "Can't have more than 255 arguments.");

            ntListAdd(arguments, expression(parser));
        } while (matchId(parser, TK_KEYWORD, ','));
    }

    consumeId(parser, TK_KEYWORD, ')', "Expect ')' after arguments.");
    return makeCall(parser->previous, callee, arguments);
}

static NT_NODE *call(NT_PARSER *parser)
{
    NT_NODE *expr = primary(parser);
    while (matchId(parser, TK_KEYWORD, '('))
    {
        expr = finishCall(parser, expr);
    }

    return expr;
}

static NT_NODE *unary(NT_PARSER *parser)
{
    if (matchId(parser, TK_KEYWORD, '-') || matchId(parser, TK_KEYWORD, '!') ||
        matchId(parser, TK_KEYWORD, '~'))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = unary(parser);
        return makeNode(NC_EXPR, NK_UNARY, op, NULL, right);
    }
    return call(parser);
}

static NT_NODE *factor(NT_PARSER *parser)
{
    NT_NODE *expr = unary(parser);

    while (matchId(parser, TK_KEYWORD, '/') || matchId(parser, TK_KEYWORD, '*') ||
           matchId(parser, TK_KEYWORD, '%'))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = unary(parser);
        expr = makeBinary(op, expr, right);
    }
    return expr;
}

static NT_NODE *term(NT_PARSER *parser)
{
    NT_NODE *expr = factor(parser);

    while (matchId(parser, TK_KEYWORD, '-') || matchId(parser, TK_KEYWORD, '+'))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = factor(parser);
        expr = makeBinary(op, expr, right);
    }
    return expr;
}

static NT_NODE *comparison(NT_PARSER *parser)
{
    NT_NODE *expr = term(parser);

    while (matchId(parser, TK_KEYWORD, '>') || matchId(parser, TK_KEYWORD, '<') ||
           matchId(parser, TK_KEYWORD, OP_GE) || matchId(parser, TK_KEYWORD, OP_LE))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = term(parser);
        expr = makeBinary(op, expr, right);
    }
    return expr;
}

static NT_NODE *equality(NT_PARSER *parser)
{
    NT_NODE *expr = comparison(parser);

    while (matchId(parser, TK_KEYWORD, OP_EQ) || matchId(parser, TK_KEYWORD, OP_NE))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = comparison(parser);
        expr = makeBinary(op, expr, right);
    }
    return expr;
}

static NT_NODE *bitwiseAnd(NT_PARSER *parser)
{
    NT_NODE *expr = equality(parser);

    while (matchId(parser, TK_KEYWORD, '^'))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = equality(parser);
        expr = makeBinary(op, expr, right);
    }
    return expr;
}

static NT_NODE *bitwiseXor(NT_PARSER *parser)
{
    NT_NODE *expr = bitwiseAnd(parser);

    while (matchId(parser, TK_KEYWORD, '^'))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = bitwiseAnd(parser);
        expr = makeBinary(op, expr, right);
    }
    return expr;
}

static NT_NODE *bitwiseOr(NT_PARSER *parser)
{
    NT_NODE *expr = bitwiseXor(parser);

    while (matchId(parser, TK_KEYWORD, '|'))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = bitwiseXor(parser);
        expr = makeBinary(op, expr, right);
    }
    return expr;
}

static NT_NODE *logicalAnd(NT_PARSER *parser)
{
    NT_NODE *expr = bitwiseOr(parser);

    while (matchId(parser, TK_KEYWORD, OP_LOGAND))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = bitwiseOr(parser);
        expr = makeLogical(op, expr, right);
    }
    return expr;
}

static NT_NODE *logicalOr(NT_PARSER *parser)
{
    NT_NODE *expr = logicalAnd(parser);

    while (matchId(parser, TK_KEYWORD, OP_LOGOR))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = logicalAnd(parser);
        expr = makeLogical(op, expr, right);
    }
    return expr;
}

static NT_NODE *assignment(NT_PARSER *parser)
{
    NT_NODE *expr = logicalOr(parser);

    if (matchId(parser, TK_KEYWORD, '='))
    {
        NT_TOKEN equal = parser->previous;
        NT_NODE *right = assignment(parser);

        if (expr->type.kind == NK_VARIABLE)
            return makeAssign(equal, expr, right);

        errorAt(equal, "Invalid assignment target.");
    }

    return expr;
}

static NT_NODE *expression(NT_PARSER *parser)
{
    return assignment(parser);
}

static NT_NODE *typeAnnotation(NT_PARSER *parser)
{
    if (!(matchId(parser, TK_KEYWORD, KW_I32) || matchId(parser, TK_KEYWORD, KW_I64) ||
          matchId(parser, TK_KEYWORD, KW_U32) || matchId(parser, TK_KEYWORD, KW_U64) ||
          matchId(parser, TK_KEYWORD, KW_F32) || matchId(parser, TK_KEYWORD, KW_F64)))
        consume(parser, TK_IDENT, "Expect a identifier as a type.");

    const NT_TOKEN type = parser->previous;
    return makeNode(NC_TYPE, NK_NONE, type, NULL, NULL);
}

static NT_NODE *parameter(NT_PARSER *parser)
{
    consume(parser, TK_IDENT, "Expect a identifier as parameter name.");
    const NT_TOKEN name = parser->previous;

    consumeId(parser, TK_KEYWORD, ':', "Expect a ':' and a parameter type.");
    NT_NODE *type = typeAnnotation(parser);

    return makeNode(NC_STMT, NK_PARAM, name, type, NULL);
}

static NT_NODE *returnStatement(NT_PARSER *parser, const bool returnValue)
{
    const NT_TOKEN token = parser->previous;

    NT_NODE *value = NULL;
    if (returnValue)
        value = expression(parser);

    return makeNode(NC_STMT, NK_RETURN, token, value, NULL);
}

static NT_NODE *functionDeclaration(NT_PARSER *parser, const bool returnValue)
{
    consume(parser, TK_IDENT, "Expect a identifier for function/method.");
    const NT_TOKEN name = parser->previous;

    consumeId(parser, TK_KEYWORD, '(', "Expect a '(' after function name.");
    NT_LIST parameters = ntCreateList();
    if (!checkId(parser, TK_KEYWORD, ')'))
    {
        do
        {
            if (ntListLen(parameters) >= 255)
                errorAtCurret(parser, "Can't have more than 255 parameters.");

            NT_NODE *param = parameter(parser);
            ntListAdd(parameters, param);
        } while (matchId(parser, TK_KEYWORD, ','));
    }
    consumeId(parser, TK_KEYWORD, ')', "Expect ')' after paramters.");

    NT_NODE *returnType = NULL;
    if (!returnValue)
    {
        if (matchId(parser, TK_KEYWORD, ':'))
            errorAtCurret(parser, "Expect a subroutine to have no return type.");
    }
    else
    {
        consumeId(parser, TK_KEYWORD, ':', "A function expect a explicit type as return.");
        returnType = typeAnnotation(parser);
    }

    NT_NODE *body;
    if (matchId(parser, TK_KEYWORD, KW_ARROW))
        body = expression(parser);
    else
        body = block(parser, KW_END, returnValue);

    if (returnValue)
        return makeFunction(NK_DEF, name, parameters, returnType, body);
    return makeFunction(NK_SUB, name, parameters, NULL, body);
}

static NT_NODE *variableDeclaration(NT_PARSER *parser)
{
    consume(parser, TK_IDENT, "Expect a identifier for variable.");
    const NT_TOKEN name = parser->previous;

    NT_NODE *type = NULL;
    NT_NODE *initializer = NULL;
    if (matchId(parser, TK_KEYWORD, ':'))
        type = typeAnnotation(parser);

    if (matchId(parser, TK_KEYWORD, '='))
        initializer = expression(parser);

    if (type == NULL && initializer == NULL)
    {
        errorAtCurret(parser, "The variable must has a type or initialize.");
        return NULL;
    }

    return makeVar(name, type, initializer);
}

static NT_NODE *typeOrModuleDeclaration(NT_PARSER *parser, NT_NODE_KIND kind)
{
    consume(parser, TK_IDENT, "Expect a identifier for the type.");
    const NT_TOKEN name = parser->previous;

    NT_LIST methods = ntCreateList();
    NT_LIST fields = ntCreateList();
    while (!checkId(parser, TK_KEYWORD, KW_END) && !ntIsAtEnd(parser->scanner))
    {
        if (check(parser, TK_IDENT))
        {
            const NT_TOKEN ident = parser->current;
            // constructor
            if (ntStrEqualsFixed(ident.lexeme, ident.lexemeLength, name.lexeme, name.lexemeLength))
                ntListAdd(methods, functionDeclaration(parser, false));
            else
                ntListAdd(fields, variableDeclaration(parser));
        }
        else if (matchId(parser, TK_KEYWORD, KW_DEF))
            ntListAdd(methods, functionDeclaration(parser, true));
        else if (matchId(parser, TK_KEYWORD, KW_SUB))
            ntListAdd(methods, functionDeclaration(parser, false));
    }

    return makeNode(NC_STMT, kind, name, fields, methods);
}

static NT_NODE *declaration(NT_PARSER *parser, const bool returnValue)
{
    if (matchId(parser, TK_KEYWORD, KW_TYPE))
        return typeOrModuleDeclaration(parser, NK_TYPE);
    if (matchId(parser, TK_KEYWORD, KW_MODULE))
        return typeOrModuleDeclaration(parser, NK_MODULE);
    if (matchId(parser, TK_KEYWORD, KW_DEF))
        return functionDeclaration(parser, true);
    if (matchId(parser, TK_KEYWORD, KW_SUB))
        return functionDeclaration(parser, false);
    if (matchId(parser, TK_KEYWORD, KW_VAR))
        return variableDeclaration(parser);
    if (match(parser, TK_LABEL))
        return labelDeclaration(parser);
    return statement(parser, returnValue);
}

// NT_NODE *ntRoot(NT_PARSER *parser, NT_LIST types, const bool returnValue)
// {
//     const NT_TOKEN token = parser->previous;
//     NT_LIST statements = ntCreateList();

//     while (!ntIsAtEnd(parser->scanner))
//         ntListAdd(statements, declaration(parser, returnValue));

//     return makeBlock(token, (NT_TOKEN){}, statements);
// }

static NT_NODE *block(NT_PARSER *parser, NT_TK_ID end, const bool returnValue)
{
    const NT_TOKEN token = parser->previous;
    NT_LIST statements = ntCreateList();

    while (!checkId(parser, TK_KEYWORD, end) && !ntIsAtEnd(parser->scanner))
        ntListAdd(statements, declaration(parser, returnValue));

    char *endLex = ntToChar(ntGetKeywordLexeme(end));
    consumeId(parser, TK_KEYWORD, end, "Expect '%s' after the code block.", endLex);
    ntFree(endLex);
    return makeBlock(token, parser->previous, statements);
}

static NT_NODE *block2(NT_PARSER *parser, NT_TK_ID end1, NT_TK_ID end2, const bool returnValue)
{
    const NT_TOKEN token = parser->previous;
    NT_LIST statements = ntCreateList();

    while (!checkId(parser, TK_KEYWORD, end1) && !checkId(parser, TK_KEYWORD, end2) &&
           !ntIsAtEnd(parser->scanner))
        ntListAdd(statements, declaration(parser, returnValue));

    char *end1Lex = ntToChar(ntGetKeywordLexeme(end1));
    char *end2Lex = ntToChar(ntGetKeywordLexeme(end2));
    consumeId2(parser, TK_KEYWORD, end1, TK_KEYWORD, end2, "Expect '%s%s%s' after the code block.",
               end1Lex, end2 != TK_ID_NONE ? "' '" : "", end2 != TK_ID_NONE ? end2Lex : "");

    ntFree(end1Lex);
    ntFree(end2Lex);
    return makeBlock(token, parser->previous, statements);
}

static NT_NODE *makeEqualExpression(NT_TOKEN mainToken, NT_TOKEN name, NT_NODE *expr)
{
    NT_NODE *variable = makeVariable(name);
    NT_NODE *comparison = makeBinary(
        (NT_TOKEN){
            .type = TK_KEYWORD,
            .line = mainToken.line,
            .id = OP_EQ,
        },
        variable, expr);
    return comparison;
}

static NT_NODE *makeIncrementStatement(NT_TOKEN mainToken, NT_TOKEN name, NT_NODE *expr)
{
    if (expr == NULL)
    {
        expr = makeLiteral(
            (NT_TOKEN){
                .type = TK_I32,
                .lexeme = U"1",
                .lexemeLength = 1,
            },
            LT_I32);
    }

    NT_NODE *variable = makeVariable(name);
    NT_NODE *sum = makeBinary(
        (NT_TOKEN){
            .type = TK_KEYWORD,
            .line = mainToken.line,
            .id = '+',
        },
        variable, expr);
    NT_NODE *assign = makeAssign(
        (NT_TOKEN){
            .type = TK_KEYWORD,
            .line = mainToken.line,
            .id = '=',
        },
        variable, sum);

    return makeNode(NC_STMT, NK_EXPR, mainToken, assign, NULL);
}

static NT_NODE *forStatement(NT_PARSER *parser, const bool returnValue)
{
    const NT_TOKEN token = parser->previous;

    consume(parser, TK_IDENT, "Expect a identifier to interate.");
    const NT_TOKEN name = parser->previous;

    consumeId(parser, TK_KEYWORD, '=', "Expect a initializer.");
    NT_NODE *initializer = expression(parser);

    consumeId(parser, TK_KEYWORD, KW_TO, "Expect a limitation.");
    NT_NODE *to = expression(parser);

    NT_NODE *step = NULL;
    if (matchId(parser, TK_KEYWORD, KW_STEP))
        step = expression(parser);

    NT_NODE *mainBody = block(parser, KW_NEXT, returnValue);
    NT_NODE *body = mainBody;

    NT_NODE **incBlock = (NT_NODE **)ntMalloc(sizeof(NT_NODE *) * 2);
    incBlock[0] = body;
    incBlock[1] = makeIncrementStatement(step->token, name, step);
    body = makeBlock(token, mainBody->token2, incBlock);

    // create loop
    NT_NODE *condition = makeEqualExpression(to->token, name, to);
    body = makeUntil(token, condition, body);

    // create var delcaration and initializer
    NT_NODE **declBlock = (NT_NODE **)ntMalloc(sizeof(NT_NODE *) * 2);
    declBlock[0] = makeVar(name, NULL, initializer);
    declBlock[1] = body;

    body = makeBlock(token, mainBody->token2, declBlock);
    return body;
}

static NT_NODE *ifStatement(NT_PARSER *parser, const bool returnValue)
{
    const NT_TOKEN token = parser->previous;
    NT_NODE *condition = expression(parser);

    NT_NODE *thenBranch = block2(parser, KW_NEXT, KW_ELSE, returnValue);
    NT_NODE *elseBranch = NULL;
    if (thenBranch->token2.id == KW_ELSE)
    {
        if (matchId(parser, TK_KEYWORD, KW_IF))
            elseBranch = ifStatement(parser, returnValue);
        else
            elseBranch = block(parser, KW_NEXT, returnValue);
    }

    return makeIf(token, condition, thenBranch, elseBranch);
}

static NT_NODE *printStatement(NT_PARSER *parser)
{
    const NT_TOKEN token = parser->previous;
    NT_NODE *expr = expression(parser);
    while (matchId(parser, TK_KEYWORD, ','))
    {
        NT_TOKEN op = parser->previous;
        NT_NODE *right = expression(parser);
        expr = makeBinary(op, expr, right);
    }
    return makeNode(NC_STMT, NK_PRINT, token, expr, NULL);
}

static NT_NODE *whileStatement(NT_PARSER *parser, const bool returnValue)
{
    const NT_TOKEN token = parser->previous;
    NT_NODE *condition = expression(parser);

    NT_NODE *body;
    if (matchId(parser, TK_KEYWORD, KW_ARROW))
        body = statement(parser, returnValue);
    else
        body = block(parser, KW_NEXT, returnValue);

    return makeWhile(token, condition, body);
}

static NT_NODE *untilStatement(NT_PARSER *parser, const bool returnValue)
{
    const NT_TOKEN token = parser->previous;
    NT_NODE *condition = expression(parser);

    NT_NODE *body = NULL;
    if (matchId(parser, TK_KEYWORD, '{'))
        body = block(parser, KW_NEXT, returnValue);
    else
        body = statement(parser, returnValue);

    return makeUntil(token, condition, body);
}

static NT_NODE *expressionStatement(NT_PARSER *parser)
{
    NT_NODE *expr = expression(parser);
    return makeNode(NC_STMT, NK_EXPR, expr->token, expr, NULL);
}

static NT_NODE *statement(NT_PARSER *parser, const bool returnValue)
{
    if (matchId(parser, TK_KEYWORD, KW_FOR))
        return forStatement(parser, returnValue);
    if (matchId(parser, TK_KEYWORD, KW_IF))
        return ifStatement(parser, returnValue);
    if (matchId(parser, TK_KEYWORD, KW_PRINT))
        return printStatement(parser);
    if (matchId(parser, TK_KEYWORD, KW_RETURN))
        return returnStatement(parser, returnValue);
    if (matchId(parser, TK_KEYWORD, KW_WHILE))
        return whileStatement(parser, returnValue);
    if (matchId(parser, TK_KEYWORD, KW_UNTIL))
        return untilStatement(parser, returnValue);
    if (matchId(parser, TK_KEYWORD, KW_DO))
        return block(parser, KW_NEXT, returnValue);

    return expressionStatement(parser);
}

NT_NODE **ntParse(NT_PARSER *parser, uint32_t *pCount)
{
    advance(parser);

    uint32_t size = 64;
    uint32_t count = 0;
    NT_NODE **statements = (NT_NODE **)ntMalloc(sizeof(NT_NODE *) * size);

    while (!ntIsAtEnd(parser->scanner))
    {
        NT_NODE *decl = declaration(parser, false);
        if (count >= size)
        {
            size = size * 3 / 2;
            statements = (NT_NODE **)ntRealloc(statements, size * sizeof(NT_NODE *));
        }
        statements[count++] = decl;
    }

    statements = (NT_NODE **)ntRealloc(statements, count * sizeof(NT_NODE *));
    *pCount = count;

    return statements;
}

static const char *const kinds[] = {
#define kind(a) #a,
#include "kind.inc"
#undef kind
};

static const char *const classes[] = {
#define class(a) #a,
#include "class.inc"
#undef class
};

static const char *const literals[] = {
#define literal(a) #a,
#include "literal.inc"
#undef literal
};

const char *ntGetKindLabel(NT_NODE_KIND kind)
{
    assert(kind >= 0 && kind < NK_LAST);
    return kinds[kind];
}

const char *ntGetClassLabel(NT_NODE_CLASS class)
{
    assert(class >= 0 && class < NC_LAST);
    return classes[class];
}

const char *ntGetLiteralTypeLabel(NT_LITERAL_TYPE type)
{
    assert(type >= 0 && type < LT_LAST);
    return literals[type];
}

static void printNode(uint32_t depth, NT_NODE *node)
{
    printf("\n");
    for (uint32_t i = 0; i < depth; ++i)
        printf("  ");

    if (node->type.kind == NK_BLOCK)
    {
        printf("{");
        for (uint32_t i = 0; i < ntListLen(node->data); ++i)
        {
            NT_NODE *stmt = (NT_NODE *)ntListGet(node->data, i);
            if (stmt != NULL)
                printNode(depth + 1, stmt);
            else
            {
                for (uint32_t i = 0; i < depth + 1; ++i)
                    printf("  ");
                printf("\nNULL");
            }
        }

        printf("\n");
        for (uint32_t i = 0; i < depth; ++i)
            printf("  ");
        printf("}");
    }
    else
    {
        printf("[");
        bool hasChild = false;
        if (node->type.literalType != LT_NONE)
        {
            char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
            printf("%s[%s]", str, literals[node->type.literalType]);
            ntFree(str);
        }
        else
        {
            printf("%s ", kinds[node->type.kind]);

            if (node->token.type == TK_IDENT || node->type.kind == NK_BINARY ||
                node->type.kind == NK_LOGICAL || node->type.kind == NK_UNARY)
            {
                char *str = ntToCharFixed(node->token.lexeme, node->token.lexemeLength);
                printf("%s", str);
                ntFree(str);
            }

            if (node->condition != NULL)
                printNode(depth + 1, node->condition);

            if (node->left != NULL)
                printNode(depth + 1, node->left);

            if (node->right != NULL)
                printNode(depth + 1, node->right);

            hasChild = node->condition != NULL || node->left != NULL || node->right != NULL;
        }

        if (hasChild)
        {
            printf("\n");
            for (uint32_t i = 0; i < depth; ++i)
                printf("  ");
        }
        printf("]");
    }
}

void ntPrintNode(uint32_t depth, NT_NODE *node)
{
    printNode(depth, node);
    printf("\n");
}
