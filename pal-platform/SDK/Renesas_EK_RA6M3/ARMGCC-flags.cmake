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

########### COMPILER FLAGS  ###########
#
#######################################
macro(SET_COMPILER_DBG_RLZ_FLAG flag value)
        SET(${flag}_DEBUG "${${flag}_DEBUG} ${value}")
        SET(${flag}_RELEASE "${${flag}_RELEASE} ${value}")
if (0)
 message("flag = ${flag}")
 message("value = ${value}")
 message("MY_C_FLAGS_RELEASE2 = ${CMAKE_C_FLAGS_RELEASE}")
endif(0) # comment end
endmacro(SET_COMPILER_DBG_RLZ_FLAG)

macro(SET_COMPILER_DBG_RLZ_COMMON_FLAG flag value)
        SET(${flag}_DEBUG "${${flag}_DEBUG} ${${value}_DEBUG}")
        SET(${flag}_RELEASE "${${flag}_RELEASE} ${${value}_RELEASE}")
endmacro(SET_COMPILER_DBG_RLZ_COMMON_FLAG)

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "")

OPTION(DEBUG_LOOP_START "Start with initial debug looping" OFF)
SET(CMAKE_FLAGS_EXTRA_DEBUG_LOOP "")
IF(DEBUG_LOOP_START)
    SET(CMAKE_FLAGS_EXTRA_DEBUG_LOOP "-DDEBUG_LOOP_START")
ENDIF(DEBUG_LOOP_START)

SET(CMAKE_FLAGS_COMMON_DEBUG "-DDEBUG -g -Og ${CMAKE_FLAGS_EXTRA_DEBUG_LOOP}")
SET(CMAKE_FLAGS_COMMON_RELEASE "-Og")

# The flags generated from the RA6M3 example
SET(CMAKE_COMPILE_FLAGS_BASIS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -D_RENESAS_RA_ -MMD -MP")

### Set ASM flags ###
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "${CMAKE_COMPILE_FLAGS_BASIS}")
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_ASM_FLAGS CMAKE_FLAGS_COMMON)

### Set C/C++ flags ###
message("${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "${CMAKE_COMPILE_FLAGS_BASIS} -std=gnu99")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "${CMAKE_COMPILE_FLAGS_BASIS}")
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_C_FLAGS CMAKE_FLAGS_COMMON)
SET_COMPILER_DBG_RLZ_COMMON_FLAG (CMAKE_CXX_FLAGS CMAKE_FLAGS_COMMON)

### Set linker flags ###
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker --gc-sections --specs=nosys.specs")

