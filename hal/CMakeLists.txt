set(project hal)

set(${project}-sources byte_order.c)

add_library(${project} ${${project}-sources} )
