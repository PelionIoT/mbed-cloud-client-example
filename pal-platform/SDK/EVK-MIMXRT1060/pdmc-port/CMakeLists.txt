CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

SET(MBED_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEVICE_FLASH")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEVICE_FLASH -std=gnu99")

SET(FLASH_IMPL_SRCS
        ${MBED_SOURCE_DIR}/pdmc-bsp/common/FlashIAP.cpp
        ${MBED_SOURCE_DIR}/pdmc-bsp/common/flash_api.c
        ${MBED_SOURCE_DIR}/pdmc-bsp/target/nxp/mimxrt1060/flexspi_nor_flash_ops.c
        ${MBED_SOURCE_DIR}/pdmc-bsp/target/nxp/mimxrt1060/rt1060_flexspi_storage.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pdmc-bsp/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pdmc-bsp/target/nxp/mimxrt1060)

add_library(pdmc_port STATIC ${FLASH_IMPL_SRCS})
add_dependencies(pdmc_port board)
target_link_libraries(pdmc_port board)
