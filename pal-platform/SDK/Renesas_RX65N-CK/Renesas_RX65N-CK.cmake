#################################################################################
#  Copyright 2020 ARM Ltd.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#################################################################################

cmake_minimum_required (VERSION 3.5.1)

message("\n\tProcessing \tSDK Renesas_RX65N-CK.cmake\n")

# select OS
set (OS_BRAND Renesas_RX65N-CK)

# select PAL default config
add_definitions(-D__RENESAS_RX65N_CK__)

# select network stack
set(NETWORK_STACK WIFI-SX-ULPGN)

# select target
set(TARGET_NAME TARGET_RX65N-CK)

#updating the autogen.cmake variables
set (TLS_LIBRARY mbedTLS)

# set SDK vars
set(SDK_NAME Renesas_RX65N-CK)
set(SDK_SRCS_DIR_NAME rx65n-cloud-kit)

set(SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../pal-platform/SDK/${SDK_NAME})
set(SDK_SRCS_DIR ${SDK_DIR}/${SDK_SRCS_DIR_NAME})

set(FIT_SRCS_DIR ${SDK_SRCS_DIR}/lib/third_party/mcu_vendor/renesas/FIT/RDP_v1.15_modified)
set(BSP_APP_SRCS_DIR ${SDK_SRCS_DIR}/demos/renesas/rx65n-cloud-kit-uart-sx-ulpgn/ccrx-e2studio)

######### Configure all Options here ##########
option(STORAGE_KVSTORE "Enable KVStore" ON)

###############################################################################
### SDK Middleware/Drivers Dirs

set(FREE_RTOS_DIR           ${SDK_SRCS_DIR}/lib/FreeRTOS)
set(FREE_RTOS_COMMON_DIR    ${SDK_SRCS_DIR}/lib/third_party/mcu_vendor/renesas/amazon_freertos_common)

set(BOARD_BSP_DIR       ${FIT_SRCS_DIR}/r_bsp)
set(DRV_SCI_DIR         ${FIT_SRCS_DIR}/r_sci_rx)
set(DRV_BYTEQ_DIR       ${FIT_SRCS_DIR}/r_byteq)
set(DRV_FLASH_DIR       ${FIT_SRCS_DIR}/r_flash_rx)

include_directories(${SDK_DIR}/kvstore_flashdriver_impl)

add_definitions(-DBSP_CFG_CONFIGURATOR_SELECT=0)

# General SDK Configs
include_directories(${SDK_DIR}/config)

# FreeRTOS Headers
include_directories(${SDK_SRCS_DIR}/lib/include)
include_directories(${FREE_RTOS_COMMON_DIR})
include_directories(${SDK_SRCS_DIR}/lib/include/private)
include_directories(${SDK_SRCS_DIR}/lib/FreeRTOS/portable/GCC/RX600v2)

# Board Common Headers
include_directories(${BOARD_BSP_DIR}/mcu/all)
include_directories(${BOARD_BSP_DIR}/mcu/rx65n/register_access/gnuc)
include_directories(${BOARD_BSP_DIR})
include_directories(${BOARD_BSP_DIR}/board/generic_rx65n)
include_directories(${BOARD_BSP_DIR}/mcu/rx65n)

# BSP Example Application Headers
include_directories(${BSP_APP_SRCS_DIR}/src/smc_gen/general)
include_directories(${BSP_APP_SRCS_DIR}/src/smc_gen/r_pincfg)

# Serial Communication Interface Driver Headers
include_directories(${DRV_SCI_DIR}/src/targets/rx65n)
include_directories(${DRV_SCI_DIR})

# Byte Queue/Cycle Buffer Headers
include_directories(${DRV_BYTEQ_DIR})

# Flash Driver Headers
include_directories(${DRV_FLASH_DIR}/src/targets/rx65n)
include_directories(${DRV_FLASH_DIR})
include_directories(${DRV_FLASH_DIR}/src)
include_directories(${DRV_FLASH_DIR}/src/flash_type_4)

# Wi-Fi SX-ULPGN Dongle Headers
include_directories(${FREE_RTOS_COMMON_DIR}/network_support/uart_sx_ulpgn)

# mbedTLS Library Headers
include_directories (${SDK_SRCS_DIR}/lib/third_party/mbedtls)
include_directories (${SDK_SRCS_DIR}/lib/third_party/mbedtls/include)
include_directories (${SDK_SRCS_DIR}/lib/third_party/mbedtls/include/mbedtls)


# Linker SDK-specific Flags
if(CMAKE_COMPILER_IS_GNUCC)
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "\
    -T${SDK_DIR}/linker_script.ld \
    -static \
    -Wl,-u,_mbed_cloud_application_entrypoint \
    -Wl,-e_PowerON_Reset \
    -Wl,-M=binary.map \
    -Wl,--wrap=malloc,--wrap=free,--wrap=calloc,--wrap=realloc \
    ")
endif(CMAKE_COMPILER_IS_GNUCC)

# Add the SDK folder into build system
set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} ${SDK_DIR})
set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${SDK_DIR}/kvstore_flashdriver_impl")
