# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(libnativeapi VERSION 0.0.1 LANGUAGES CXX)

# Enable Objective-C++
if(APPLE)
    enable_language(OBJCXX)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common source files
file(GLOB COMMON_SOURCES "*.cpp")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "platform/*")

# Platform-specific source files
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB PLATFORM_SOURCES "platform/linux/*_linux.cpp")
    # Find packages for Linux
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
    pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
    pkg_check_modules(XI REQUIRED IMPORTED_TARGET xi)
elseif(APPLE)
    file(GLOB PLATFORM_SOURCES "platform/macos/*_macos.mm")
elseif(WIN32)
    file(GLOB PLATFORM_SOURCES "platform/windows/*_windows.cpp")
else()
    set(PLATFORM_SOURCES "")
endif()

# Add library target
add_library(libnativeapi STATIC
  ${COMMON_SOURCES}
  ${PLATFORM_SOURCES}
)

# Set library properties
set_target_properties(libnativeapi PROPERTIES
  PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/../include/**/*.h"
)

# Set library include directories
target_include_directories(libnativeapi PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<INSTALL_INTERFACE:include>
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_include_directories(libnativeapi PUBLIC ${GTK_INCLUDE_DIRS})
endif ()

# Link required frameworks and libraries
if(APPLE)
    target_link_libraries(libnativeapi PUBLIC "-framework Cocoa")
    target_compile_options(libnativeapi PRIVATE "-x" "objective-c++")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(libnativeapi PUBLIC PkgConfig::GTK PkgConfig::X11 PkgConfig::XI pthread)
endif ()
