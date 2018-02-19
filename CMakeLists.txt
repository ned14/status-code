cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(status-code VERSION 1.0 LANGUAGES CXX)
enable_testing()
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(PROJECT_IS_DEPENDENCY OFF)
else()
  set(PROJECT_IS_DEPENDENCY ON)
endif()

add_library(status-code INTERFACE)
target_compile_features(status-code INTERFACE cxx_std_11)
target_include_directories(status-code INTERFACE "include")

#export(
#  TARGETS status-code
#  FILE "StatusCodeTargets.cmake"
#  EXPORT_LINK_INTERFACE_LIBRARIES
#)

if(NOT PROJECT_IS_DEPENDENCY)
  add_executable(test-status-code "test/main.cpp")
  target_link_libraries(test-status-code PRIVATE status-code)
  set_target_properties(test-status-code PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_test(NAME test-status-code COMMAND $<TARGET_FILE:test-status-code>)
  
  if(WIN32)
    add_executable(generate-tables "utils/generate-tables.cpp")
    target_link_libraries(test-status-code PRIVATE status-code)
    set_target_properties(test-status-code PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
  endif()
endif()
