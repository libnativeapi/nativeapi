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
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific source files
if(LINUX)
  set(PLATFORM_SOURCES "screen_retriever_linux.cpp")
elseif(APPLE)
  set(PLATFORM_SOURCES "screen_retriever_macos.mm")
elseif(WIN32)
  set(PLATFORM_SOURCES "screen_retriever_windows.cpp")
else()
  set(PLATFORM_SOURCES "")
endif()

# Add library target
add_library(libnativeapi STATIC
  "screen_retriever.cpp"
  ${PLATFORM_SOURCES}
)

# Set library properties
set_target_properties(libnativeapi PROPERTIES
  PUBLIC_HEADER "libnativeapi.h;screen_retriever.h;screen_retriever_linux.h;screen_retriever_macos.h;screen_retriever_windows.h"
)

# Set library include directories
target_include_directories(libnativeapi PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# Link required frameworks (macOS only)
if(APPLE)
  target_link_libraries(libnativeapi PUBLIC "-framework Cocoa")
  target_compile_options(libnativeapi PRIVATE "-x" "objective-c++")
endif()
