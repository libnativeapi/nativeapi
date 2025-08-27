cmake_minimum_required(VERSION 3.10)

project(tray_example VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(tray_example main.cpp)

# Include the parent directory for headers
target_include_directories(tray_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src)

# Link with the main library
target_link_libraries(tray_example libnativeapi)