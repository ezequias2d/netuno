#include <assert.h>
#include <netuno/gc.h>
#include <netuno/memory.h>

NT_GC *ntCreateGarbageCollector(void)
{
    NT_GC *gc = (NT_GC *)ntMalloc(sizeof(NT_GC));
    ntInitArray(&gc->objects);
    return gc;
}

void ntFreeGarbageCollector(NT_GC *gc)
{
    for (size_t i = 0; i < gc->objects.count / sizeof(NT_OBJECT *); ++i)
    {
        NT_OBJECT *ptr;
        const size_t size =
            ntArrayGet(&gc->objects, sizeof(NT_OBJECT *) * i, &ptr, sizeof(NT_OBJECT *));
        assert(size);
        ptr->refCount = 1;
        ntFreeObject(ptr);
    }

    ntDeinitArray(&gc->objects);
    ntFree(gc);
}

static int64_t ntFindObject(const NT_GC *gc, const NT_OBJECT *object)
{
    NT_OBJECT *ptr = NULL;
    for (size_t offset = 0; offset < gc->objects.count;)
    {
        const size_t inc = ntArrayGet(&gc->objects, offset, &ptr, sizeof(NT_OBJECT *));
        if (ptr == object)
            return offset / sizeof(NT_OBJECT *);
        offset += inc;
    }
    return -1;
}

uint32_t ntAddObject(NT_GC *gc, NT_OBJECT *object)
{
    int64_t find = ntFindObject(gc, object);
    if (find != -1)
        return (uint32_t)find;

    object->refCount = 1;
    const size_t offset = ntArrayAdd(&gc->objects, &object, sizeof(NT_OBJECT *));
    return offset / sizeof(NT_OBJECT *);
}

NT_OBJECT *ntGetObject(const NT_GC *gc, uint32_t id)
{
    NT_OBJECT *ptr;
    ntArrayGet(&gc->objects, sizeof(NT_OBJECT *) * id, &ptr, sizeof(NT_OBJECT *));
    return ptr;
}

void ntRefObject(NT_GC *gc, uint32_t id)
{
    NT_OBJECT *obj = ntGetObject(gc, id);
    obj->refCount++;
}

void ntUnrefObject(NT_GC *gc, uint32_t id)
{
    NT_OBJECT *obj = ntGetObject(gc, id);
    ntFreeObject(obj);
}
