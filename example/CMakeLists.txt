cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(status-code-examples VERSION 1.0 LANGUAGES CXX)

add_subdirectory(".." "status-code" EXCLUDE_FROM_ALL)

add_executable(file_io_error "file_io_error.cpp")
target_link_libraries(file_io_error PRIVATE status-code)

add_executable(thrown_exception "thrown_exception.cpp")
target_link_libraries(thrown_exception PRIVATE status-code)
target_compile_features(thrown_exception PRIVATE cxx_std_17)
