#include <assert.h>
#include <netuno/memory.h>
#include <netuno/module.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/varint.h>

static bool refEquals(NT_OBJECT *obj1, NT_OBJECT *obj2)
{
    assert(obj1);
    assert(IS_VALID_OBJECT(obj1));
    assert(obj1->type->objectType == NT_OBJECT_MODULE);

    assert(obj2);
    assert(IS_VALID_OBJECT(obj2));
    assert(obj2->type->objectType == NT_OBJECT_MODULE);

    return obj1 == obj2;
}

static void freeModule(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_MODULE);

    NT_MODULE *module = (NT_MODULE *)object;

    ntDeinitArray(&module->code);
    ntDeinitArray(&module->lines);
    ntDeinitArray(&module->constants);
}

static const NT_STRING *moduleToString(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_MODULE);

    NT_MODULE *module = (NT_MODULE *)object;

    const size_t length = module->object.type->typeName->length + 1 + module->name->length;
    char_t *name = (char_t *)ntMalloc((length + 1) * sizeof(char_t));
    name[length] = U'\0';

    // copy type name (Module)
    ntMemcpy(name, module->object.type->typeName->chars, module->object.type->typeName->length);

    // add space
    name[module->object.type->typeName->length] = U' ';

    // copy module name
    ntMemcpy(name + module->object.type->typeName->length + 1, module->name->chars,
             module->name->length);

    return ntTakeString(name, length);
}

static NT_TYPE MODULE_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_MODULE,
    .typeName = NULL,
    .free = freeModule,
    .string = moduleToString,
    .equals = refEquals,
    .stackSize = sizeof(NT_REF),
    .instanceSize = sizeof(NT_MODULE),
};

const NT_TYPE *ntModuleType(void)
{
    if (MODULE_TYPE.object.type == NULL)
        MODULE_TYPE.object.type = ntType();
    if (MODULE_TYPE.typeName == NULL)
        MODULE_TYPE.typeName = ntCopyString(U"Module", 3);
    return &MODULE_TYPE;
}

NT_MODULE *ntCreateModule(void)
{
    NT_MODULE *module = (NT_MODULE *)ntCreateObject(ntModuleType());

    ntInitArray(&module->code);
    ntInitArray(&module->lines);
    ntInitArray(&module->constants);
    ntInitSymbolTable(&module->fields, NULL, STT_MODULE, 0);

    return module;
}

static void addLine(NT_MODULE *module, const size_t offset, const size_t line)
{
    bool addLine = false;
    if (module->lines.count < sizeof(NT_LINE))
    {
        addLine = true;
    }
    else
    {
        NT_LINE l;
        ntArrayGet(&module->lines, module->lines.count - sizeof(NT_LINE), &l, sizeof(NT_LINE));
        if (l.line != line)
            addLine = true;
    }

    if (addLine)
    {
        NT_LINE l;
        l.start = offset;
        l.line = line;
        ntArrayAdd(&module->lines, &l, sizeof(NT_LINE));
    }
}

int64_t ntGetLine(const NT_MODULE *module, const size_t offset, bool *atStart)
{
    NT_LINE result = {.start = 0, .line = -1};
    for (size_t i = 0; i < module->lines.count; i += sizeof(NT_LINE))
    {
        NT_LINE current;
        ntArrayGet(&module->lines, i, &current, sizeof(NT_LINE));
        if (current.start > offset)
            break;
        result = current;
    }
    *atStart = (result.start == offset);
    return result.line;
}

size_t ntWriteModule(NT_MODULE *module, const uint8_t value, const int64_t line)
{
    size_t offset = ntArrayAdd(&module->code, &value, sizeof(uint8_t));
    addLine(module, offset, line);
    return offset;
}

void ntInsertModule(NT_MODULE *module, const size_t offset, const void *data, const size_t length)
{
    ntArrayInsert(&module->code, offset, data, length);
}

void ntInsertModuleVarint(NT_MODULE *module, const size_t offset, const uint64_t value)
{
    ntArrayInsertVarint(&module->code, offset, value);
}

size_t ntWriteModuleVarint(NT_MODULE *module, const uint64_t value, const int64_t line)
{
    size_t offset = ntArrayAddVarint(&module->code, value);
    addLine(module, offset, line);
    return offset;
}

uint8_t ntRead(const NT_MODULE *module, const size_t offset)
{
    uint8_t value;
    const bool result =
        ntArrayGet(&module->code, offset, &value, sizeof(uint8_t)) == sizeof(uint8_t);
    assert(result);
    return value;
}

size_t ntReadVariant(const NT_MODULE *module, const size_t offset, uint64_t *value)
{
    return ntArrayGetVarint(&module->code, offset, value);
}

bool ntModuleAddSymbol(NT_MODULE *module, const NT_STRING *name, NT_SYMBOL_TYPE symbolType,
                       const NT_TYPE *type, void *data)
{
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = symbolType,
        .data = data,
        .exprType = type,
    };
    return ntInsertSymbol(&module->fields, &entry);
}

uint64_t ntAddConstant32(NT_MODULE *module, const uint32_t value)
{
    size_t offset;
    if (ntArrayFind(&module->constants, &value, sizeof(value), &offset))
        return offset;

    ntArrayAddU32(&module->constants, value);
    return module->constants.count - sizeof(value);
}

uint64_t ntAddConstant64(NT_MODULE *module, const uint64_t value)
{
    size_t offset;
    if (ntArrayFind(&module->constants, &value, sizeof(value), &offset))
        return offset;

    ntArrayAddU64(&module->constants, value);
    return module->constants.count - sizeof(value);
}

uint64_t ntAddConstantString(NT_MODULE *module, const char_t *str, const size_t length)
{
    size_t offset;

    const size_t nullTerminatedSize = sizeof(char_t) * (length + 1);
    char_t *nullTerminatedStr = (char_t *)ntMalloc(nullTerminatedSize);
    ntMemcpy(nullTerminatedStr, str, length * sizeof(char_t));
    nullTerminatedStr[length] = '\0';

    if (ntArrayFind(&module->constants, str, nullTerminatedSize, &offset))
    {
        ntFree(nullTerminatedStr);
        return offset;
    }
    ntArrayAdd(&module->constants, nullTerminatedStr, nullTerminatedSize);
    ntFree(nullTerminatedStr);
    return module->constants.count - nullTerminatedSize;
}

const NT_DELEGATE *ntAddModuleFunction(NT_MODULE *module, const NT_STRING *name,
                                       const NT_DELEGATE_TYPE *delegateType, size_t pc, bool public)
{
    assert(module);
    assert(delegateType);
    assert(IS_VALID_TYPE(delegateType));
    assert(delegateType->type.objectType == NT_OBJECT_DELEGATE);

    const NT_DELEGATE *delegate = (const NT_DELEGATE *)ntDelegate(delegateType, module, pc, name);

    if (name)
    {
        const NT_SYMBOL_TYPE symbolType =
            (((const NT_DELEGATE_TYPE *)delegate->object.type)->returnType != NULL
                 ? SYMBOL_TYPE_FUNCTION
                 : SYMBOL_TYPE_SUBROUTINE) |
            (public ? SYMBOL_TYPE_PUBLIC : SYMBOL_TYPE_PRIVATE);
        ntModuleAddSymbol(module, name, symbolType, delegate->object.type, (void *)delegate);
    }

    return delegate;
}
