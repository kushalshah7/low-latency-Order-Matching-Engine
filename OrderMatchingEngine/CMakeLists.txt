cmake_minimum_required(VERSION 3.10)
project(OrderMatchingEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization (macOS compatible)
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra")

# Source files for the library
set(ENGINE_SOURCES
    OrderBook.cpp
    MatchingEngine.cpp
)

# Create static library
add_library(matching_engine STATIC ${ENGINE_SOURCES})

# Main demo executable
add_executable(demo main.cpp)
target_link_libraries(demo matching_engine)

# Test executable
add_executable(test_engine test_matching_engine.cpp)
target_link_libraries(test_engine matching_engine)

# Enable testing
enable_testing()
add_test(NAME MatchingEngineTests COMMAND test_engine)