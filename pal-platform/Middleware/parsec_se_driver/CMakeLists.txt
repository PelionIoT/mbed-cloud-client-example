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

set(BUILD_CMD cargo build)
set(WORKING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/parsec_se_driver")
set(ENV{MBEDTLS_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../mbedtls/mbedtls/include)

message ("Setting env variable MBEDTLS_INCLUDE_DIR to $ENV{MBEDTLS_INCLUDE_DIR}")

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
message ("Building Debug parsec_se_driver in ${WORKING_DIR} with ${BUILD_CMD}")
execute_process( COMMAND ${BUILD_CMD}
                 RESULT_VARIABLE CMD_ERROR
                 WORKING_DIRECTORY ${WORKING_DIR} )
else()
message ("Building Release parsec_se_driver in ${WORKING_DIR} with ${BUILD_CMD} --release")
execute_process( COMMAND ${BUILD_CMD} --release
                 RESULT_VARIABLE CMD_ERROR
                 WORKING_DIRECTORY ${WORKING_DIR} )
endif()

if(NOT ${CMD_ERROR} MATCHES "0")
MESSAGE(SEND_ERROR "BUILD_CMD STATUS:" ${CMD_ERROR})
endif()

