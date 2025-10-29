cmake_minimum_required(VERSION 3.10)

project(storage_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(storage_example main.cpp)

# Link nativeapi library
target_link_libraries(storage_example PRIVATE nativeapi)

# Platform-specific settings
if(APPLE)
    target_link_libraries(storage_example PRIVATE "-framework Cocoa" "-framework Security")
elseif(WIN32)
    target_link_libraries(storage_example PRIVATE user32 shell32 dwmapi gdiplus crypt32)
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENSSL REQUIRED IMPORTED_TARGET openssl)
    target_link_libraries(storage_example PRIVATE PkgConfig::OPENSSL pthread)
endif()

