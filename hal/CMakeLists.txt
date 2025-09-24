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
    registers.cpp
)

add_library(${project} STATIC ${${project}-sources})

install(TARGETS ${project}
    EXPORT primal-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES
    type_destination_decl.h
    numeric_decl.h
    byte_order.h
    hal.h
    opcode.h
    registers.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/primal
)
