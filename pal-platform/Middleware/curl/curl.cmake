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

if (NOT MBED_CLOUD_CLIENT_CURL_DYNAMIC_LINK)
    set (CMAKE_USE_OPENSSL OFF CACHE BOOL "disable openssl" FORCE)
    set (BUILD_TESTING OFF CACHE BOOL "disable testing" FORCE)
    set (BUILD_CURL_EXE OFF CACHE BOOL "don't build exe" FORCE)
    set (BUILD_SHARED_LIBS OFF CACHE BOOL "don't build share libs" FORCE)
    set (ENABLE_INET_PTON OFF CACHE BOOL "disable INET option" FORCE)
    set (CURL_ZLIB OFF CACHE BOOL "disable zlib" FORCE)
    set (HTTP_ONLY ON CACHE BOOL "set http only mode" FORCE)
    set (CMAKE_USE_LIBSSH2 OFF CACHE BOOL "disable ssh2" FORCE)
    set (CURL_DISABLE_CRYPTO_AUTH ON CACHE BOOL "disable crypto" FORCE)
    set (ENABLE_IPV6 OFF CACHE BOOL "disable ipv6" FORCE)
    set (CURL_DISABLE_MQTT ON CACHE BOOL "disable MQTT" FORCE)
    set (CURL_DISABLE_VERBOSE_STRINGS ON CACHE BOOL "disable verbose" FORCE)
    
    set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${CMAKE_SOURCE_DIR}/pal-platform/Middleware/curl/curl")
endif()