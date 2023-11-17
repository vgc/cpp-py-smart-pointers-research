cmake_minimum_required(VERSION 3.12...3.22)
project(cpp-py-smart-pointers-research)
set(CMAKE_CXX_STANDARD 17)

find_package(Python 3.6 COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Python Executable:    ${Python_EXECUTABLE}")

add_subdirectory(third/pybind11)

# Define a tests target that builds all tests (in particular, C++ tests)
enable_testing()
add_custom_target(all_tests)
set_target_properties(all_tests PROPERTIES FOLDER misc)

# Define a helper function that each experiment will use
function(add_experiment LIB_NAME)
    set(BASE_TARGET         ${LIB_NAME})
    set(LIB_TARGET          ${LIB_NAME}_lib)
    set(WRAPS_TARGET        ${LIB_NAME}_wraps)
    set(CHECK_TARGET        ${LIB_NAME}_check)
    set(TESTS_TARGET        ${LIB_NAME}_tests)
    set(CPP_TESTS_TARGET    ${LIB_NAME}_cpp_tests)
    set(PYTHON_TESTS_TARGET ${LIB_NAME}_python_tests)

    # Parse arguments
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs CPP_LIBRARY_FILES PYTHON_MODULE_FILES PYTHON_TEST_FILES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Generic target building both the library and the python module
    add_custom_target(${BASE_TARGET})
    set_target_properties(${BASE_TARGET}
        PROPERTIES
            FOLDER misc/libs
    )

    # C++ library
    add_library(${LIB_TARGET} SHARED ${ARG_CPP_LIBRARY_FILES})
    set_target_properties(${LIB_TARGET}
        PROPERTIES
            OUTPUT_NAME ${LIB_NAME}
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/lib
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/bin
            FOLDER libs/${LIB_NAME}
    )
    target_compile_definitions(${LIB_TARGET} PRIVATE EXPORTS)
    set_target_properties(${LIB_TARGET} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    add_dependencies(${BASE_TARGET} ${LIB_TARGET})

    # Python module
    pybind11_add_module(${WRAPS_TARGET} ${ARG_PYTHON_MODULE_FILES})
    set_target_properties(${WRAPS_TARGET}
        PROPERTIES
            OUTPUT_NAME ${LIB_NAME}
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/python
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/python
            FOLDER libs/${LIB_NAME}
    )
    target_link_libraries(${WRAPS_TARGET} PRIVATE ${LIB_TARGET})
    add_dependencies(${BASE_TARGET} ${WRAPS_TARGET})

    # Tests (for now, there are only Python tests, but this anticipate potential C++ tests)
    add_custom_target(${TESTS_TARGET} SOURCES ${ARG_CPP_TEST_FILES} ${ARG_PYTHON_TEST_FILES})
    set_target_properties(${TESTS_TARGET} PROPERTIES FOLDER libs/${LIB_NAME}/tests)
    add_dependencies(all_tests ${TESTS_TARGET})

    # Python tests
    if(ARG_PYTHON_TEST_FILES)
        add_custom_target(${PYTHON_TESTS_TARGET} SOURCES ${ARG_PYTHON_TEST_FILES})
        set_target_properties(${PYTHON_TESTS_TARGET} PROPERTIES FOLDER libs/${LIB_NAME}/tests)
        add_dependencies(${TESTS_TARGET} ${PYTHON_TESTS_TARGET})
        add_dependencies(${PYTHON_TESTS_TARGET} ${BASE_TARGET})
    endif()
    foreach(FILENAME ${ARG_PYTHON_TEST_FILES})
        set(TEST_TARGET ${LIB_NAME}_${FILENAME})
        add_test(
            NAME ${TEST_TARGET}
            COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME} -v
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        if(WIN32)
            set_tests_properties(${TEST_TARGET} PROPERTIES
                ENVIRONMENT "PATH=%PATH%\;${CMAKE_BINARY_DIR}/$<CONFIG>/bin"
            )
        else()
            set_tests_properties(${TEST_TARGET} PROPERTIES
                ENVIRONMENT PYTHONPATH=${CMAKE_BINARY_DIR}/$<CONFIG>/python:$ENV{PYTHONPATH}
            )
        endif()
    endforeach()

    # Create a target <libname>_check that builds and runs the tests
    if (CMAKE_CONFIGURATION_TYPES)
        add_custom_target(${CHECK_TARGET} SOURCES ${ARG_CPP_TEST_FILES} ${ARG_PYTHON_TEST_FILES}
            COMMAND ${CMAKE_CTEST_COMMAND}
                --force-new-ctest-process --output-on-failure
                --build-config "$<CONFIGURATION>"
                -R ${LIB_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    else()
        add_custom_target(${CHECK_TARGET} SOURCES ${ARG_CPP_TEST_FILES} ${ARG_PYTHON_TEST_FILES}
            COMMAND ${CMAKE_CTEST_COMMAND}
                --force-new-ctest-process --output-on-failure
                -R ${LIB_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif()
    set_target_properties(${CHECK_TARGET} PROPERTIES FOLDER libs/${LIB_NAME})
    add_dependencies(${CHECK_TARGET} ${TESTS_TARGET})

endfunction()

add_subdirectory(libs)
