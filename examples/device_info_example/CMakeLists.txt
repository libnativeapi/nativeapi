cmake_minimum_required(VERSION 3.10)

project(device_info_example)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(device_info_example main.cpp)

# Link with the native API library
target_link_libraries(device_info_example nativeapi)

# Set include directories
target_include_directories(device_info_example PRIVATE ../../include)
