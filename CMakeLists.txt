cmake_minimum_required(VERSION 3.10)

project(nativeapi_library VERSION 0.0.1 LANGUAGES CXX C)

# Add library subdirectory
add_subdirectory(src)

# Add example programs subdirectory
add_subdirectory(examples/display_example)
add_subdirectory(examples/keyboard_example)
add_subdirectory(examples/menu_example)
add_subdirectory(examples/menu_c_example)
add_subdirectory(examples/tray_icon_example)
add_subdirectory(examples/tray_icon_c_example)
add_subdirectory(examples/window_example)
