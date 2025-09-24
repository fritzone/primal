cmake_minimum_required(VERSION 3.10)
project(PrimalDebugger LANGUAGES CXX)

# Use C++17 (or newer if you like)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Add executable from main.cpp
add_executable(PrimalDebugger main.cpp)

# Link SDL2
target_link_libraries(PrimalDebugger PRIVATE SDL2::SDL2)
