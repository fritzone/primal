set(project primitive_test)

add_executable(${project} tests.cpp catch_main.cpp)

target_link_libraries(${project} compiler vm)