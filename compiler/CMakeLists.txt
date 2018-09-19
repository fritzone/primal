set(project compiler)

#
# Generic project source files
#
set(${project}-sources
        ${CMAKE_CURRENT_SOURCE_DIR}/ast.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/token.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sequence.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/label.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/options.cpp
)

set(${project}-headers
        ${CMAKE_CURRENT_SOURCE_DIR}/asm_compiler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ast.h
        ${CMAKE_CURRENT_SOURCE_DIR}/parser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/token.h
        ${CMAKE_CURRENT_SOURCE_DIR}/operators.h
        ${CMAKE_CURRENT_SOURCE_DIR}/lexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/util.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source.h
        ${CMAKE_CURRENT_SOURCE_DIR}/sequence.h
        ${CMAKE_CURRENT_SOURCE_DIR}/variable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/generate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/keywords.h
        ${CMAKE_CURRENT_SOURCE_DIR}/keyword_def.h
        ${CMAKE_CURRENT_SOURCE_DIR}/compiler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/label.h
        ${CMAKE_CURRENT_SOURCE_DIR}/options.h
)

#
# Keywords, extend here if necessary
#
set(keyword-sources
        ${CMAKE_CURRENT_SOURCE_DIR}/kw_let.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kw_if.cpp
)
set(keyword-headers
    ${CMAKE_CURRENT_SOURCE_DIR}/kw_let.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kw_if.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kw_asm.h
)

list(APPEND ${project}-sources ${keyword-sources})
list(APPEND ${project}-headers ${keyword-headers})

add_library(${project}
        ${${project}-sources}
        ${${project}-headers}
        kw_asm.cpp)

target_link_libraries(${project} opcode-compilers)