cmake_minimum_required(VERSION 3.10)

project(nativeapi_library VERSION 0.0.1 LANGUAGES CXX C)

# Add library subdirectory
add_subdirectory(src)

# Add example programs subdirectory
add_subdirectory(examples/application_example)
add_subdirectory(examples/application_c_example)
add_subdirectory(examples/display_example)
add_subdirectory(examples/display_c_example)
add_subdirectory(examples/id_allocator_example)
add_subdirectory(examples/keyboard_example)
add_subdirectory(examples/menu_example)
add_subdirectory(examples/menu_c_example)
add_subdirectory(examples/storage_example)
add_subdirectory(examples/storage_c_example)
add_subdirectory(examples/tray_icon_example)
add_subdirectory(examples/tray_icon_c_example)
add_subdirectory(examples/window_c_example)
add_subdirectory(examples/window_example)
