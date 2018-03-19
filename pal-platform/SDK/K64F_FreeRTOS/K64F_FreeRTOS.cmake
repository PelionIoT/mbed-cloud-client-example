#################################################################################
#  Copyright 2016, 2017 ARM Ltd.
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

SET(K64F_FREERTOS_FOLDER ${CMAKE_SOURCE_DIR}/pal-platform/SDK/K64F_FreeRTOS/K64F_FreeRTOS/)

######### Configure all Options here ##########
option(PAL_USE_CMSIS "Include CMSIS in build" OFF)
option(PAL_USE_NETWORKING "Allow networking" ON)
option(PAL_BUILD_BOARD_BRINGUP_TESTS "Build Tests" ON)
option(PAL_ENABLED "Enable PAL" ON)

#updating the autogen.cmake variables
set (MBED_CLOUD_CLIENT_OS  "FreeRTOS")
set (MBED_CLOUD_CLIENT_DEVICE  "K64F")
set (PAL_TARGET_OS FreeRTOS_8.2.3)
set (OS_BRAND FreeRTOS)
set (PAL_TARGET_DEVICE "MK64F")
set (CPU "cortex-m4")

#set (PAL_TLS_LIB "MBEDTLS")
set (TLS_LIBRARY mbedTLS)

#make mbedtls compile without its apps and tests
SET(ENABLE_PROGRAMS OFF CACHE STRING "Avoid compiling mbedtls programs" )
SET(ENABLE_TESTING OFF CACHE STRING "Avoid compiling mbedtls tests" )

set (PAL_NETWORK_LIB "lwip_1.4.1")

set (PAL_BOARD_LD_SCRIPT MK64FN1M0xxx12.ld)
SET(PAL_BOARD_SCATTER_FILE MK64FN1M0xxx12_flash.scf)

set (PAL_FATFS_LIB fatfs_0.11a)
set (PAL_SDMMC_LIB sdmmc_2.0.0)

set (NETWORK_STACK)   #dirctory name for the stack, in PAL porting folders

# Disable Building mbedtls programs & tests
SET(ENABLE_PROGRAMS OFF)
SET(ENABLE_TESTING OFF)

if(NOT MBED_APP_START)
  set(MBED_APP_START "0" CACHE STRING "" FORCE)
endif()

if(NOT MBED_APP_SIZE)
  set(MBED_APP_SIZE "0x100000" CACHE STRING "" FORCE)
endif()

message(" PROJECT BINARY DIR ${PROJECT_BINARY_DIR}")
# .h files to look for
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-DFSL_RTOS_FREE_RTOS")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-DFSL_RTOS_FREE_RTOS")
#  additional directories to look for CMakeLists.txt
include_directories("${K64F_FREERTOS_FOLDER}/OS/FreeRTOS/${PAL_TARGET_OS}/Source/include")

set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${K64F_FREERTOS_FOLDER}/OS/FreeRTOS/${PAL_TARGET_OS}")
if (${CPU} MATCHES "cortex-m4")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    	include_directories("${K64F_FREERTOS_FOLDER}/OS/FreeRTOS/${PAL_TARGET_OS}/Source/portable/GCC/ARM_CM4F")
    elseif (CMAKE_C_COMPILER_ID STREQUAL "ARMCC")
        include_directories(${K64F_FREERTOS_FOLDER}/OS/FreeRTOS/${PAL_TARGET_OS}/Source/portable/RVDS/ARM_CM4F)
        SET_COMPILER_DBG_RLZ_FLAG(CMAKE_ASM_FLAGS "--cpu Cortex-M4.fp")
        SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "--cpu Cortex-M4.fp")
        SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "--cpu Cortex-M4.fp")
        SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--cpu Cortex-M4.fp")
    endif()
endif()
#add library to build
set (PLATFORM_LIBS ${PLATFORM_LIBS} FreeRTOS)
add_definitions(-DOS_IS_FREERTOS)
add_definitions(-D__FREERTOS__)
add_definitions(-DFRDM_K64F)
add_definitions(-DFREEDOM)
add_definitions(-DCPU_MK64FN1M0VMD12)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mfpu=fpv4-sp-d16")

    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mfpu=fpv4-sp-d16")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-mfpu=fpv4-sp-d16")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mfpu=fpv4-sp-d16")

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -T${K64F_FREERTOS_FOLDER}/Device/${PAL_TARGET_DEVICE}/gcc/${PAL_BOARD_LD_SCRIPT} -static")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -T${K64F_FREERTOS_FOLDER}/Device/${PAL_TARGET_DEVICE}/gcc/${PAL_BOARD_LD_SCRIPT} -static")

    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --defsym=MBED_APP_START=${MBED_APP_START}")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --defsym=MBED_APP_SIZE=${MBED_APP_SIZE}")
elseif (CMAKE_C_COMPILER_ID STREQUAL "ARMCC")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_ASM_FLAGS "--apcs=interwork")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_ASM_FLAGS "--pd \"__MICROLIB SETA 1\"")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_ASM_FLAGS "--pd \"__EVAL SETA 1\"")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_ASM_FLAGS "--pd \"__UVISION_VERSION SETA 516\"")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_ASM_FLAGS "--pd \"MK64FN1M0xxx12 SETA 1\"")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--scatter ${K64F_FREERTOS_FOLDER}/Device/${PAL_TARGET_DEVICE}/arm/${PAL_BOARD_SCATTER_FILE}")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--predefine=\"-D__ram_vector_table__=1\"")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--predefine=\"-DMBED_APP_START=${MBED_APP_START}\"")
    SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--predefine=\"-DMBED_APP_SIZE=${MBED_APP_SIZE}\"")
endif()

# .h files to look for
#  additional directories to look for CMakeLists.txt
#if (PAL_USE_CMSIS)
	include_directories ("${K64F_FREERTOS_FOLDER}/OS/FreeRTOS/CMSIS/Include")
#endif()

include_directories(${K64F_FREERTOS_FOLDER}/Device/${PAL_TARGET_DEVICE})
include_directories(${K64F_FREERTOS_FOLDER}/Device/${PAL_TARGET_DEVICE}/utilities)
include_directories(${K64F_FREERTOS_FOLDER}/Device/${PAL_TARGET_DEVICE}/drivers)
#  additional directories to look for CMakeLists.txt

set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${K64F_FREERTOS_FOLDER}/Device/${PAL_TARGET_DEVICE}")

set (PLATFORM_LIBS ${PLATFORM_LIBS} board)

add_definitions(-DSUPPORT_RTOS_NO_CMSIS)


set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${K64F_FREERTOS_FOLDER}/Middleware/${PAL_FATFS_LIB}")
set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${K64F_FREERTOS_FOLDER}/Middleware/${PAL_SDMMC_LIB}")
set (PLATFORM_LIBS  ${PAL_FATFS_LIB} ${PAL_SDMMC_LIB} ${PLATFORM_LIBS})
set (PAL_CFSTORE_STORAGE fatfs_sd)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_FATFS_LIB}/src)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_SDMMC_LIB}/inc)


set(NETWORK_STACK LWIP)
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-DUSE_RTOS=1")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "-DUSE_RTOS=1")
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --defsym=__ram_vector_table__=1")
endif()

#  additional directories to look for CMakeLists.txt
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/port)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/port/arch)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/include)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/include/ipv4)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/include/ipv4/lwip)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/include/ipv6)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/include/ipv6/lwip)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/include/lwip)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/include/netif)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/include/posix)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/netif)
include_directories(${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}/src/netif/ppp)

set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${K64F_FREERTOS_FOLDER}/Middleware/${PAL_NETWORK_LIB}")

set (PLATFORM_LIBS lwip_1.4.1 ${PLATFORM_LIBS} )


# @@@@@@@@@@@@@@@@@ IF(CMAKE_BUILD_TYPE MATCHES Release) @@@@@@@@@@@@@@@@

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fomit-frame-pointer")
        SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fomit-frame-pointer")
    endif()

	include_directories ("${K64F_FREERTOS_FOLDER}/Middleware/mbedtls/")
	include_directories ("${K64F_FREERTOS_FOLDER}/Middleware/mbedtls/include")
	include_directories ("${K64F_FREERTOS_FOLDER}/Middleware/mbedtls/include/mbedtls")
	include_directories ("${K64F_FREERTOS_FOLDER}/Middleware/mbedtls_port/ksdk")
	include_directories ("${K64F_FREERTOS_FOLDER}/Middleware/mmcau_2.0.0")

	add_definitions(-DFREESCALE_KSDK_BM)
if (PAL_CERT_TIME_VERIFY)
	add_definitions(-DMBEDTLS_HAVE_TIME)
	add_definitions(-DMBEDTLS_PLATFORM_TIME_ALT)
endif()

	list(APPEND Additional_SRC "${K64F_FREERTOS_FOLDER}/Middleware/mbedtls_port/ksdk/ksdk_mbedtls.c")

	# Additional directories to look for CMakeLists.txt

	set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${K64F_FREERTOS_FOLDER}/Middleware/mbedtls")

	set (PLATFORM_LIBS mbedtls ${PLATFORM_LIBS})

	include_directories(./pal-platform/Middleware/mmcau_2.0.0/mmcau_2.0.0)

	set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${K64F_FREERTOS_FOLDER}/Middleware/mmcau_2.0.0")
	list (APPEND PLATFORM_LIBS mmcau_2.0.0)
    

 # endif(CMAKE_BUILD_TYPE MATCHES Release) # comment end
