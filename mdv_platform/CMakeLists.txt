cmake_minimum_required(VERSION 3.10)
project(mdv_platform)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_algorithm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_bitset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_bloom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_btree.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_hashmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_mst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/generic/mdv_vector.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_binn.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_condvar.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_def.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_ebus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_enumerator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_epoll.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_errno.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_eventfd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_file.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_filesystem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_jobber.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_lrucache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_mutex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_queuefd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_rollbacker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_router.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_safeptr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_threadpool.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_threads.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_timerfd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_topology.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_uuid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mdv_vm.c
)

use_c11()

find_package(Threads REQUIRED)

add_library(mdv_platform STATIC ${SOURCES})

add_library(mdv::platform ALIAS mdv_platform)

target_include_directories(mdv_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/generic)

target_link_libraries(mdv_platform
                      PUBLIC mdv::zf_log mdv::binn mdv::uuid4
                      PRIVATE Threads::Threads m)
