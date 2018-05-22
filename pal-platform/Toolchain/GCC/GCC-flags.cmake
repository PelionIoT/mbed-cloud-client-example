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

macro(SET_COMPILER_DBG_RLZ_FLAG flag value)
    SET(${flag}_DEBUG "${${flag}_DEBUG} ${value}")
    SET(${flag}_RELEASE "${${flag}_RELEASE} ${value}")
#enable this if for debugging
if (0)
 message("flag = ${flag}")
 message("value = ${value}")
 message("MY_C_FLAGS_RELEASE2 = ${CMAKE_C_FLAGS_RELEASE}")
endif(0) # comment end
endmacro(SET_COMPILER_DBG_RLZ_FLAG)

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "")

########### COMPILER FLAGS  ###########
#
#######################################

########### DEBUG ###########

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-D__STARTUP_CLEAR_BSS")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mcpu=${CPU}")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-Wall")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mfloat-abi=hard")
# Floating point support

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mthumb")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-mapcs")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_ASM_FLAGS "-std=c99")

# Debug specific
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
if (PAL_GCOV_SUPPORT)
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0  -fprofile-arcs -ftest-coverage")
endif ()

# Board specific

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-Wall")
# Board specific
if (${CPU} MATCHES "x86_64")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-m32")
endif ()
if (${CPU} MATCHES "cortex-m4")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mcpu=${CPU}")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mfloat-abi=hard")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mapcs")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-mthumb")
endif()

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-MMD")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-MP")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "-std=gnu99")


########### Release specific ###########
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")
######################################

########### LINKER FLAGS  ###########
#
#
#####################################

########### DEBUG ###########
# Debug specific
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")
if (PAL_GCOV_SUPPORT)
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g -O0 -lgcov -fprofile-arcs")
endif ()

########### RELEASE ###########

if (${CPU} MATCHES "x86_64")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-m32")
elseif (${CPU} MATCHES "cortex-m4")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mcpu=${CPU}")     
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mfloat-abi=hard")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mthumb")
    SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-mapcs")
endif()

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wall")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fno-common")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-ffunction-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fdata-sections")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-ffreestanding")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-fno-builtin")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Xlinker")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "--gc-sections")

if (PAL_MEMORY_STATISTICS)
	SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wl,--wrap=malloc")
	SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wl,--wrap=free")
	SET_COMPILER_DBG_RLZ_FLAG (CMAKE_EXE_LINKER_FLAGS "-Wl,--wrap=calloc")
	add_definitions("-DPAL_MEMORY_STATISTICS")
endif()

# This is because of mbedTLS is removing all debug and release compilation flags
# but keeping the global common flags so need to set all of them... no harm is done
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} @${CMAKE_SOURCE_DIR}/include_file.txt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} @${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_C_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")
SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS "@${CMAKE_SOURCE_DIR}/include_file.txt")

MESSAGE(STATUS "BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
