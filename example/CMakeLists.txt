cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(status-code-examples VERSION 1.0 LANGUAGES CXX)

add_subdirectory(".." "status-code" EXCLUDE_FROM_ALL)

add_executable(file_io_error "file_io_error.cpp")
target_link_libraries(file_io_error PRIVATE status-code)
