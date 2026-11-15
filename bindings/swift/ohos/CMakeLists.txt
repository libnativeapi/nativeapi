cmake_minimum_required(VERSION 3.16)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "OHOS")
  message(FATAL_ERROR "Use the HarmonyOS NDK toolchain for the NativeAPI Swift host")
endif()

get_filename_component(NATIVEAPI_CORE "${CMAKE_CURRENT_LIST_DIR}/../../../core" ABSOLUTE)
if(NOT TARGET nativeapi)
  add_subdirectory("${NATIVEAPI_CORE}/src" nativeapi-core)
endif()

# The UIAbility host owns the Node-API environment and calls the existing
# lifecycle functions in ohos_runtime.h before entering Swift.
add_library(nativeapi_swift_ohos INTERFACE)
target_include_directories(nativeapi_swift_ohos INTERFACE "${NATIVEAPI_CORE}/src/platform/ohos")
target_link_libraries(nativeapi_swift_ohos INTERFACE nativeapi)
