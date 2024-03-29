set(ROOT_SOURCES
    "array.c"
    "varint.c"
    "vm.c"
    "debug.c"
    "symbol.c"
    "object.c"
    "string.c"
    "str.c"
    "table.c"
    "memory.c"
    "delegate.c"
    "assembly.c"
    "module.c"
    "native.c"
    "console.c"
    "path.c"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

source_group("" FILES ${ROOT_SOURCES})

set(DEBUG_LIB_SUFFIX "")

if(WIN32)
    set(DEBUG_LIB_SUFFIX "d")
endif(WIN32)

add_library(ntr SHARED ${ROOT_SOURCES})

target_include_directories(ntr PRIVATE
    ${NTR_ROOT_DIR}/source
)

target_include_directories(ntr PUBLIC
    ${NTR_ROOT_DIR}/include)

target_link_libraries(ntr PRIVATE m)

if(NOT WIN32)
    target_link_libraries(ntr
        PRIVATE dl
        PRIVATE pthread
    )
endif(NOT WIN32)
