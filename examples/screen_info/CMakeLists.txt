cmake_minimum_required(VERSION 3.10)

project(screen_info_example VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add example program
add_executable(screen_info_example
  "main.cpp"
)

# Link main library
target_link_libraries(screen_info_example PRIVATE libnativeapi)

# Set example program properties
set_target_properties(screen_info_example PROPERTIES
  OUTPUT_NAME "screen_info_example"
)

# Set example program compile options (macOS only)
if(APPLE)
  set_source_files_properties("main.cpp"
    PROPERTIES
    COMPILE_FLAGS "-x objective-c++"
  )
endif()
