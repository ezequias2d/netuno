set(ROOT_SOURCES
    "array.c"
    "memory.c"
    "varint.c"
    "str.c"
    "path.c"
    "table.c"
    "string.c"
)

set(NIR_SOURCES
    "nir/constant.c"
    "nir/context.c"
    "nir/value.c"
    "nir/basic_block.c"
    "nir/instruction.c"
    "nir/list.c"
    "nir/module.c"
    "nir/type.c"
    "nir/argument.c"
    "nir/function.c"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

source_group("" FILES ${ROOT_SOURCES} ${NIR_SOURCES})

set(DEBUG_LIB_SUFFIX "")

if(WIN32)
    set(DEBUG_LIB_SUFFIX "d")
endif(WIN32)

add_library(nir SHARED ${ROOT_SOURCES} ${NIR_SOURCES})

target_include_directories(nir PRIVATE
    ${NIR_ROOT_DIR}/source
)

target_include_directories(nir PUBLIC
    ${NIR_ROOT_DIR}/include)

target_link_libraries(nir PRIVATE m)

if(NOT WIN32)
    target_link_libraries(nir
        PRIVATE dl
        PRIVATE pthread
    )
endif(NOT WIN32)
