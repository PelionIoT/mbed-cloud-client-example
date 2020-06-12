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

set(CMAKE_EXECUTABLE_SUFFIX "")

macro(ELF_TO_BIN target_name target_dir)
   add_custom_command(
       TARGET ${target_name}.elf
       POST_BUILD
       COMMAND ${CMAKE_OBJCOPY} -O binary ${target_dir}/${target_name}.elf ${target_dir}/${target_name}.bin
       COMMENT "converting to .bin"
       VERBATIM
   )
endmacro(ELF_TO_BIN)

include(${PROJECT_BINARY_DIR}/autogen.cmake)
include(${CMAKE_SOURCE_DIR}/pal-platform/common.cmake)

########### COMPILER FLAGS  ###########
#
#######################################

########### Common ASM Flags###########
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mcpu=cortex-m4")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-Wall")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mfloat-abi=hard")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-D__STARTUP_CLEAR_BSS")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mthumb")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mapcs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-std=gnu99")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mfpu=fpv4-sp-d16")

########### Debug specific ###########
SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")
SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -g")

########### Release specific ###########
SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} -DNDEBUG")

message("${PROJECT_BINARY_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "@${PROJECT_BINARY_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "@${PROJECT_BINARY_DIR}/include_file.txt")

########### Common C Flags ###########
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mcpu=cortex-m4")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-Wall")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mfloat-abi=hard")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fomit-frame-pointer")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-D__STARTUP_CLEAR_BSS")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mthumb")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-MMD")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-MP")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mapcs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-std=gnu99")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-D_GNU_SOURCE")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mfpu=fpv4-sp-d16")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-DFSL_RTOS_FREE_RTOS")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-DUSE_RTOS=1")

########### Debug specific ###########
string(REPLACE "-DDEBUG" "" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
string(REPLACE "-g" "" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
string(REPLACE "-O1" "" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1")

########### Release specific ###########
string(REPLACE "-DNDEBUG" "" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
string(REPLACE "-Os" "" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")

########### Common CXX Flags ###########
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-mcpu=cortex-m4")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-Wall")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-mfloat-abi=hard")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-D__STARTUP_CLEAR_BSS")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-mthumb")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-MMD")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-MP")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-mapcs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-std=gnu++11")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-fno-rtti")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-D_GNU_SOURCE")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-mfpu=fpv4-sp-d16")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-DFSL_RTOS_FREE_RTOS")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-DUSE_RTOS=1")

########### Debug specific ###########
string(REPLACE "-DDEBUG" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
string(REPLACE "-g" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
string(REPLACE "-O1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1")

########### Release specific ###########
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
string(REPLACE "-Os" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")

########### LINKER FLAGS  ###########
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m4")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wall")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mfloat-abi=hard")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mthumb")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mapcs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "--gc-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-static")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-z")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "muldefs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mfpu=fpv4-sp-d16")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --defsym=__ram_vector_table__=1")
#SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker -Map=output.map")

########### Debug specific ###########
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")

if (PAL_MEMORY_STATISTICS) #currently working only in gcc based compilers
        SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wl,--wrap=malloc")
        SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wl,--wrap=free")
        SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wl,--wrap=calloc")
        add_definitions("-DPAL_MEMORY_STATISTICS")
endif()

if ("${MBED_CLOUD_CLIENT_NATIVE_SDK}" STREQUAL "True")
    SET(APP_ROOT_FOLDER ${CMAKE_SOURCE_DIR})
    SET(LPC54_NXP_FOLDER ${CMAKE_SOURCE_DIR}/../../../../)
    SET(LPC54_MW_ARM_PELION_ROOT_FOLDER ${CMAKE_SOURCE_DIR}/../../../../middleware/arm-pelion)
    SET(LPC54_MW_ARM_PELION_MCC_FOLDER ${LPC54_MW_ARM_PELION_ROOT_FOLDER}/mbed-cloud-client)
    SET(LPC54_MW_ARM_PELION_FLASH_FOLDER ${LPC54_MW_ARM_PELION_ROOT_FOLDER}/pdmc-port)
else()
    SET(APP_ROOT_FOLDER ${CMAKE_SOURCE_DIR}/pal-platform/SDK/LPCXpresso54628)
    SET(LPC54_NXP_FOLDER ${CMAKE_SOURCE_DIR}/pal-platform/SDK/LPCXpresso54628/SDK_LPCXpresso54628)
    SET(LPC54_MW_ARM_PELION_MCC_FOLDER ${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB})
    SET(LPC54_MW_ARM_PELION_FLASH_FOLDER ${APP_ROOT_FOLDER}/pdmc-port)
    message(STATUS "APP_ROOT_FOLDER = ${APP_ROOT_FOLDER}")
    message(STATUS "LPC54_NXP_FOLDER = ${LPC54_NXP_FOLDER}")
    message(STATUS "LPC54_MW_ARM_PELION_MCC_FOLDER = ${LPC54_MW_ARM_PELION_MCC_FOLDER}")
    message(STATUS "LPC54_MW_ARM_PELION_FLASH_FOLDER = ${LPC54_MW_ARM_PELION_FLASH_FOLDER}")
endif()

######### Configure all Options here ##########
option(PAL_USE_CMSIS "Include CMSIS in build" OFF)
option(PAL_USE_NETWORKING "Allow networking" ON)
option(PAL_BUILD_BOARD_BRINGUP_TESTS "Build Tests" ON)
option(PAL_ENABLED "Enable PAL" ON)

#updating the autogen.cmake variables
#set (MBED_CLOUD_CLIENT_OS  "FreeRTOS")
#set (MBED_CLOUD_CLIENT_DEVICE  "LPC54628")
set (PAL_TARGET_OS "amazon-freertos")
set (PAL_TARGET_DEVICE "LPC54628")
set (CPU "cortex-m4")

#set (PAL_TLS_LIB "MBEDTLS")
set (TLS_LIBRARY mbedTLS)
set (NETWORK_STACK LWIP)

#make mbedtls compile without its apps and tests
SET(ENABLE_PROGRAMS OFF CACHE STRING "Avoid compiling mbedtls programs" )
SET(ENABLE_TESTING OFF CACHE STRING "Avoid compiling mbedtls tests" )

set (PAL_NETWORK_LIB "lwip")
set (PAL_BOARD_LD_SCRIPT LPC54628J512_flash.ld)

# Disable Building mbedtls programs & tests
SET(ENABLE_PROGRAMS OFF)
SET(ENABLE_TESTING OFF)

if(NOT UPDATE_LINKING)
  set(MBED_APP_START "0x0")
  set(MBED_APP_SIZE "0x80000")
  message("No UPDATE_LINKING defined, setting MBED_APP_START to ${MBED_APP_START} and MBED_APP_SIZE to ${MBED_APP_SIZE}")
else()
  set(MBED_APP_START "0x8400")
  set(MBED_APP_SIZE "0x67C00")
  message("UPDATE_LINKING defined, setting MBED_APP_START to ${MBED_APP_START} and MBED_APP_SIZE to ${MBED_APP_SIZE}")
endif()

message(" PROJECT BINARY DIR ${PROJECT_BINARY_DIR}")

add_definitions(-D__NXP_FREERTOS__)
add_definitions(-DFREEDOM)
add_definitions(-DCPU_LPC54628J512ET180)
add_definitions(-DUSE_RTOS=1)
add_definitions(-DLWIP_DNS=1)
add_definitions(-DLWIP_DHCP=1)
add_definitions(-DLWIP_NETIF_STATUS_CALLBACK=1)
add_definitions(-DLWIP_NETIF_LINK_CALLBACK=1)
add_definitions(-DLWIP_TIMEVAL_PRIVATE=0)
add_definitions(-DFSL_RTOS_FREE_RTOS)
add_definitions(-DLWIP_IPV6=1)
add_definitions(-DLWIP_SO_SNDTIMEO=1)
add_definitions(-DSDK_DEBUGCONSOLE_UART=1)
add_definitions(-DMBED_CLOUD_APPLICATION_NONSTANDARD_ENTRYPOINT)

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSERIAL_PORT_TYPE_UART=1")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DSERIAL_PORT_TYPE_UART=1")

#  additional directories to look for CMakeLists.txt
include_directories(${APP_ROOT_FOLDER})
include_directories(${APP_ROOT_FOLDER}/pelion_enet)
include_directories(${LPC54_NXP_FOLDER})
include_directories(${LPC54_NXP_FOLDER}/boards/lpcxpresso54628/project_template)
include_directories(${LPC54_NXP_FOLDER}/devices)
include_directories(${LPC54_NXP_FOLDER}/devices/LPC54628)
include_directories(${LPC54_NXP_FOLDER}/devices/LPC54628/drivers)
include_directories(${LPC54_NXP_FOLDER}/devices/LPC54628/utilities/debug_console)
include_directories(${LPC54_NXP_FOLDER}/devices/LPC54628/utilities/str)
include_directories(${LPC54_NXP_FOLDER}/CMSIS)
include_directories(${LPC54_NXP_FOLDER}/CMSIS/Include)
include_directories(${LPC54_NXP_FOLDER}/components/serial_manager)
include_directories(${LPC54_NXP_FOLDER}/components/uart)
include_directories(${LPC54_NXP_FOLDER}/rtos)
include_directories(${LPC54_NXP_FOLDER}/rtos/amazon-freertos)
include_directories(${LPC54_NXP_FOLDER}/rtos/amazon-freertos/freertos_kernel)
include_directories(${LPC54_NXP_FOLDER}/rtos/amazon-freertos/freertos_kernel/include)
include_directories(${LPC54_NXP_FOLDER}/rtos/amazon-freertos/freertos_kernel/portable)
include_directories(${LPC54_NXP_FOLDER}/rtos/amazon-freertos/freertos_kernel/portable/GCC/ARM_CM4F)
include_directories(${LPC54_NXP_FOLDER}/middleware/pdmc-port)
include_directories(${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB})
include_directories(${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB}/port)
include_directories(${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB}/src)
include_directories(${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB}/src/include)
include_directories(${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB}/src/include/compat)
include_directories(${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB}/src/include/lwip)
include_directories(${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB}/src/include/netif)
include_directories(${LPC54_NXP_FOLDER}/middleware/mbedtls)
include_directories(${LPC54_NXP_FOLDER}/middleware/mbedtls/include)
include_directories(${LPC54_NXP_FOLDER}/middleware/mbedtls/include/mbedtls)
include_directories(${LPC54_NXP_FOLDER}/middleware/mbedtls/include/mbedtls/port/ksdk)
include_directories(${LPC54_NXP_FOLDER}/middleware/mbedtls/port/ksdk)
include_directories(${LPC54_NXP_FOLDER}/middleware/mmcau)


#  additional directories to look for CMakeLists.txt
if ("${MBED_CLOUD_CLIENT_NATIVE_SDK}" STREQUAL "False")
    set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${APP_ROOT_FOLDER}")
endif()

# Workaround for lwip cmake bug is in CMakeLists.txt. Uncomment the following line if workaround is no longer needed.
#set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB}")

set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${LPC54_MW_ARM_PELION_FLASH_FOLDER}")
if ("${MBED_CLOUD_CLIENT_NATIVE_SDK}" STREQUAL "True")
    set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${LPC54_MW_ARM_PELION_MCC_FOLDER}")
endif()

#add library to build
set (PLATFORM_LIBS ${PLATFORM_LIBS} NXP_FreeRTOS)
list (APPEND ${PLATFORM_LIBS} lwipcore)
list (APPEND ${PLATFORM_LIBS} lwip_port)
list (APPEND ${PLATFORM_LIBS} board)
list (APPEND ${PLATFORM_LIBS} mbedtls)
list (APPEND ${PLATFORM_LIBS} mbedx509)
list (APPEND ${PLATFORM_LIBS} mbedcrypto)
list (APPEND ${PLATFORM_LIBS} ksdk)
list (APPEND ${PLATFORM_LIBS} mmcau)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -T${APP_ROOT_FOLDER}/generated/linkscripts/${PAL_BOARD_LD_SCRIPT} -static")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -T${APP_ROOT_FOLDER}/generated/linkscripts/${PAL_BOARD_LD_SCRIPT} -static")

    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --defsym=MBED_APP_START=${MBED_APP_START}")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --defsym=MBED_APP_SIZE=${MBED_APP_SIZE}")

endif()

if (PAL_CERT_TIME_VERIFY)
        add_definitions(-DMBEDTLS_HAVE_TIME)
        add_definitions(-DMBEDTLS_PLATFORM_TIME_ALT)
endif()
