cmake_minimum_required(VERSION 3.16)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "OHOS")
  message(FATAL_ERROR "Use the HarmonyOS NDK toolchain for the NativeAPI Swift host")
endif()

get_filename_component(NATIVEAPI_CORE "${CMAKE_CURRENT_LIST_DIR}/../../../core" ABSOLUTE)
get_filename_component(NATIVEAPI_SWIFT_BINDING "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
if(NOT TARGET nativeapi)
  add_subdirectory("${NATIVEAPI_CORE}/src" nativeapi-core)
endif()

# Provides Swift-on-OpenHarmony runtime glue: libdispatch main-queue polling on
# the N-API uv loop, RunLoopTimer support, and missing musl/Foundation shims.
add_library(nativeapi_swift_ohos STATIC
  "${CMAKE_CURRENT_LIST_DIR}/swift_ohos_support.cpp"
)
target_compile_features(nativeapi_swift_ohos PUBLIC cxx_std_17)
target_include_directories(nativeapi_swift_ohos PUBLIC
  "${CMAKE_CURRENT_LIST_DIR}/include"
  "${NATIVEAPI_CORE}/src/platform/ohos"
  "${NATIVEAPI_SWIFT_BINDING}/Sources/CNativeAPI/include"
)
target_link_libraries(nativeapi_swift_ohos PUBLIC
  nativeapi
  libace_napi.z.so
  c++_shared
  uv
  dl
  m
)

# Links a shared library target against the output directory prepared by
# bindings/swift/scripts/build-ohos.sh (NativeAPI.o, swiftrt.o, and the
# patched Swift Static Linux SDK runtime archives).
function(nativeapi_target_link_swift_ohos target runtime_dir)
  target_sources(${target} PRIVATE
    "${runtime_dir}/NativeAPI.o"
    "${runtime_dir}/swiftrt.o"
  )
  target_link_libraries(${target} PRIVATE
    nativeapi_swift_ohos
    -Wl,--start-group
    "${runtime_dir}/libswiftCore.a"
    "${runtime_dir}/libswift_Concurrency.a"
    "${runtime_dir}/libswift_StringProcessing.a"
    "${runtime_dir}/libswift_RegexParser.a"
    "${runtime_dir}/libswiftMusl.a"
    "${runtime_dir}/libswiftSynchronization.a"
    "${runtime_dir}/libswiftDispatch.a"
    "${runtime_dir}/libdispatch.a"
    "${runtime_dir}/libBlocksRuntime.a"
    "${runtime_dir}/libFoundation.a"
    "${runtime_dir}/libFoundationEssentials.a"
    "${runtime_dir}/libFoundationInternationalization.a"
    "${runtime_dir}/libCoreFoundation.a"
    "${runtime_dir}/lib_FoundationCShims.a"
    "${runtime_dir}/lib_FoundationCollections.a"
    "${runtime_dir}/lib_FoundationICU.a"
    "${runtime_dir}/libfts.a"
    "${runtime_dir}/libc++.a"
    "${runtime_dir}/libc++abi.a"
    -Wl,--end-group
  )
endfunction()
