cmake_minimum_required(VERSION 3.10)

# Project name
set(JSON_REST_CLIENT "JsonRestClient")
project(
    ${JSON_REST_CLIENT}
    LANGUAGES CXX
    VERSION 0.1
    DESCRIPTION "JSON REST Client"
)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Directory where RapidJSON will be cloned
set(RAPIDJSON_INCLUDE_DIR "${CMAKE_BINARY_DIR}/rapidjson/include")

# Clone RapidJSON if it's not already cloned
if(NOT EXISTS "${RAPIDJSON_INCLUDE_DIR}/rapidjson")
    message(STATUS "Cloning RapidJSON repository...")
    execute_process(
        COMMAND git clone --depth 1 https://github.com/Tencent/rapidjson.git rapidjson
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endif()

message(STATUS "RapidJSON source directory: ${RAPIDJSON_INCLUDE_DIR}")

find_package(CURL REQUIRED)

# Set compiler flags
add_compile_options(-Wall -Wextra -Werror)

set(SOURCE_FILES
    src/client.cpp
    src/data_objects.cpp
    src/query_to_json.cpp
    src/tables.cpp
)

set(JSON_REST_CLIENT_LIB "JRC")

# Add the library
add_library(${JSON_REST_CLIENT_LIB} "${SOURCE_FILES}")
target_include_directories(${JSON_REST_CLIENT_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${JSON_REST_CLIENT_LIB} PUBLIC ${RAPIDJSON_INCLUDE_DIR})
target_link_libraries(${JSON_REST_CLIENT_LIB} CURL::libcurl)

# Add the executable
add_executable(${JSON_REST_CLIENT} "main.cpp")
target_link_libraries(${JSON_REST_CLIENT} ${JSON_REST_CLIENT_LIB})

# Enable testing
enable_testing()

# Download and unpack GoogleTest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

# Add GoogleTest targets
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

set(TEST_FILES
    tests/test_data_objects.cpp
    tests/test_tables.cpp
)

# Link test executables against GoogleTest

function(create_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name}
        gtest_main
        ${JSON_REST_CLIENT_LIB}
    )
    add_test(NAME ${test_name} COMMAND ${test_name})

    # Add a label
    set_tests_properties(${test_name} PROPERTIES LABELS "unit_test" )
endfunction()

create_test("data_objects_test" "tests/test_data_objects.cpp")
create_test("tables_test" "tests/test_tables.cpp")
