add_executable(url_opener_test url_opener_test.cpp)
target_link_libraries(url_opener_test PRIVATE nativeapi)
add_test(NAME url_opener_test COMMAND url_opener_test)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_executable(autostart_test autostart_test.cpp)
  target_link_libraries(autostart_test PRIVATE nativeapi)
  add_test(NAME autostart_test COMMAND autostart_test)
endif()
