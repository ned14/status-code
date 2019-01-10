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
target_sources(status-code INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include/com_code.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/config.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/error.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/errored_status_code.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/generic_code.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/iostream_support.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/nt_code.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/posix_code.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/status_code.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/status_code_domain.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/status_code_ptr.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/status_error.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/std_error_code.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/system_code.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/system_error2.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/win32_code.hpp"
)

#export(
#  TARGETS status-code
#  FILE "StatusCodeTargets.cmake"
#  EXPORT_LINK_INTERFACE_LIBRARIES
#)

if(NOT PROJECT_IS_DEPENDENCY)
  include(FindPythonInterp)
  # Make preprocessed edition of this library target
  if(NOT PYTHONINTERP_FOUND)
    message(WARNING "NOT rebuilding preprocessed edition of library due to python not being installed")
  else()
    # See if the ply package is installed so pcpp can run
    execute_process(COMMAND python -c "import pcpp" RESULT_VARIABLE python_has_pcpp)
    if(NOT python_has_pcpp EQUAL 0)
      message(WARNING "NOT rebuilding preprocessed edition of library due to installed python not having the pcpp package installed. "
        "Do '(sudo) pip install pcpp' to fix.")
    else()
      add_custom_target(status-code-pp 
        pcpp -o "${CMAKE_CURRENT_SOURCE_DIR}/single-header/system_error2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/system_error2.hpp"
        --passthru-defines --passthru-unfound-includes --passthru-unknown-exprs
        --passthru-comments --line-directive --compress # --debug
        -U STANDARDESE_IS_IN_THE_HOUSE
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Preprocessing ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}.hpp into ${CMAKE_CURRENT_SOURCE_DIR}/single-header/${PROJECT_NAME}.hpp ..."
      )
      if(NOT CMAKE_VERSION VERSION_LESS 3.3)
        add_dependencies(status-code status-code-pp)
      endif()
    endif()
  endif()

  add_executable(test-status-code "test/main.cpp")
  target_link_libraries(test-status-code PRIVATE status-code)
  set_target_properties(test-status-code PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_test(NAME test-status-code COMMAND $<TARGET_FILE:test-status-code>)
  
  add_executable(test-status-code-p0709a "test/p0709a.cpp")
  target_link_libraries(test-status-code-p0709a PRIVATE status-code)
  set_target_properties(test-status-code-p0709a PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_test(NAME test-status-code-p0709a COMMAND $<TARGET_FILE:test-status-code-p0709a>)
  
  if(WIN32)
    add_executable(generate-tables "utils/generate-tables.cpp")
    target_link_libraries(test-status-code PRIVATE status-code)
    set_target_properties(test-status-code PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
  endif()
endif()
