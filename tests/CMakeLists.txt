set(project primal_test)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
include(CTest)
enable_testing()

# Fetch Catch2 if not installed
include(FetchContent)
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0  # specify a version
)
FetchContent_MakeAvailable(catch2)

# Add the test executable
add_executable(${project} tests.cpp)

# Link Catch2 to the test executable
target_link_libraries(${project} PRIVATE Catch2::Catch2WithMain compiler vm)

# Register the test with CTest
include(Catch)
catch_discover_tests(${project})

