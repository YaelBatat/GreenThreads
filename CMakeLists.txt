cmake_minimum_required(VERSION 3.10)
project(GreenThreads)

# Use C++17 for modern features (std::unique_ptr, std::function)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compiler warnings to ensure high code quality
add_compile_options(-Wall -Wextra)

# Platform-specific definitions
if(APPLE)
    add_compile_definitions(__APPLE__)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(__linux__)
endif()

# Include directories
include_directories(include)

# Create the library
add_library(green_threads
        src/Thread.cpp
        src/Scheduler.cpp
        src/lib.cpp
)

# Create the demo application
add_executable(demo_app examples/main.cpp)
target_link_libraries(demo_app green_threads)