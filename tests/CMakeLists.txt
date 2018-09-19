set(project primitive_test)

add_executable(${project} parser_test.cpp)

target_link_libraries(${project} compiler vm)