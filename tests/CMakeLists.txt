set(project primitive_test)

add_executable(${project} parser_test.cpp catch_main.cpp)

target_link_libraries(${project} compiler vm)