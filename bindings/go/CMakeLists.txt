cmake_minimum_required(VERSION 3.24)
project(nativeapi_go LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../core/src" nativeapi_core)

# Whole-archive keeps every C ABI symbol, including APIs Go hasn't called yet.
add_library(nativeapi_go SHARED src/runtime.cpp)
target_link_libraries(nativeapi_go PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,nativeapi>)
set_target_properties(nativeapi_go PROPERTIES
  LINKER_LANGUAGE CXX
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
# cgo uses one stable path for both single- and multi-configuration generators.
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  set_target_properties(nativeapi_go PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}"
  )
endforeach()
