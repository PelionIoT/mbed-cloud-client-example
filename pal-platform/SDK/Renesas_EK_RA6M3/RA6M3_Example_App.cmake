#################################################################################
#  Copyright 2016-2020 ARM Ltd.
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

# CROSS COMPILER SETTINGS
cmake_minimum_required (VERSION 2.8)
SET(CMAKE_SYSTEM_NAME Generic)

SET(RENESAS_RA_FREERTOS_EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/Renesas_RA_FreeRTOS/example_projects/ek_ra6m3/source/ethernet/ethernet_ek_ra6m3_ep/e2studio)
SET(FREERTOS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/amazon-freertos)

# Use the application's FreeRTOS*Config.h files
#include_directories(${RENESAS_RA_FREERTOS_EXAMPLE_FOLDER}/ra_cfg/aws)

# LWIP alternative library for example
SET(FreeRTOS_Plus_SRCS
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/portable/BufferManagement/BufferAllocation_2.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_DHCP.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_ARP.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_IP.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_DNS.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_Sockets.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_Stream_Buffer.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_TCP_WIN.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_TCP_IP.c
        ${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/source/FreeRTOS_UDP_IP.c
)

# Source needed to build TCP example
SET(RENESAS_RA_EXAMPLE_SRCS
        ${RENESAS_RA_FREERTOS_EXAMPLE_FOLDER}/src/net_thread_entry.c
        ${RENESAS_RA_FREERTOS_EXAMPLE_FOLDER}/ra_gen/main.c
)

add_executable(renesasFreeRTOS.elf ${RENESAS_RA_EXAMPLE_SRCS} ${FreeRTOS_Plus_SRCS})
add_dependencies(renesasFreeRTOS.elf Board FreeRTOS)
target_link_libraries(renesasFreeRTOS.elf -Wl,--whole-archive Board FreeRTOS -Wl,--no-whole-archive)

