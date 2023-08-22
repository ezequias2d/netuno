set(ROOT_SOURCES
    "array.c"
    "memory.c"
    "varint.c"
    "str.c"
    "path.c"
    "table.c"
    "string.c"
)

set(NIL_SOURCES
    "nil/constant.c"
    "nil/context.c"
    "nil/value.c"
    "nil/basic_block.c"
    "nil/instruction.c"
    "nil/list.c"
    "nil/module.c"
    "nil/type.c"
    "nil/argument.c"
    "nil/function.c"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

source_group("" FILES ${ROOT_SOURCES} ${NIL_SOURCES})

set(DEBUG_LIB_SUFFIX "")

if(WIN32)
    set(DEBUG_LIB_SUFFIX "d")
endif(WIN32)

add_library(nil SHARED ${ROOT_SOURCES} ${NIL_SOURCES})

target_include_directories(nil PRIVATE
    ${NIL_ROOT_DIR}/source
)

target_include_directories(nil PUBLIC
    ${NIL_ROOT_DIR}/include)

target_link_libraries(nil PRIVATE m)

if(NOT WIN32)
    target_link_libraries(nil
        PRIVATE dl
        PRIVATE pthread
    )
endif(NOT WIN32)
