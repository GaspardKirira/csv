cmake_minimum_required(VERSION 3.20)
project(csv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === CSV library (header-only) ===
add_library(csv INTERFACE)
add_library(csv::csv ALIAS csv)

target_include_directories(csv INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(csv INTERFACE /W4 /permissive-)
else()
  target_compile_options(csv INTERFACE -Wall -Wextra -Wpedantic)
endif()

# === Testing ===
include(CTest)
enable_testing()

add_executable(csv_basic_test tests/test_basic.cpp)
target_link_libraries(csv_basic_test PRIVATE csv::csv)
add_test(NAME csv.basic COMMAND csv_basic_test)

# === Examples ===
# Automatically add all .cpp files in examples/ as separate executables
file(GLOB EXAMPLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")

foreach(example_src IN LISTS EXAMPLE_SOURCES)
  get_filename_component(example_name ${example_src} NAME_WE)
  add_executable(${example_name} ${example_src})
  target_link_libraries(${example_name} PRIVATE csv::csv)
endforeach()
