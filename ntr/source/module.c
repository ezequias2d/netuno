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
    assert(object->type->objectType == NT_OBJECT_TYPE_TYPE);
    assert(((NT_TYPE *)object)->objectType == NT_OBJECT_MODULE);

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

    const size_t length = module->type.typeName->length + 1 + module->type.typeName->length;
    char_t *name = (char_t *)ntMalloc((length + 1) * sizeof(char_t));
    name[length] = U'\0';

    // copy type name (Module)
    const char_t *moduleKeyword = U"Module";
    const size_t moduleKeywordSize = ntStrLen(moduleKeyword);
    ntMemcpy(name, moduleKeyword, moduleKeywordSize);

    // add space
    name[moduleKeywordSize] = U' ';

    // copy module name
    ntMemcpy(name + moduleKeywordSize + 1, module->type.typeName->chars,
             module->type.typeName->length);

    return ntTakeString(name, length);
}

static NT_TYPE MODULE_TYPE = {
    .object =
        {
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_TYPE_TYPE,
    .typeName = NULL,
    .free = freeModule,
    .string = moduleToString,
    .equals = refEquals,
    .stackSize = sizeof(NT_REF),
    .instanceSize = sizeof(NT_MODULE),
    .baseType = NULL,
};

const NT_TYPE *ntModuleType(void)
{
    if (MODULE_TYPE.object.type == NULL)
    {
        MODULE_TYPE.object.type = ntType();
        MODULE_TYPE.typeName = ntCopyString(U"Module", 3);
        MODULE_TYPE.baseType = ntType();
        ntInitSymbolTable(&MODULE_TYPE.fields, (NT_SYMBOL_TABLE *)&ntType()->fields, STT_TYPE, 0);
    }

    return &MODULE_TYPE;
}

NT_MODULE *ntCreateModule(void)
{
    NT_MODULE *module = (NT_MODULE *)ntCreateObject(ntModuleType());
    ntInitModule(module);
    return module;
}

void ntInitModule(NT_MODULE *module)
{
    const NT_TYPE *type = ntModuleType();
    if (module->type.object.type != type)
    {
        module->type.object.type = type;
        module->type.object.refCount = 1;
    }

    module->type.objectType = NT_OBJECT_MODULE;
    module->type.typeName = NULL;
    module->type.free = NULL;
    module->type.string = NULL;
    module->type.equals = NULL;
    module->type.stackSize = 0;
    module->type.instanceSize = 0;
    module->type.baseType = NULL;
    ntInitSymbolTable(&module->type.fields, NULL, STT_TYPE, 0);

    ntInitArray(&module->code);
    ntInitArray(&module->lines);
    ntInitArray(&module->constants);
}

static void addLine(NT_MODULE *module, const size_t length, const size_t line)
{
    bool addLine = false;
    if (module->lines.count < sizeof(NT_LINE))
    {
        addLine = true;
    }
    else
    {
        NT_LINE l;
        const size_t offset = module->lines.count - sizeof(NT_LINE);
        ntArrayGet(&module->lines, offset, &l, sizeof(NT_LINE));
        if (l.line != line)
            addLine = true;
        else
        {
            l.length += length;
            ntArraySet(&module->lines, offset, &l, sizeof(NT_LINE));
        }
    }

    if (addLine)
    {
        NT_LINE l;
        l.length = length;
        l.line = line;
        ntArrayAdd(&module->lines, &l, sizeof(NT_LINE));
    }
}

static void insertLine(NT_MODULE *module, const size_t offset, const size_t length)
{
    const size_t count = module->lines.size / sizeof(NT_LINE);
    assert(count);

    size_t i = 0;
    NT_LINE line;
    ntArrayGet(&module->lines, 0, &line, sizeof(NT_LINE));
    size_t current = line.length;
    while (i + 1 < count && current < offset)
    {
        i++;
        ntArrayGet(&module->lines, i * sizeof(NT_LINE), &line, sizeof(NT_LINE));
        current += line.length;
    }

    line.length += length;
    ntArraySet(&module->lines, i * sizeof(NT_LINE), &line, sizeof(NT_LINE));
}

int64_t ntGetLine(const NT_MODULE *module, const size_t offset, bool *atStart)
{
    NT_LINE line = {.length = 0, .line = -1};
    size_t current = 0;
    for (size_t i = 0; i < module->lines.count; i += sizeof(NT_LINE))
    {
        ntArrayGet(&module->lines, i, &line, sizeof(NT_LINE));
        current += line.length;
        if (offset < current)
            break;
    }
    *atStart = (current - line.length == offset);
    return line.line;
}

size_t ntWriteModule(NT_MODULE *module, const uint8_t value, const int64_t line)
{
    size_t offset = ntArrayAdd(&module->code, &value, sizeof(uint8_t));
    addLine(module, sizeof(value), line);
    return offset;
}

void ntInsertModule(NT_MODULE *module, const size_t offset, const void *data, const size_t length)
{
    ntArrayInsert(&module->code, offset, data, length);
    insertLine(module, offset, length);
}

void ntInsertModuleVarint(NT_MODULE *module, const size_t offset, const uint64_t value)
{
    const size_t length = ntArrayInsertVarint(&module->code, offset, value);
    insertLine(module, offset, length);
}

size_t ntWriteModuleVarint(NT_MODULE *module, const uint64_t value, const int64_t line)
{
    size_t length = 0;
    const size_t offset = ntArrayAddVarint(&module->code, value, &length);
    assert(length);
    addLine(module, length, line);
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
        .weak = false,
    };
    return ntInsertSymbol(&module->type.fields, &entry);
}

bool ntModuleAddWeakSymbol(NT_MODULE *module, const NT_STRING *name, NT_SYMBOL_TYPE symbolType,
                           const NT_TYPE *type, void *data)
{
    const NT_SYMBOL_ENTRY entry = {
        .symbol_name = name,
        .type = symbolType,
        .data = data,
        .exprType = type,
        .weak = true,
    };
    return ntInsertSymbol(&module->type.fields, &entry);
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

void ntAddModuleWeakFunction(NT_MODULE *module, const NT_STRING *name,
                             const NT_DELEGATE_TYPE *delegateType, bool public)
{
    assert(module);
    assert(delegateType);
    assert(IS_VALID_TYPE(delegateType));
    assert(delegateType->type.objectType == NT_OBJECT_DELEGATE);

    const NT_DELEGATE *delegate =
        (const NT_DELEGATE *)ntDelegate(delegateType, module, SIZE_MAX, name);

    if (name)
    {
        const NT_SYMBOL_TYPE symbolType =
            (delegateType->returnType != NULL ? SYMBOL_TYPE_FUNCTION : SYMBOL_TYPE_SUBROUTINE) |
            (public ? SYMBOL_TYPE_PUBLIC : SYMBOL_TYPE_PRIVATE);
        ntModuleAddWeakSymbol(module, name, symbolType, (const NT_TYPE *)delegateType,
                              (void *)delegate);
    }
}

const NT_DELEGATE *ntAddModuleFunction(NT_MODULE *module, const NT_STRING *name,
                                       const NT_DELEGATE_TYPE *delegateType, size_t pc, bool public)
{
    assert(module);
    assert(delegateType);
    assert(IS_VALID_TYPE(delegateType));
    assert(delegateType->type.objectType == NT_OBJECT_DELEGATE);

    NT_DELEGATE *delegate = NULL;
    if (name)
    {
        NT_SYMBOL_ENTRY entry;
        if (ntLookupSymbolCurrent(&module->type.fields, name->chars, name->length, &entry) &&
            entry.weak)
        {
            delegate = (NT_DELEGATE *)entry.data;
            assert(delegate);
            assert(IS_VALID_OBJECT(delegate));
            assert(delegate->object.type->objectType == NT_OBJECT_DELEGATE);
        }
    }

    if (delegate == NULL)
        delegate = (NT_DELEGATE *)ntDelegate(delegateType, module, pc, name);
    else
    {
        delegate->addr = pc;
        delegate->sourceModule = module;
    }

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

const NT_DELEGATE *ntAddNativeModuleFunction(NT_MODULE *module, const NT_STRING *name,
                                             const NT_DELEGATE_TYPE *delegateType, nativeFun func,
                                             bool public)
{
    assert(module);
    assert(delegateType);
    assert(IS_VALID_TYPE(delegateType));
    assert(delegateType->type.objectType == NT_OBJECT_DELEGATE);

    const NT_DELEGATE *delegate = (const NT_DELEGATE *)ntNativeDelegate(delegateType, func, name);

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
