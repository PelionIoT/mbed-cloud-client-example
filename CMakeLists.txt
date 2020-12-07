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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/platform)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/include)

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

if(ENABLE_DEVICE_SENTRY)
# MCCE device sentry example enabled, so add appropriate include files directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbed-cloud-client/device-sentry-client/source/include)
endif()

# Contain developer credential information
set(MBED_CLIENT_COMMON_CONFIG_SOURCE
  "${CMAKE_CURRENT_SOURCE_DIR}/update_default_resources.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/mbed_cloud_dev_credentials.c"
)

add_subdirectory(source/platform ${CMAKE_CURRENT_BINARY_DIR}/source/platform)
add_subdirectory(mbed-cloud-client ${CMAKE_CURRENT_BINARY_DIR}/mbed-cloud-client)

add_library(mbedClientSharedObjects STATIC ${MBED_CLIENT_COMMON_CONFIG_SOURCE})

# Needed to link sources if there is cross-dependencies between application<->library.
set (SRC_LIBS ${SRC_LIBS} PARENT_SCOPE)

CREATE_TEST_LIBRARY(mbedCloudClientExample "${MBED_CLOUD_CLIENT_EXAMPLE_SRC}" "")

target_link_libraries(mbedCloudClientExample mbedCloudClient platformCommon mbedClientSharedObjects)

