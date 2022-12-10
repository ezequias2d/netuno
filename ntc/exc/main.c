#include <netuno/debug.h>
#include <netuno/ntc.h>
#include <netuno/nto.h>
#include <netuno/vm.h>

const char_t *str = (char_t *)(L"sub main()\n"
                               L"   if false\n"
                               L"       print int(2.0f)\n"
                               L"   else\n"
                               L"       print 1\n"
                               L"   next\n"
                               L"end\n");
int main()
{
    NT_ASSEMBLY *assembly = ntCreateAssembly();
    NT_CHUNK *chunk = ntCompile(assembly, str);
    // ntDisassembleChunk(chunk, "main");

    NT_VM *vm = ntCreateVM();
    ntRun(vm, chunk);
    ntFreeVM(vm);
    // NT_CHUNK *chunk = ntCreateChunk();

    // uint64_t contant = ntAddConstant32(chunk, 1);
    // ntWriteChunk(chunk, OP_CONST_32, 123);
    // ntWriteChunkVarint(chunk, contant, 123);
    // ntWriteChunk(chunk, OP_CONST_32, 123);
    // ntWriteChunkVarint(chunk, contant, 123);
    // // ntWriteChunk(chunk, OP_NEG_I32, 123);
    // ntWriteChunk(chunk, OP_ADD_I32, 123);
    // ntWriteChunk(chunk, OP_RETURN, 124);

    // NT_VM *vm = ntCreateVM();
    // ntRun(vm, chunk);
    // ntFreeVM(vm);
    // // ntDisassembleChunk(chunk, "test chunk");
    // ntFreeChunk(chunk);
    // // testNetuno();
    ntFreeAssembly(assembly);
    return 0;
}
