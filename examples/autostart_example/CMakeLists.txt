cmake_minimum_required(VERSION 3.10)

project(autostart_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(autostart_example main.cpp)

# Link nativeapi library
target_link_libraries(autostart_example PRIVATE nativeapi)
