set(ROOT_SOURCES
    "array.c"
    "memory.c"
    "varint.c"
    "str.c"
    "path.c"
    "table.c"
    "string.c"
    "trie.c"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

source_group("" FILES ${ROOT_SOURCES})

set(DEBUG_LIB_SUFFIX "")

if(WIN32)
    set(DEBUG_LIB_SUFFIX "d")
endif(WIN32)

add_library(nlib STATIC ${ROOT_SOURCES})

target_include_directories(nlib PRIVATE
    ${NLIB_ROOT_DIR}/source
)

target_include_directories(nlib PUBLIC
    ${NLIB_ROOT_DIR}/include)

target_link_libraries(nlib PRIVATE m)

if(NOT WIN32)
    target_link_libraries(nlib
        PRIVATE dl
        PRIVATE pthread
    )
endif(NOT WIN32)
