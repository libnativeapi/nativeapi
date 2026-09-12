add_executable(url_opener_test url_opener_test.cpp)
target_link_libraries(url_opener_test PRIVATE nativeapi)
add_test(NAME url_opener_test COMMAND url_opener_test)

# Links the full library because EventEmitter now routes async delivery through
# the platform dispatcher. The test installs its own dispatcher (see
# FakeMainThread) so no run loop is required at runtime.
find_package(Threads REQUIRED)
add_executable(event_emitter_test event_emitter_test.cpp)
target_link_libraries(event_emitter_test PRIVATE nativeapi Threads::Threads)
add_test(NAME event_emitter_test COMMAND event_emitter_test)
set_tests_properties(event_emitter_test PROPERTIES TIMEOUT 120)

add_executable(id_allocator_test id_allocator_test.cpp)
target_link_libraries(id_allocator_test PRIVATE nativeapi Threads::Threads)
add_test(NAME id_allocator_test COMMAND id_allocator_test)

add_executable(handle_table_test handle_table_test.cpp)
target_link_libraries(handle_table_test PRIVATE nativeapi Threads::Threads)
add_test(NAME handle_table_test COMMAND handle_table_test)
set_tests_properties(handle_table_test PROPERTIES TIMEOUT 120)

add_executable(shortcut_accelerator_test shortcut_accelerator_test.cpp)
target_link_libraries(shortcut_accelerator_test PRIVATE nativeapi)
add_test(NAME shortcut_accelerator_test COMMAND shortcut_accelerator_test)

if(WIN32)
  add_executable(menu_backend_test menu_backend_test.cpp)
  target_link_libraries(menu_backend_test PRIVATE nativeapi)
  add_test(NAME menu_backend_test COMMAND menu_backend_test)
  if(NATIVEAPI_ENABLE_WINUI3)
    target_sources(menu_backend_test PRIVATE "../cmake/winui3-example.manifest")
    nativeapi_deploy_winui3(menu_backend_test)
  endif()
endif()

# Explicit desktop integration test; not part of headless CTest runs.
if(WIN32 AND NATIVEAPI_ENABLE_WINUI3)
  add_executable(winui3_dialog_test winui3_dialog_test.cpp ../cmake/winui3-example.manifest)
  target_link_libraries(winui3_dialog_test PRIVATE nativeapi)
  nativeapi_deploy_winui3(winui3_dialog_test)
endif()
