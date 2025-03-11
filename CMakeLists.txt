cmake_minimum_required(VERSION 3.10)
project(ArtTreeProject)

set(CMAKE_CXX_STANDARD 17)

# Add Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(googletest)

# Enable AddressSanitizer
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Link Google Test to the test executable
enable_testing()
add_executable(ArtTreeTest unittest/node_test.cpp)
target_link_libraries(ArtTreeTest gtest gtest_main)
add_test(NAME ArtTreeTest COMMAND ArtTreeTest)