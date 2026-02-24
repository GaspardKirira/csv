cmake_minimum_required(VERSION 3.20)
project(csv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

include(CTest)
enable_testing()

add_executable(csv_basic_test tests/test_basic.cpp)
target_link_libraries(csv_basic_test PRIVATE csv::csv)
add_test(NAME csv.basic COMMAND csv_basic_test)
