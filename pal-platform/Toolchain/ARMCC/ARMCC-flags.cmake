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

# Release/Debug common
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "-c")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "--apcs=interwork")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "--split_sections")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "--library_interface=armcc")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "--library_type=standardlib")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "--c99")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "--diag_suppress=66,177,1296,186")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "--gnu")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "-D__EVAL")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "-D__MICROLIB")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS "-DPRINTF_ADVANCED_ENABLE=1")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_C_FLAGS " --via=${CMAKE_SOURCE_DIR}/include_file.txt")



###################################################### CXX FLAGS #######################################################

# Release/Debug specific
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG" CACHE INTERNAL "cxx compiler flags release")

SET_COMPILER_DBG_RLZ_FLAG (CMAKE_CXX_FLAGS " --via=${CMAKE_SOURCE_DIR}/include_file.txt")


# Release/Debug common
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "-c")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "--cpp")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "--apcs=interwork")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "--split_sections")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "--library_interface=armcc")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "--library_type=standardlib")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "--diag_suppress=66,177,1296,186")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "--gnu")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "-D__EVAL")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "-D__MICROLIB")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "-DPRINTF_ADVANCED_ENABLE=1")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_CXX_FLAGS "-D__STDC_FORMAT_MACROS")


# Release/Debug common
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--library_type=microlib")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--nodebug")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--diag_suppress 6314,6238")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--strict")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--remove")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--summary_stderr")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--info summarysizes")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--info sizes")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--info totals")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--info unused")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--info veneers")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--map")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--xref")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--callgraph")
SET_COMPILER_DBG_RLZ_FLAG(CMAKE_EXE_LINKER_FLAGS "--symbols")
