set(ROOT_SOURCES
    "constant.c"
    "context.c"
    "value.c"
    "basic_block.c"
    "instruction.c"
    "list.c"
    "module.c"
    "type.c"
    "argument.c"
    "function.c"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

source_group("" FILES ${ROOT_SOURCES} ${NIL_SOURCES})

set(DEBUG_LIB_SUFFIX "")

if(WIN32)
    set(DEBUG_LIB_SUFFIX "d")
endif(WIN32)

add_library(nil STATIC ${ROOT_SOURCES} ${NIL_SOURCES})

target_include_directories(nil PRIVATE
    ${NIL_ROOT_DIR}/source
)

target_include_directories(nil PUBLIC
    ${NIL_ROOT_DIR}/include)

target_link_libraries(nil PUBLIC nlib)

if(NOT WIN32)
    target_link_libraries(nil
        PRIVATE dl
        PRIVATE pthread
    )
endif(NOT WIN32)
