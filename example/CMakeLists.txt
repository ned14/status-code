cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(status-code-examples VERSION 1.0 LANGUAGES CXX)

add_subdirectory(".." "status-code" EXCLUDE_FROM_ALL)

add_executable(file_io_error "file_io_error.cpp")
target_link_libraries(file_io_error PRIVATE status-code)

add_executable(quick_status_code_from_enum "quick_status_code_from_enum.cpp")
target_link_libraries(quick_status_code_from_enum PRIVATE status-code)

add_executable(thrown_exception "thrown_exception.cpp")
target_link_libraries(thrown_exception PRIVATE status-code)
target_compile_features(thrown_exception PRIVATE cxx_std_17)

file(DOWNLOAD "https://raw.githubusercontent.com/ned14/outcome/develop/single-header/outcome-experimental.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/outcome-experimental.hpp" SHOW_PROGRESS)
if(EXISTS "outcome-experimental.hpp")
  add_executable(variant_return "variant_return.cpp")
endif()
