set(project hal)

set(${project}-sources
    byte_order.c
    byte_order.h
    exceptions.h
    hal.h
    numeric_decl.h
    opcode.h
    registers.h
    type_destination_decl.h
    type_destination.cpp
)

add_library(${project} STATIC ${${project}-sources} )
