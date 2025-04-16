cmake_minimum_required(VERSION 3.10)

project(nswindow_example VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Objective-C++
enable_language(OBJCXX)

# Add example program
add_executable(nswindow_example
  "main.mm"
)

# Link main library
target_link_libraries(nswindow_example PRIVATE libnativeapi)

# Set example program properties
set_target_properties(nswindow_example PROPERTIES
  OUTPUT_NAME "nswindow_example"
)

# Set example program compile options (macOS only)
if(APPLE)
  set_source_files_properties("main.mm"
    PROPERTIES
    COMPILE_FLAGS "-x objective-c++"
  )
endif()
