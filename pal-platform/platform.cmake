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

include(autogen.cmake)

if ("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
    set(MBED_CLOUD_CLIENT_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/pal-platform/Toolchain/${MBED_CLOUD_CLIENT_TOOLCHAIN}/${MBED_CLOUD_CLIENT_TOOLCHAIN}.cmake)
    if(EXISTS ${MBED_CLOUD_CLIENT_TOOLCHAIN_FILE})
        set(CMAKE_TOOLCHAIN_FILE ${MBED_CLOUD_CLIENT_TOOLCHAIN_FILE})
        message(STATUS "cmake file found for Toolchain in ${MBED_CLOUD_CLIENT_TOOLCHAIN_FILE}")
    else()
            message(STATUS "No cmake file found for Toolchain in ${MBED_CLOUD_CLIENT_TOOLCHAIN_FILE}")
    endif()
else()
        message(STATUS "Toolchain file was set manualy!")
endif()
include(${CMAKE_TOOLCHAIN_FILE})

set(CMAKE_DEVICE_PATH ${CMAKE_SOURCE_DIR}/pal-platform/Device/${MBED_CLOUD_CLIENT_DEVICE}/${MBED_CLOUD_CLIENT_DEVICE}.cmake)
if(EXISTS ${CMAKE_DEVICE_PATH})
        include(${CMAKE_DEVICE_PATH})
        message(STATUS "cmake file found for Device in ${CMAKE_DEVICE_PATH}")
else()
        message(STATUS "No cmake file found for Device in ${CMAKE_DEVICE_PATH}")
endif()


set(CMAKE_OS_PATH ${CMAKE_SOURCE_DIR}/pal-platform/OS/${MBED_CLOUD_CLIENT_OS}/${MBED_CLOUD_CLIENT_OS}.cmake)
if(EXISTS ${CMAKE_OS_PATH})
        include( ${CMAKE_OS_PATH})
        message(STATUS "cmake file found for os in  ${CMAKE_OS_PATH}")
else()
        message(STATUS "No cmake file found for os in  ${CMAKE_OS_PATH}")
endif()

set(CMAKE_SDK_PATH ${CMAKE_SOURCE_DIR}/pal-platform/SDK/${MBED_CLOUD_CLIENT_SDK}/${MBED_CLOUD_CLIENT_SDK}.cmake)
if(EXISTS ${CMAKE_SDK_PATH})
        include(${CMAKE_SDK_PATH})
        message(STATUS "cmake file found for sdk in  ${CMAKE_SDK_PATH}")
else()
        message(STATUS "No cmake file found for sdk in  ${CMAKE_SDK_PATH}")
endif()

foreach(MW ${MBED_CLOUD_CLIENT_MIDDLEWARE})
set(CMAKE_MW_PATH ${CMAKE_SOURCE_DIR}/pal-platform/Middleware/${MW}/${MW}.cmake)
if(EXISTS ${CMAKE_MW_PATH})
        include(${CMAKE_MW_PATH})
        message(STATUS "cmake file found for Middleware in ${CMAKE_MW_PATH}")
else()
        message(STATUS "No cmake file found for Middleware in ${CMAKE_MW_PATH}")
endif()
endforeach(MW MBED_CLOUD_PLATFORM_MW)
