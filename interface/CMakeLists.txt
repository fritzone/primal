cmake_minimum_required(VERSION 3.10)
project(interface)

# Define the interface library. This tells CMake that the library is header-only
# and doesn't need to be compiled. The name "inteface" is used here.
add_library(interface INTERFACE)

# Specify the public include directory for the library.
# This makes headers in the "include" directory available to any target that
# links against this library.
target_include_directories(interface
    INTERFACE
        # When building this project, headers are found in the local "include" directory.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

        # When this project is installed, headers will be in the "include" directory
        # relative to the installation prefix.
        $<INSTALL_INTERFACE:include>
)

# --- Optional: Installation Rules ---
# This part is needed if you plan to install the library so it can be found
# by other CMake projects using find_package().

# Install the header files from the 'include/' directory into the destination
# 'include' directory (e.g., /usr/local/include).
install(
    DIRECTORY include/
    DESTINATION include
)
