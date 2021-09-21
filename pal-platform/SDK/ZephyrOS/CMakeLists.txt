cmake_minimum_required(VERSION 3.13.1)

# Define configuration files.
set(CONF_FILE "${CMAKE_SOURCE_DIR}/pal-platform/SDK/ZephyrOS/prj.conf")

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pal-platform/SDK/ZephyrOS/boards/${BOARD}.overlay")
    set(DTC_OVERLAY_FILE "${CMAKE_SOURCE_DIR}/pal-platform/SDK/ZephyrOS/boards/${BOARD}.overlay")
endif()

# Copy Kconfig file to application root for Zephyr to pick it up
configure_file(${CMAKE_SOURCE_DIR}/pal-platform/SDK/ZephyrOS/Kconfig ${CMAKE_SOURCE_DIR} COPYONLY)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project("Pelion Client")

################################################################################

add_definitions(-DMBED_CONF_APP_DEVELOPER_MODE=$<BOOL:${CONFIG_PELION_EXAMPLE_DEVELOPER_MODE}>)

add_definitions(-DMBED_CLOUD_APPLICATION_NONSTANDARD_ENTRYPOINT=1)
add_definitions(-DPAL_USER_DEFINED_CONFIGURATION="pal_config_zephyr.h")
add_definitions(-DPAL_USE_HW_TRNG=1)

include_directories(${CMAKE_SOURCE_DIR}/pal-platform/SDK/ZephyrOS)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/source)
include_directories(${CMAKE_SOURCE_DIR}/source/include)
include_directories(${CMAKE_SOURCE_DIR}/source/platform/include)

SET(APP_SRCS
    ${CMAKE_SOURCE_DIR}/main.cpp
    ${CMAKE_SOURCE_DIR}/update_ui_example.cpp
    ${CMAKE_SOURCE_DIR}/source/pdmc_example.cpp
    ${CMAKE_SOURCE_DIR}/source/application_init.cpp
    ${CMAKE_SOURCE_DIR}/source/blinky.cpp
    ${CMAKE_SOURCE_DIR}/source/certificate_enrollment_user_cb.cpp
    ${CMAKE_SOURCE_DIR}/source/platform/ZephyrOS/mcc_common_button_and_led.c
    ${CMAKE_SOURCE_DIR}/source/platform/ZephyrOS/mcc_common_setup.c
    ${CMAKE_SOURCE_DIR}/mbed_cloud_dev_credentials.c
    ${CMAKE_SOURCE_DIR}/update_default_resources.c
)

target_sources(app PRIVATE ${CMAKE_SOURCE_DIR}/pal-platform/SDK/ZephyrOS/src/main.cpp ${APP_SRCS})

if(CONFIG_PELION_SOURCES)
  target_include_directories(app PRIVATE ${ZEPHYR_BASE}/subsys/storage/flash_map)
  # Copy configuration file to include directory visible for all libraries.
  # Needed for the file to be visible by Zephyr's mbedtls library.
  configure_file("${CMAKE_SOURCE_DIR}/pal-platform/SDK/ZephyrOS/config-tls-generic.h" "${CMAKE_BINARY_DIR}/zephyr/include/generated" COPYONLY)
endif()
