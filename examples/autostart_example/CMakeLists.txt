cmake_minimum_required(VERSION 3.10)

project(autostart_example)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(autostart_example main.cpp)

# Link with the native API library
target_link_libraries(autostart_example nativeapi)

# Set include directories
target_include_directories(autostart_example PRIVATE ../../include)
