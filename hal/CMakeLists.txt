set(project hal)

set(${project}-sources byte_order.c)

add_library(${project} STATIC ${${project}-sources} exceptions.h)
