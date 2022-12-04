# executable
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(ntc-bin main.c)
set_target_properties(ntc-bin PROPERTIES OUTPUT_NAME ntc)

target_link_libraries(ntc-bin PUBLIC ntc)
target_link_libraries(ntc-bin PUBLIC ntr)
