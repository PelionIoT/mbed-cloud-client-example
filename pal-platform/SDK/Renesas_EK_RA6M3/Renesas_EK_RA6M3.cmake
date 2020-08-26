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

# CROSS COMPILER SETTINGS
cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

set (OS_BRAND Renesas_EK_RA6M3)
add_definitions(-D__RENESAS_EK_RA6M3__)

#include(${CMAKE_SOURCE_DIR}/pal-platform/common.cmake)

######### Configure all Options here ##########
option(PAL_USE_CMSIS "Include CMSIS in build" OFF)
option(PAL_USE_NETWORKING "Allow networking" OFF)
option(PAL_BUILD_BOARD_BRINGUP_TESTS "Build Tests" ON)
option(PAL_ENABLED "Enable PAL" ON)
option(STORAGE_KVSTORE "Enable KVStore" ON)

#updating the autogen.cmake variables
set (PAL_TARGET_DEVICE "EK-RA6M3")
set (MBED_CLOUD_CLIENT_DEVICE EK-RA6M3)
set (TLS_LIBRARY mbedTLS)
set (NETWORK_STACK LWIP)

#If your target SDK has mbedtls then compile without its apps and tests
SET(ENABLE_PROGRAMS OFF CACHE STRING "Avoid compiling mbedtls programs" )
SET(ENABLE_TESTING OFF CACHE STRING "Avoid compiling mbedtls tests" )

# Disable Building mbedtls programs & tests
SET(ENABLE_PROGRAMS OFF)
SET(ENABLE_TESTING OFF)

if(NOT UPDATE_LINKING)
  set(MBED_APP_START "0x0")
  set(MBED_APP_SIZE  "0x00200000")
  message("No UPDATE_LINKING defined, setting MBED_APP_START to ${MBED_APP_START} and MBED_APP_SIZE to ${MBED_APP_SIZE}")
else()
  set(MBED_APP_START "0x00010400")
  set(MBED_APP_SIZE  "0x000EFC00")
  message("UPDATE_LINKING defined, setting MBED_APP_START to ${MBED_APP_START} and MBED_APP_SIZE to ${MBED_APP_SIZE}")
endif()

SET(RENESAS_RA_FSP_FOLDER ${CMAKE_SOURCE_DIR}/pal-platform/SDK/Renesas_EK_RA6M3/fsp)
SET(FREERTOS_FOLDER ${CMAKE_SOURCE_DIR}/pal-platform/SDK/Renesas_EK_RA6M3/amazon-freertos)

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -T${CMAKE_SOURCE_DIR}/pal-platform/SDK/Renesas_EK_RA6M3/ra6m3.ld -static")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -T${CMAKE_SOURCE_DIR}/pal-platform/SDK/Renesas_EK_RA6M3/ra6m3.ld -static")

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --defsym=MBED_APP_START=${MBED_APP_START}")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --defsym=MBED_APP_SIZE=${MBED_APP_SIZE}")

# Do not call include_directories from here. Instead, use define_${target}.txt file.

#Add the SDK folder into build system
set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${CMAKE_SOURCE_DIR}/pal-platform/SDK/Renesas_EK_RA6M3")
