cmake_minimum_required(VERSION 3.10)

project(autostart_c_example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add executable
add_executable(autostart_c_example main.c)

# Link nativeapi library
target_link_libraries(autostart_c_example PRIVATE nativeapi)
