cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

option(BEMAN_STATUS_CODE "If we are targeting the Beman project" OFF)

if(BEMAN_STATUS_CODE)
  set(PROJECT_NAME beman.status_code)
  set(LIBRARY_NAME beman.status_code)
  set(PATH_PREFIX "beman/status_code")
  set(EXPORT_NAME "status_code")
  set(EXPORT_NAMESPACE "beman::")
else()
  set(PROJECT_NAME status-code)
  set(LIBRARY_NAME status-code)
  set(PATH_PREFIX "status-code")
  set(EXPORT_NAME "hl")
  set(EXPORT_NAMESPACE "status-code::")
endif()

project(${PROJECT_NAME} VERSION 1.0
  DESCRIPTION "The reference implementation for https://wg21.link/P1028 status_code"
  LANGUAGES CXX
)
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

add_library(${LIBRARY_NAME} INTERFACE)
target_compile_features(${LIBRARY_NAME} INTERFACE cxx_std_11)
if(BEMAN_STATUS_CODE)
  target_compile_definitions(${LIBRARY_NAME} INTERFACE
    "SYSTEM_ERROR2_NAMESPACE=beman::status_code"
    "SYSTEM_ERROR2_NAMESPACE_BEGIN=namespace beman { namespace status_code {"
    "SYSTEM_ERROR2_NAMESPACE_END=} }"
  )
endif()
target_include_directories(${LIBRARY_NAME} INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
)
foreach(source 
    "include/${PATH_PREFIX}/detail/nt_code_to_generic_code.ipp"
    "include/${PATH_PREFIX}/detail/nt_code_to_win32_code.ipp"
    "include/${PATH_PREFIX}/detail/win32_code_to_generic_code.ipp"
    "include/${PATH_PREFIX}/boost_error_code.hpp"
    "include/${PATH_PREFIX}/com_code.hpp"
    "include/${PATH_PREFIX}/config.hpp"
    "include/${PATH_PREFIX}/error.hpp"
    "include/${PATH_PREFIX}/errored_status_code.hpp"
    "include/${PATH_PREFIX}/generic_code.hpp"
    "include/${PATH_PREFIX}/getaddrinfo_code.hpp"
    "include/${PATH_PREFIX}/http_status_code.hpp"
    "include/${PATH_PREFIX}/iostream_support.hpp"
    "include/${PATH_PREFIX}/nested_status_code.hpp"
    "include/${PATH_PREFIX}/nt_code.hpp"
    "include/${PATH_PREFIX}/posix_code.hpp"
    "include/${PATH_PREFIX}/quick_status_code_from_enum.hpp"
    "include/${PATH_PREFIX}/result.hpp"
    "include/${PATH_PREFIX}/status_code.hpp"
    "include/${PATH_PREFIX}/status_code_domain.hpp"
    "include/${PATH_PREFIX}/status_error.hpp"
    "include/${PATH_PREFIX}/std_error_code.hpp"
    "include/${PATH_PREFIX}/system_code.hpp"
    "include/${PATH_PREFIX}/system_code_from_exception.hpp"
    "include/${PATH_PREFIX}/system_error2.hpp"
    "include/${PATH_PREFIX}/win32_code.hpp"
  )
  target_sources(${LIBRARY_NAME} INTERFACE
    "$<INSTALL_INTERFACE:${source}>"
  )
  get_filename_component(dir ${source} DIRECTORY)
  install(FILES "${source}"
    DESTINATION "${dir}"
  )
endforeach()

install(TARGETS ${LIBRARY_NAME}
        EXPORT ${LIBRARY_NAME}Exports
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
set_target_properties(${LIBRARY_NAME} PROPERTIES EXPORT_NAME ${EXPORT_NAME})

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  @ONLY
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
install(EXPORT ${LIBRARY_NAME}Exports
  NAMESPACE ${EXPORT_NAMESPACE}
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/include/${PATH_PREFIX}/system_error2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/${PATH_PREFIX}/nested_status_code.hpp"
        --passthru-defines --passthru-unfound-includes --passthru-unknown-exprs
        --passthru-comments --line-directive --compress # --debug
        -U STANDARDESE_IS_IN_THE_HOUSE
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Preprocessing ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}.hpp into ${CMAKE_CURRENT_SOURCE_DIR}/single-header/${PROJECT_NAME}.hpp ..."
      )
      add_custom_target(status-code-nowindows-pp 
        pcpp -o "${CMAKE_CURRENT_SOURCE_DIR}/single-header/system_error2-nowindows.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/${PATH_PREFIX}/system_error2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/${PATH_PREFIX}/nested_status_code.hpp"
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

  function(add_status_code_example target)
    cmake_parse_arguments(ADD_EXAMPLE "" "" "SOURCES;FEATURES;PROPERTIES" ${ARGN})
    add_executable(${target} ${ADD_EXAMPLE_SOURCES})
    target_compile_features(${target} PRIVATE ${ADD_EXAMPLE_FEATURES})
    target_compile_definitions(${target} PRIVATE "STATUS_CODE_INCLUDE_PATH=\"${PATH_PREFIX}/system_error2.hpp\"")
    target_link_libraries(${target} PRIVATE ${LIBRARY_NAME})
    set_target_properties(${target} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
      ${ADD_EXAMPLE_PROPERTIES}
    )
  endfunction()
  function(add_status_code_test target)
    add_status_code_example(${target} ${ARGN})
    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
  endfunction()

  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "6.0")
    add_status_code_test(test-result SOURCES "test/result.cpp" FEATURES cxx_std_17)
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
  
  add_status_code_test(test-issue0050 SOURCES "test/issue0050.cpp")
  add_status_code_test(test-issue0056 SOURCES "test/issue0056.cpp" FEATURES cxx_std_17)
  add_status_code_test(test-status-code SOURCES "test/main.cpp")
  add_status_code_test(test-status-code-noexcept SOURCES "test/main.cpp" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_EXCEPTIONS Off
    CXX_RTTI Off
  )
  
  add_status_code_test(test-status-code-not-posix SOURCES "test/main.cpp")
  target_compile_definitions(test-status-code-not-posix PRIVATE SYSTEM_ERROR2_NOT_POSIX=1 "SYSTEM_ERROR2_FATAL=::abort()")
  
  add_status_code_test(test-status-code-p0709a SOURCES "test/p0709a.cpp")
  
  if(WIN32)
    add_executable(generate-tables "utils/generate-tables.cpp")
    set_target_properties(generate-tables PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
  endif()

  # Compile examples
  add_status_code_example(example-quick_status_code_from_enum SOURCES "example/quick_status_code_from_enum.cpp")
  add_status_code_example(example-thrown_exception SOURCES "example/thrown_exception.cpp" FEATURES cxx_std_17)
  add_status_code_example(example-file_io_error SOURCES "wg21/file_io_error.cpp")  
endif()
