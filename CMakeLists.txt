INCLUDE(CMakeForceCompiler)
# CROSS COMPILER SETTING
cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(mbedCloudClientExample)

add_definitions(-DNS_EXCLUDE_HIGHRES_TIMER)
add_definitions(-DNS_EVENTLOOP_USE_TICK_TIMER)
add_definitions(-DMBED_CONF_MBED_CLIENT_EVENT_LOOP_SIZE=8192)
if(${OS_BRAND} STREQUAL "FreeRTOS")
    add_definitions(-DMBED_CONF_MBED_CLIENT_DNS_THREAD_STACK_SIZE=2048)
else()
    add_definitions(-DMBED_CONF_MBED_CLIENT_DNS_THREAD_STACK_SIZE=102400)
endif()

# mbed-cloud-client-example
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/include)

# This is needed for compiling and linking SW.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/source/platform")

FILE(GLOB MBED_CLOUD_CLIENT_EXAMPLE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
    )

CREATE_TEST_LIBRARY(mbedCloudClientExample "${MBED_CLOUD_CLIENT_EXAMPLE_SRC}" "")

add_dependencies(mbedCloudClientExample mbedCloudClient)

ADDSUBDIRS()
