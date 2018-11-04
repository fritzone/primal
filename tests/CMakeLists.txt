set(project primal_test)

add_executable(${project} tests.cpp catch_main.cpp)

target_link_libraries(${project} util compiler vm )
