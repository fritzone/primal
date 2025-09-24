cmake_minimum_required(VERSION 3.16)
project(primal_vm_static LANGUAGES CXX)

# Paths to your already-built static libs (adjust paths!)
set(LIB_VM         "${CMAKE_BINARY_DIR}/lib/libvm.a")
set(LIB_COMPILER   "${CMAKE_BINARY_DIR}/lib/libcompiler.a")
set(LIB_OPCODE     "${CMAKE_BINARY_DIR}/lib/libopcode-impl.a")
set(LIB_UTIL       "${CMAKE_BINARY_DIR}/lib/libutil.a")
set(LIB_HAL        "${CMAKE_BINARY_DIR}/lib/libhal.a")

# Output fat lib
set(PRIMAL_LIB "${CMAKE_BINARY_DIR}/libprimal.a")

# Rule to build fat lib using ar + MRI script
add_custom_command(
    OUTPUT ${PRIMAL_LIB}
    COMMAND ${CMAKE_COMMAND} -E echo "create ${PRIMAL_LIB}" > merge.mri
    COMMAND ${CMAKE_COMMAND} -E echo "addlib ${LIB_VM}" >> merge.mri
    COMMAND ${CMAKE_COMMAND} -E echo "addlib ${LIB_COMPILER}" >> merge.mri
    COMMAND ${CMAKE_COMMAND} -E echo "addlib ${LIB_OPCODE}" >> merge.mri
    COMMAND ${CMAKE_COMMAND} -E echo "addlib ${LIB_UTIL}" >> merge.mri
    COMMAND ${CMAKE_COMMAND} -E echo "addlib ${LIB_HAL}" >> merge.mri
    COMMAND ${CMAKE_COMMAND} -E echo "save" >> merge.mri
    COMMAND ${CMAKE_COMMAND} -E echo "end" >> merge.mri
    COMMAND ${CMAKE_AR} -M < merge.mri
    COMMAND ${CMAKE_COMMAND} -E rm merge.mri
    DEPENDS ${LIB_VM} ${LIB_COMPILER} ${LIB_OPCODE} ${LIB_UTIL} ${LIB_HAL} ${LIB_INTERFACE}
    VERBATIM
)

# Define a phony target for convenience
add_custom_target(primal ALL DEPENDS ${PRIMAL_LIB})

install(FILES ${PRIMAL_LIB}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
