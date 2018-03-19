
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


set( CMAKE_SYSTEM_NAME Linux )
#set( CMAKE_C_COMPILER  $ENV{CC} )
#set( CMAKE_CXX_COMPILER  $ENV{CXX} )
string(REGEX MATCH "sysroots/([a-zA-Z0-9]+)" CMAKE_SYSTEM_PROCESSOR "$ENV{SDKTARGETSYSROOT}")
string(REGEX REPLACE "sysroots/" "" CMAKE_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
set( CMAKE_CXX_FLAGS $ENV{CXXFLAGS}  CACHE STRING "" FORCE )
set( CMAKE_C_FLAGS $ENV{CFLAGS} CACHE STRING "" FORCE ) #same flags for C sources
set( CMAKE_LDFLAGS_FLAGS ${CMAKE_CXX_FLAGS} CACHE STRING "" FORCE ) #same flags for C sources
set( CMAKE_LIBRARY_PATH ${OECORE_TARGET_SYSROOT}/usr/lib )
set( CMAKE_FIND_ROOT_PATH $ENV{OECORE_TARGET_SYSROOT} $ENV{OECORE_NATIVE_SYSROOT} )
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
set( CMAKE_INSTALL_PREFIX $ENV{OECORE_TARGET_SYSROOT}/usr CACHE STRING "" FORCE)
set( ORC_INCLUDE_DIRS $ENV{OECORE_TARGET_SYSROOT}/usr/include/orc-0.4 )
set( ORC_LIBRARY_DIRS $ENV{OECORE_TARGET_SYSROOT}/usr/lib )



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

SET(TOOLCHAIN_FLAGS_FILE "${CMAKE_SOURCE_DIR}/../pal-platform/Toolchain/POKY-GLIBC/POKY-GLIBC-flags.cmake" CACHE INTERNAL "linker flags file")
