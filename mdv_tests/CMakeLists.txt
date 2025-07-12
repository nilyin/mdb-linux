cmake_minimum_required(VERSION 3.10)
project(mdv_tests)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mdv_perf_simple.c)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mdv_perf_minimal.c)

use_c11()

add_executable(mdv_tests ${SOURCES})

target_compile_options(mdv_tests PRIVATE -Wall -Wextra)

target_include_directories(mdv_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mdv_tests PRIVATE mdv::platform mdv::core mdv::types mdv::crypto mdv::storage)

# Simple performance test
add_executable(mdv_perf_simple mdv_perf_simple.c)
target_compile_options(mdv_perf_simple PRIVATE -Wall -Wextra)
target_include_directories(mdv_perf_simple PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mdv_perf_simple PRIVATE mdv::platform mdv::core mdv::types mdv::crypto mdv::storage)

# Minimal performance test
add_executable(mdv_perf_minimal mdv_perf_minimal.c)
target_compile_options(mdv_perf_minimal PRIVATE -Wall -Wextra)
target_include_directories(mdv_perf_minimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mdv_perf_minimal PRIVATE mdv::platform mdv::core mdv::types mdv::crypto mdv::storage)
