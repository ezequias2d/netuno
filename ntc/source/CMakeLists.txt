set(ROOT_SOURCES
    "netuno.c"
    "parser.c"
    "scanner.c"
    "list.c"
    "nil_codegen.c"
    "resolver.c"
    "report.c"
    "type.c"
    "scope.c"
    "modules/console.c"
    "modules/string.c"
    "modules/numbers.c"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

source_group("" FILES ${ROOT_SOURCES})

set(DEBUG_LIB_SUFFIX "")

if(WIN32)
    set(DEBUG_LIB_SUFFIX "d")
endif(WIN32)

add_library(ntc SHARED ${ROOT_SOURCES})

target_include_directories(ntc PRIVATE
    ${NTC_ROOT_DIR}/source
)

target_include_directories(ntc PUBLIC
    ${NTC_ROOT_DIR}/include)

target_link_libraries(ntc PRIVATE nil nlib)

if(NOT WIN32)
    target_link_libraries(ntc
        PRIVATE dl
        PRIVATE pthread
    )
endif(NOT WIN32)
