cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

project(status-code VERSION 1.0 LANGUAGES CXX)
include(GNUInstallDirs)
enable_testing()
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(status-code_IS_DEPENDENCY OFF)
else()
  set(status-code_IS_DEPENDENCY ON)
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
target_include_directories(status-code INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
)
foreach(source 
    "include/status-code/detail/nt_code_to_generic_code.ipp"
    "include/status-code/detail/nt_code_to_win32_code.ipp"
    "include/status-code/detail/win32_code_to_generic_code.ipp"
    "include/status-code/boost_error_code.hpp"
    "include/status-code/com_code.hpp"
    "include/status-code/config.hpp"
    "include/status-code/error.hpp"
    "include/status-code/errored_status_code.hpp"
    "include/status-code/generic_code.hpp"
    "include/status-code/getaddrinfo_code.hpp"
    "include/status-code/http_status_code.hpp"
    "include/status-code/iostream_support.hpp"
    "include/status-code/nested_status_code.hpp"
    "include/status-code/nt_code.hpp"
    "include/status-code/posix_code.hpp"
    "include/status-code/quick_status_code_from_enum.hpp"
    "include/status-code/result.hpp"
    "include/status-code/status_code.hpp"
    "include/status-code/status_code_domain.hpp"
    "include/status-code/status_error.hpp"
    "include/status-code/std_error_code.hpp"
    "include/status-code/system_code.hpp"
    "include/status-code/system_code_from_exception.hpp"
    "include/status-code/system_error2.hpp"
    "include/status-code/win32_code.hpp"
  )
  target_sources(status-code INTERFACE
    "$<INSTALL_INTERFACE:${source}>"
  )
  get_filename_component(dir ${source} DIRECTORY)
  install(FILES "${source}"
    DESTINATION "${dir}"
  )
endforeach()

install(TARGETS status-code
        EXPORT status-codeExports
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
set_target_properties(status-code PROPERTIES EXPORT_NAME hl)

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  @ONLY
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
install(EXPORT status-codeExports
  NAMESPACE status-code::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/status-code"
)

if(NOT status-code_IS_DEPENDENCY AND (NOT DEFINED BUILD_TESTING OR BUILD_TESTING))
  include(FindPythonInterp)
  # Make preprocessed edition of this library target
  if(NOT PYTHONINTERP_FOUND)
    message(WARNING "NOT rebuilding preprocessed edition of library due to python not being installed")
  else()
    # See if the ply package is installed so pcpp can run
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" -c "import pcpp" RESULT_VARIABLE python_has_pcpp)
    if(NOT python_has_pcpp EQUAL 0)
      message(WARNING "NOT rebuilding preprocessed edition of library due to installed python not having the pcpp package installed. "
        "Do '(sudo) pip install pcpp' to fix.")
    else()
      add_custom_target(status-code-pp 
        pcpp -o "${CMAKE_CURRENT_SOURCE_DIR}/single-header/system_error2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/status-code/system_error2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/status-code/nested_status_code.hpp"
        --passthru-defines --passthru-unfound-includes --passthru-unknown-exprs
        --passthru-comments --line-directive --compress # --debug
        -U STANDARDESE_IS_IN_THE_HOUSE
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Preprocessing ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}.hpp into ${CMAKE_CURRENT_SOURCE_DIR}/single-header/${PROJECT_NAME}.hpp ..."
      )
      add_custom_target(status-code-nowindows-pp 
        pcpp -o "${CMAKE_CURRENT_SOURCE_DIR}/single-header/system_error2-nowindows.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/status-code/system_error2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/status-code/nested_status_code.hpp"
        --passthru-defines --passthru-unfound-includes --passthru-unknown-exprs
        --passthru-comments --line-directive --compress # --debug
        -U STANDARDESE_IS_IN_THE_HOUSE -U _WIN32
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Preprocessing ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}.hpp into ${CMAKE_CURRENT_SOURCE_DIR}/single-header/${PROJECT_NAME}-nowindows.hpp ..."
      )
      if(NOT CMAKE_VERSION VERSION_LESS 3.3)
        add_dependencies(status-code status-code-pp status-code-nowindows-pp)
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

# Forgot to git add this on the other computer
#  find_package(Boost COMPONENTS system)
#  if(Boost_FOUND)
#    add_executable(test-boost_error_code "test/boost_error_code.cpp")
#    target_link_libraries(test-boost_error_code PRIVATE status-code Boost::system)
#    set_target_properties(test-boost_error_code PROPERTIES
#      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
#    )
#    add_test(NAME test-boost_error_code COMMAND $<TARGET_FILE:test-boost_error_code>)
#  endif()
  
  add_executable(test-issue0050 "test/issue0050.cpp")
  target_link_libraries(test-issue0050 PRIVATE status-code)
  set_target_properties(test-issue0050 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_test(NAME test-issue0050 COMMAND $<TARGET_FILE:test-issue0050>)
  
  add_executable(test-issue0056 "test/issue0056.cpp")
  target_compile_features(test-issue0056 PRIVATE cxx_std_17)
  target_link_libraries(test-issue0056 PRIVATE status-code)
  set_target_properties(test-issue0056 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_test(NAME test-issue0056 COMMAND $<TARGET_FILE:test-issue0056>)
  
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
  
  add_executable(test-status-code-not-posix "test/main.cpp")
  target_compile_definitions(test-status-code-not-posix PRIVATE SYSTEM_ERROR2_NOT_POSIX=1 "SYSTEM_ERROR2_FATAL=::abort()")
  target_link_libraries(test-status-code-not-posix PRIVATE status-code)
  set_target_properties(test-status-code-not-posix PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_test(NAME test-status-code-not-posix COMMAND $<TARGET_FILE:test-status-code-not-posix>)
  
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

  # Compile examples
  add_executable(example-quick_status_code_from_enum "example/quick_status_code_from_enum.cpp")
  target_link_libraries(example-quick_status_code_from_enum PRIVATE status-code)
  set_target_properties(example-quick_status_code_from_enum PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_executable(example-thrown_exception "example/thrown_exception.cpp")
  target_link_libraries(example-thrown_exception PRIVATE status-code)
  set_target_properties(example-thrown_exception PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  target_compile_features(example-thrown_exception PRIVATE cxx_std_17)
  add_executable(example-file_io_error "wg21/file_io_error.cpp")
  target_link_libraries(example-file_io_error PRIVATE status-code)
  set_target_properties(example-file_io_error PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  
endif()
