
#################################################################################
#  Copyright 2016-2018 ARM Ltd.
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

INCLUDE(CMakeForceCompiler)

#This file defines needed options for native GCC compiler.

######### Configure all Options here ##########
################################################

# TOOLCHAIN EXTENSION
IF(WIN32)
    SET(TOOLCHAIN_EXT ".exe")
ELSE()
    SET(TOOLCHAIN_EXT "")
ENDIF()

# EXECUTABLE EXTENSION
SET (CMAKE_EXECUTABLE_SUFFIX "")

# TOOLCHAIN_DIR AND NANO LIBRARY
SET(TOOLCHAIN_DIR $ENV{GCC_DIR})
STRING(REGEX REPLACE "\\\\" "/" TOOLCHAIN_DIR "${TOOLCHAIN_DIR}")

IF(NOT TOOLCHAIN_DIR)
    set(TOOLCHAIN_DIR "/usr")
ENDIF()


# TARGET_TRIPLET - none in the case of native compilation
SET(TARGET_TRIPLET "")

SET(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_DIR}/bin)
SET(TOOLCHAIN_INC_DIR ${TOOLCHAIN_DIR}/include)
SET(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_DIR}/lib)

MESSAGE(STATUS "TOOLCHAIN_DIR: " ${TOOLCHAIN_DIR})
MESSAGE(STATUS "TOOLCHAIN_BIN_DIR: " ${TOOLCHAIN_BIN_DIR})
MESSAGE(STATUS "TOOLCHAIN_INC_DIR: " ${TOOLCHAIN_INC_DIR})
MESSAGE(STATUS "TOOLCHAIN_LIB_DIR: " ${TOOLCHAIN_LIB_DIR})
SET(CMAKE_SYSTEM_NAME Generic)
#SET(CMAKE_SYSTEM_PROCESSOR arm)

CMAKE_FORCE_C_COMPILER(${TOOLCHAIN_BIN_DIR}/gcc${TOOLCHAIN_EXT} GNU)
CMAKE_FORCE_CXX_COMPILER(${TOOLCHAIN_BIN_DIR}/g++${TOOLCHAIN_EXT} GNU)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/gcc${TOOLCHAIN_EXT})

SET(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/objcopy CACHE INTERNAL "objcopy tool")
# SET(CMAKE_OBJCOPY ${TOOLCHAIN_DIR}/${TARGET_TRIPLET}/bin/objcopy CACHE INTERNAL "objcopy tool")
SET(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/objdump CACHE INTERNAL "objdump tool")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 " CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 " CACHE INTERNAL "cxx compiler flags release")   
SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE}" CACHE INTERNAL "asm compiler flags release")
SET(CMAKE_EXE_LINKER_FLAGS_RELESE "${CMAKE_EXE_LINKER_FLAGS_RELESE}" CACHE INTERNAL "linker flags release")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g" CACHE INTERNAL "c compiler flags debug")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g" CACHE INTERNAL "cxx compiler flags debug")
SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -g" CACHE INTERNAL "asm compiler flags debug")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE INTERNAL "linker flags debug")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 " CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 " CACHE INTERNAL "cxx compiler flags release")
SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE}" CACHE INTERNAL "asm compiler flags release")
SET(CMAKE_EXE_LINKER_FLAGS_RELESE "${CMAKE_EXE_LINKER_FLAGS_RELESE}" CACHE INTERNAL "linker flags release")

SET(TOOLCHAIN_FLAGS_FILE "${CMAKE_SOURCE_DIR}/../pal-platform/Toolchain/GCC/GCC-flags.cmake" CACHE INTERNAL "linker flags file")

########### DEBUG ###########
# Debug specific
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")
#########

SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/${TARGET_TRIPLET} ${EXTRA_FIND_PATH})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

MESSAGE(STATUS "BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

