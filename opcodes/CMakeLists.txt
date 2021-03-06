set(project opcodes)
# cmake_policy(SET CMP0053 OLD)

set(registered_opcodes "")
string(TIMESTAMP now)

function(register_opcode opcode bincode pc fam)

    set(OPCODE ${opcode})
    set(OPCODE_CODE ${bincode})
    set(PARAMCOUNT ${pc})
    set(FAMILY ${fam})
    string(TIMESTAMP now)

    configure_file("opcode_decl.template"
                   "${CMAKE_CURRENT_BINARY_DIR}/${opcode}.h"
                   @ONLY
            )

    configure_file("opcode_compile.template"
                   "${CMAKE_CURRENT_BINARY_DIR}/compile_${opcode}.cpp"
                   @ONLY)

    set(registered_opcodes "${registered_opcodes};${opcode}" PARENT_SCOPE)
endfunction()

########################################################################################################################
# Register all the opcodes and the corresponding binary value the keywords, add yours and extend here if necessary.    #
#                                                                                                                      #
#              Register the keyword by using: register_opcode(NEW_OPCODE BINARY_VALUE PARAMCOUNT)                      #
#                                                                                                                      #
#       Do not forget to provide the implementation in the impl directory to perform the required operations.          #
#                                                                                                                      #
########################################################################################################################

register_opcode("MOV" 0x40 2 OF_ARITH)
register_opcode("ADD" 0x41 2 OF_ARITH)
register_opcode("SUB" 0x42 2 OF_ARITH)
register_opcode("MUL" 0x43 2 OF_ARITH)
register_opcode("DIV" 0x44 2 OF_ARITH)
register_opcode("MOD" 0x45 2 OF_ARITH)
register_opcode("OR" 0x46 2 OF_BIN)
register_opcode("AND" 0x47 2 OF_BIN)
register_opcode("EQ" 0x48 2 OF_COMP)
register_opcode("NEQ" 0x49 2 OF_COMP)
register_opcode("JT" 0x4A 1 OF_JUMP)
register_opcode("JNT" 0x4B 1 OF_JUMP)
register_opcode("JMP" 0x4C 1 OF_JUMP)
register_opcode("LT" 0x4D 2 OF_COMP)
register_opcode("GT" 0x4E 2 OF_COMP)
register_opcode("LTE" 0x4F 2 OF_COMP)
register_opcode("GTE" 0x50 2 OF_COMP)
register_opcode("PUSH" 0x51 1 OF_STACK)
register_opcode("POP" 0x52 1 OF_STACK)
register_opcode("COPY" 0x53 3 OF_MEM)
register_opcode("DJT" 0x54 2 OF_JUMP)
register_opcode("DJNT" 0x55 2 OF_JUMP)
register_opcode("DJMP" 0x56 2 OF_JUMP)
register_opcode("XOR" 0x57 2 OF_BIN)
register_opcode("NOT" 0x58 1 OF_BIN)
register_opcode("CALL" 0x59 1 OF_JUMP)
register_opcode("RET" 0x60 0 OF_JUMP)
register_opcode("INTR" 0x61 1 OF_JUMP)

########################################################################################################################
#                 Done, no more opcodes have to be added after this point in the code                                  #
########################################################################################################################

# Now create the "opcodes.h"
set(OPCH "${CMAKE_CURRENT_BINARY_DIR}/opcodes.h")
file(WRITE ${OPCH} "// Autogenerated by CMake on ${now}. All modifications to this file will be lost\n")
file(APPEND ${OPCH} "#ifndef OPCODES_H_INCLUDED\n")
file(APPEND ${OPCH} "#define OPCODES_H_INCLUDED\n")
foreach(opcode ${registered_opcodes})
    file(APPEND ${OPCH} "#include \"${opcode}.h\"\n")
endforeach()
file(APPEND ${OPCH} "\nnamespace primal {\n")
file(APPEND ${OPCH} "\nvoid register_opcodes();\n")
file(APPEND ${OPCH} "\nvoid register_opcode_compilers();\n")
file(APPEND ${OPCH} "\n}\n")

file(APPEND ${OPCH} "#endif\n")

# Firstly create the libraryy for the opcode implementers
set(opcode-impl-sources "")
set(OPIC "${CMAKE_CURRENT_BINARY_DIR}/opcode-impl.cpp")
file(WRITE ${OPIC} "// Autogenerated by CMake on ${now}. All modifications to this file will be lost\n")
file(APPEND ${OPIC} "#include \"opcodes.h\"\n")
file(APPEND ${OPIC} "#include <vm_impl.h>\n\n")
file(APPEND ${OPIC} "namespace primal {\n")
file(APPEND ${OPIC} "void register_opcodes() {\n")
foreach(opcode ${registered_opcodes})
    set(VOPCODE ${opcode})
    string(CONFIGURE "\tvm_impl::register_opcode(primal::opcodes::${VOPCODE}(), [&](primal::vm* v) -> bool { return primal::impl_${VOPCODE}(v); });" conf_line @ONLY)
    file(APPEND ${OPIC} "${conf_line}\n")

    # and here add the cpp file to the project sources
    list(APPEND opcode-impl-sources ${CMAKE_CURRENT_SOURCE_DIR}/impl/impl_${opcode}.cpp)
endforeach()
file(APPEND ${OPIC} "}\n}\n")

add_library(opcode-impl ${opcode-impl-sources} ${OPIC})
TARGET_LINK_LIBRARIES(opcode-impl util)

set(opcode-compiler-sources "")

# And the opcode compilers
set(OPCC "${CMAKE_CURRENT_BINARY_DIR}/opcode-compl.cpp")
file(WRITE ${OPCC} "// Autogenerated by CMake on ${now}. All modifications to this file will be lost\n")
file(APPEND ${OPCC} "#include <asm_compiler.h>\n")
file(APPEND ${OPCC} "#include <opcodes.h>\n")
file(APPEND ${OPCC} "namespace primal { \n void register_opcode_compilers() {\n")
foreach(opcode ${registered_opcodes})
    set(VOPCODE ${opcode})
    string(CONFIGURE "\tprimal::opcode_compiler_store::instance().register_opcode(primal::opcodes::${VOPCODE}(), [&](std::vector<primal::token>& t, compiler* c) -> std::vector<uint8_t> { return primal::compile_${VOPCODE}(t,c); });" conf_line @ONLY)
    file(APPEND ${OPCC} "${conf_line}\n")
    # and here add the cpp file to the project sources
    list(APPEND opcode-compiler-sources ${CMAKE_CURRENT_BINARY_DIR}/compile_${opcode}.cpp)
endforeach()
file(APPEND ${OPCC} "}\n\n }\n")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(${project}-sources ${OPCH} ${OPCC} ${opcode-sources})
add_library(opcode-compilers ${opcode-compiler-sources} ${OPCC})

target_link_libraries(opcode-compilers compiler util)
