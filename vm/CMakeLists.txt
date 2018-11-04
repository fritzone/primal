include_directories()

set(project vm)

set(${project}-sources vm.cpp)

add_library(${project} STATIC
        ${${project}-sources}
)

target_link_libraries(${project} hal opcode-impl util)