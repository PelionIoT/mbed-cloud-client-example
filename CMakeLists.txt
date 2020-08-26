INCLUDE(CMakeForceCompiler)
# CROSS COMPILER SETTING
cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(mbedCloudClientExample)

add_definitions(-DNS_EXCLUDE_HIGHRES_TIMER)
add_definitions(-DNS_EVENTLOOP_USE_TICK_TIMER)
add_definitions(-DMBED_CONF_MBED_CLIENT_EVENT_LOOP_SIZE=8192)

# mbed-cloud-client-example
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/include)

# This is needed for compiling and linking SW.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/source/platform")

FILE(GLOB MBED_CLOUD_CLIENT_EXAMPLE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"

if(PARSEC_TPM_SE_SUPPORT)
    "${CMAKE_CURRENT_SOURCE_DIR}/source/platform/secure_element/mcc_se_init.c"
endif()

    )

if(PARSEC_TPM_SE_SUPPORT)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/platform/secure_element/se_configs)
endif()


CREATE_TEST_LIBRARY(mbedCloudClientExample "${MBED_CLOUD_CLIENT_EXAMPLE_SRC}" "")

add_dependencies(mbedCloudClientExample mbedCloudClient)


ADDSUBDIRS()
