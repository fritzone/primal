set(project primdis)

# Executable definition
add_executable(${project} main.cpp)

# Link against the necessary primal libraries
target_link_libraries(${project} PRIVATE hal util compiler opcode-impl)

# Add include directories so it can find the primal headers
target_include_directories(${project} PRIVATE
    ${CMAKE_SOURCE_DIR}/hal
    ${CMAKE_SOURCE_DIR}/compiler
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_BINARY_DIR}/opcodes
)

install(TARGETS ${project} DESTINATION ${CMAKE_INSTALL_BINDIR})
