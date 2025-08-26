set(project hal)

set(${project}-sources byte_order.c)

add_library(${project} STATIC ${${project}-sources} byte_order.h  exceptions.h  hal.h  numeric_decl.h  opcode.h  registers.h  type_destination_decl.h)
