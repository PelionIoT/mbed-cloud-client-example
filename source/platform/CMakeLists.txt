INCLUDE(CMakeForceCompiler)
# CROSS COMPILER SETTING
cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(platformCommon)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/include)
if(${OS_BRAND} STREQUAL "FreeRTOS")
    ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/${OS_BRAND})
endif()

FILE(GLOB PLATFORM_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/${OS_BRAND}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/${OS_BRAND}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/${OS_BRAND}/*.cpp"
    )

# A source file which provides pal_plat_osGetRoTFromHW() implementation.
# By default the insecure version is used if PAL_USE_HW_ROT flag is set, so be warned.
# The compilation will also issue a warning, as will the first use of the insecure function.
# On "real" platform this needs to be replaced either here or in PAL with the platform specific one.
FILE(GLOB PLATFORM_ROT_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/pal_plat_rot_insecure.c"
    )


# Library for providing the mcc_* functions.
CREATE_LIBRARY(platformCommon "${PLATFORM_SRC}" "")

# Create a separate library from the ROT implementation. This is needed as
# the PAL's ROT module will need to be linked against some ROT implementation.
# This can be overridden either in PAL by setting PAL_USE_HW_ROT=0, which enables its own SOTP based
# ROT emulation, or by adding a platform specific pal_plat_rot.c code into PAL or by creating platformROT
# library and providing PAL_USE_PLATFORM_ROT_OVERRIDE flag to PAL's CMakeLists.txt.
CREATE_LIBRARY(platformCommonROT ${PLATFORM_ROT_SRC} "")
