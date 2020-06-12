CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

SET(PDMC_PORT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

SET(FLASH_IMPL_SRCS
        ${PDMC_PORT_SOURCE_DIR}/pdmc-bsp/common/ExternalBlockDevice.cpp
        ${PDMC_PORT_SOURCE_DIR}/pdmc-bsp/common/FlashIAP.cpp
        ${PDMC_PORT_SOURCE_DIR}/pdmc-bsp/target/nxp/lpcxpresso54/lpc546xx_flash_api.c
        ${PDMC_PORT_SOURCE_DIR}/pdmc-bsp/target/nxp/lpcxpresso54/lpc546xx_spifi_storage.c
)

include_directories(${PDMC_PORT_SOURCE_DIR}/pdmc-bsp/common)
include_directories(${PDMC_PORT_SOURCE_DIR}/pdmc-bsp/target/nxp/lpcxpresso54)

add_library(pdmc_port STATIC ${FLASH_IMPL_SRCS})
add_dependencies(pdmc_port board)
target_link_libraries(pdmc_port board)
