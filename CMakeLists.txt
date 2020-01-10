cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(status-code VERSION 1.0 LANGUAGES CXX)
enable_testing()
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(PROJECT_IS_DEPENDENCY OFF)
else()
  set(PROJECT_IS_DEPENDENCY ON)
endif()

# On MSVC very annoyingly cmake puts /EHsc into the global flags which means you
# get a warning when you try to disable exceptions. I hate to use this
# globally imposed solution, but we are going to hack the global flags to use properties to
# determine whether they are on or off
#
# Create custom properties called CXX_EXCEPTIONS and CXX_RTTI
# These get placed at global, directory and target scopes
foreach(scope GLOBAL DIRECTORY TARGET)
  define_property(${scope} PROPERTY "CXX_EXCEPTIONS" INHERITED
    BRIEF_DOCS "Enable C++ exceptions, defaults to ON at global scope"
    FULL_DOCS "Not choosing ON nor OFF with exact capitalisation will lead to misoperation!"
  )
  define_property(${scope} PROPERTY "CXX_RTTI" INHERITED
    BRIEF_DOCS "Enable C++ runtime type information, defaults to ON at global scope"
    FULL_DOCS "Not choosing ON nor OFF with exact capitalisation will lead to misoperation!"
  )
endforeach()
# Set the default for these properties at global scope. If they are not set per target or
# whatever, the next highest scope will be looked up
set_property(GLOBAL PROPERTY CXX_EXCEPTIONS ON)
set_property(GLOBAL PROPERTY CXX_RTTI ON)
if(MSVC)
  # Purge unconditional use of these flags and remove all the ignored
  # cruft which cmake adds for the LLVM-vs* toolset.
  set(purgelist
    "/EHsc"
    "/GR"
    "/Gm-"
    "-fms-extensions"
    "-fms-compatibility"
    #"-Wall"
    "-frtti"
    "-fexceptions"
    "-gline-tables-only"
    "-fno-inline"
    #"-O0"
  )
  foreach(flag
          CMAKE_C_FLAGS                CMAKE_CXX_FLAGS
          CMAKE_C_FLAGS_DEBUG          CMAKE_CXX_FLAGS_DEBUG
          CMAKE_C_FLAGS_RELEASE        CMAKE_CXX_FLAGS_RELEASE
          CMAKE_C_FLAGS_MINSIZEREL     CMAKE_CXX_FLAGS_MINSIZEREL
          CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO
          )
    foreach(item ${purgelist})
      string(REPLACE "${item}"  "" ${flag} "${${flag}}")
    endforeach()
    string(REPLACE "-O0"  "/O0" ${flag} "${${flag}}")
    string(REPLACE "-O1"  "/O1" ${flag} "${${flag}}")
    string(REPLACE "-O2"  "/O2" ${flag} "${${flag}}")
    #message(STATUS "${flag} = ${${flag}}")
  endforeach()
  # Restore those same, but now selected by the properties
  add_compile_options(
    $<$<STREQUAL:$<TARGET_PROPERTY:CXX_EXCEPTIONS>,ON>:/EHsc>
    $<$<STREQUAL:$<TARGET_PROPERTY:CXX_RTTI>,OFF>:/GR->
  )
else()
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:$<$<STREQUAL:$<TARGET_PROPERTY:CXX_EXCEPTIONS>,ON>:-fexceptions>>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<STREQUAL:$<TARGET_PROPERTY:CXX_RTTI>,ON>:-frtti>>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<STREQUAL:$<TARGET_PROPERTY:CXX_EXCEPTIONS>,OFF>:-fno-exceptions>>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<STREQUAL:$<TARGET_PROPERTY:CXX_RTTI>,OFF>:-fno-rtti>>
  )
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
  "${CMAKE_CURRENT_SOURCE_DIR}/include/result.hpp"
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

  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "6.0")
    add_executable(test-result "test/result.cpp")
    target_compile_features(test-result PRIVATE cxx_std_17)
    target_link_libraries(test-result PRIVATE status-code)
    set_target_properties(test-result PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    add_test(NAME test-result COMMAND $<TARGET_FILE:test-result>)
  endif()

  add_executable(test-status-code "test/main.cpp")
  target_link_libraries(test-status-code PRIVATE status-code)
  set_target_properties(test-status-code PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_test(NAME test-status-code COMMAND $<TARGET_FILE:test-status-code>)
  
  add_executable(test-status-code-noexcept "test/main.cpp")
  target_link_libraries(test-status-code-noexcept PRIVATE status-code)
  set_target_properties(test-status-code-noexcept PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_EXCEPTIONS Off
    CXX_RTTI Off
  )
  add_test(NAME test-status-code-noexcept COMMAND $<TARGET_FILE:test-status-code-noexcept>)
  
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
