# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(nativeapi VERSION 0.0.1 LANGUAGES CXX)

# Enable Objective-C++
if(APPLE)
    enable_language(OBJCXX)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common source files
file(GLOB COMMON_SOURCES "*.cpp" "foundation/*.cpp")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "platform/*")

# C API source files
file(GLOB CAPI_SOURCES "capi/*.cpp" "capi/*.c")

# Platform-specific source files
if(ANDROID)
    file(GLOB PLATFORM_SOURCES "platform/android/*.cpp")
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    file(GLOB PLATFORM_SOURCES "platform/ios/*.mm")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB PLATFORM_SOURCES "platform/linux/*.cpp")
    # Find packages for Linux
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
    pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
    pkg_check_modules(XI REQUIRED IMPORTED_TARGET xi)
    pkg_check_modules(AYATANA_APPINDICATOR REQUIRED IMPORTED_TARGET ayatana-appindicator3-0.1)
elseif(APPLE)
    file(GLOB PLATFORM_SOURCES "platform/macos/*.mm")
elseif(CMAKE_SYSTEM_NAME STREQUAL "OHOS")
    file(GLOB PLATFORM_SOURCES "platform/ohos/*.cpp")
elseif(WIN32)
    file(GLOB PLATFORM_SOURCES "platform/windows/*.cpp")
else()
    set(PLATFORM_SOURCES "")
endif()

# Add library target
add_library(nativeapi STATIC
  ${COMMON_SOURCES}
  ${PLATFORM_SOURCES}
  ${CAPI_SOURCES}
)

# Set library properties
set_target_properties(nativeapi PROPERTIES
  PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/../include/**/*.h"
)

# Set library include directories
target_include_directories(nativeapi PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<INSTALL_INTERFACE:include>
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_include_directories(nativeapi PUBLIC ${GTK_INCLUDE_DIRS})
endif ()

# Link required frameworks and libraries
if(ANDROID)
    target_link_libraries(nativeapi PUBLIC log android)
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(nativeapi PUBLIC "-framework UIKit" "-framework Foundation" "-framework CoreGraphics")
    target_compile_options(nativeapi PRIVATE "-x" "objective-c++")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(nativeapi PUBLIC PkgConfig::GTK PkgConfig::X11 PkgConfig::XI PkgConfig::AYATANA_APPINDICATOR pthread)
elseif(APPLE)
    target_link_libraries(nativeapi PUBLIC "-framework Cocoa")
    target_compile_options(nativeapi PRIVATE "-x" "objective-c++")
elseif(CMAKE_SYSTEM_NAME STREQUAL "OHOS")
    target_link_libraries(nativeapi PUBLIC hilog_ndk.z)
elseif(WIN32)
    target_link_libraries(nativeapi PUBLIC user32 shell32 dwmapi gdiplus crypt32)
endif ()
