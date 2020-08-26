#################################################################################
#  Copyright 2016-2020 ARM Ltd.
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

# RX65N CLOUD KIT SDK CROSS COMPILER SETTINGS

# CROSS COMPILER SETTINGS
cmake_minimum_required (VERSION 2.8)
SET(CMAKE_SYSTEM_NAME Generic)

message("\n\tProcessing \tSDK CMakeLists.txt in ${CMAKE_CURRENT_SOURCE_DIR}\n")

project(SDK_RX65N-CK_FreeRTOS)

###############################################################################
### Libraries Sources

set(FREE_RTOS_SRCS
    ${FREE_RTOS_DIR}/event_groups.c
    ${FREE_RTOS_DIR}/list.c
    ${FREE_RTOS_DIR}/queue.c
    ${FREE_RTOS_DIR}/stream_buffer.c
    ${FREE_RTOS_DIR}/tasks.c
    ${FREE_RTOS_DIR}/timers.c

    ${FREE_RTOS_DIR}/portable/GCC/RX600v2/port.c
    ${FREE_RTOS_DIR}/portable/MemMang/heap_4.c

    ${SDK_SRCS_DIR}/lib/third_party/mcu_vendor/renesas/amazon_freertos_common/freertos_start.c
    ${SDK_SRCS_DIR}/lib/third_party/mcu_vendor/renesas/amazon_freertos_common/serial_term_uart.c
)

set(BOARD_BSP_SRCS
    ${BOARD_BSP_DIR}/board/all/reset_program.S
    ${BOARD_BSP_DIR}/board/all/resetprg.c
    ${BOARD_BSP_DIR}/board/all/dbsct.c
    ${BOARD_BSP_DIR}/board/all/lowlvl.c
    ${BOARD_BSP_DIR}/board/all/lowsrc.c
    ${BOARD_BSP_DIR}/board/all/sbrk.c
    ${BOARD_BSP_DIR}/board/rx65n/vecttbl.c
    ${BOARD_BSP_DIR}/board/rx65n/clkconf.c
    ${BOARD_BSP_DIR}/board/generic_rx65n/r_bsp_hwsetup_reference.c
    # ${BOARD_BSP_DIR}/board/rskrx65n/r_bsp_hwsetup_reference.c
    # ${BOARD_BSP_DIR}/board/targetboardrx65n/r_bsp_hwsetup_reference.c
    
    ${BOARD_BSP_DIR}/mcu/all/r_bsp_common.c

    ${BOARD_BSP_DIR}/mcu/rx65n/cpu.c
    ${BOARD_BSP_DIR}/mcu/rx65n/locking.c
    ${BOARD_BSP_DIR}/mcu/rx65n/mcu_clocks.c
    ${BOARD_BSP_DIR}/mcu/rx65n/mcu_init.c
    ${BOARD_BSP_DIR}/mcu/rx65n/mcu_interrupts.c
    ${BOARD_BSP_DIR}/mcu/rx65n/mcu_locks.c
    ${BOARD_BSP_DIR}/mcu/rx65n/mcu_mapped_interrupts.c
    ${BOARD_BSP_DIR}/mcu/rx65n/mcu_startup.c

    ${BSP_APP_SRCS_DIR}/src/smc_gen/general/r_cg_hardware_setup.c
    ${BSP_APP_SRCS_DIR}/src/smc_gen/general/r_smc_cgc.c
    ${BSP_APP_SRCS_DIR}/src/smc_gen/general/r_smc_cgc_user.c

    ${SDK_DIR}/port/port.c
    ${SDK_DIR}/port/port.cpp
)

set(DRV_SCI_SRCS
    ${DRV_SCI_DIR}/src/targets/rx65n/r_sci_rx65n_data.c
    ${DRV_SCI_DIR}/src/targets/rx65n/r_sci_rx65n.c
    ${DRV_SCI_DIR}/src/r_sci_rx.c
    ${BSP_APP_SRCS_DIR}/src/smc_gen/r_pincfg/r_sci_rx_pinset.c
)

set(DRV_BYTEQ_SRCS
    ${DRV_BYTEQ_DIR}/src/r_byteq.c
)

set(DRV_FLASH_SRCS
    ${DRV_FLASH_DIR}/src/flash_type_4/r_flash_type4.c
    ${DRV_FLASH_DIR}/src/r_flash_fcu.c
    ${DRV_FLASH_DIR}/src/r_flash_group.c
    ${DRV_FLASH_DIR}/src/r_flash_rx.c
)

set(WIFI_DRV_SRCS
    ${FREE_RTOS_COMMON_DIR}/network_support/uart_sx_ulpgn/sx_ulpgn_driver.c
    ${SDK_SRCS_DIR}/lib/wifi/portable/renesas/rx65n-cloud-kit-uart-sx-ulpgn/aws_wifi.c
)

set(SDK_LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.c
)

###############################################################################
### SDK Libraries

add_library(freertos STATIC ${FREE_RTOS_SRCS})
add_library(board_bsp STATIC ${BOARD_BSP_SRCS})
add_library(drv_sci STATIC ${DRV_SCI_SRCS})
add_library(drv_byteq STATIC ${DRV_BYTEQ_SRCS})
add_library(drv_flash STATIC ${DRV_FLASH_SRCS})

add_library(drv_wifi STATIC ${WIFI_DRV_SRCS})
add_dependencies(drv_wifi freertos drv_sci drv_byteq)

add_library(PLATFORM_LIBS STATIC ${SDK_LIBS_SRCS})


###############################################################################
### mbedTLS Library
include_directories (${SDK_SRCS_DIR}/lib/third_party/mbedtls)
include_directories (${SDK_SRCS_DIR}/lib/third_party/mbedtls/include)
include_directories (${SDK_SRCS_DIR}/lib/third_party/mbedtls/include/mbedtls)

set(MBEDTLS_SOURCE_DIR ${SDK_SRCS_DIR}/lib/third_party/mbedtls/library)

set(MBEDCRYPTO_SOURCES
    ${MBEDTLS_SOURCE_DIR}/aes.c
    ${MBEDTLS_SOURCE_DIR}/aesni.c
    ${MBEDTLS_SOURCE_DIR}/arc4.c
    #${MBEDTLS_SOURCE_DIR}/aria.c
    ${MBEDTLS_SOURCE_DIR}/asn1parse.c
    ${MBEDTLS_SOURCE_DIR}/asn1write.c
    ${MBEDTLS_SOURCE_DIR}/base64.c
    ${MBEDTLS_SOURCE_DIR}/bignum.c
    ${MBEDTLS_SOURCE_DIR}/blowfish.c
    ${MBEDTLS_SOURCE_DIR}/camellia.c
    ${MBEDTLS_SOURCE_DIR}/ccm.c
    #${MBEDTLS_SOURCE_DIR}/chacha20.c
    #${MBEDTLS_SOURCE_DIR}/chachapoly.c
    ${MBEDTLS_SOURCE_DIR}/cipher.c
    ${MBEDTLS_SOURCE_DIR}/cipher_wrap.c
    ${MBEDTLS_SOURCE_DIR}/cmac.c
    ${MBEDTLS_SOURCE_DIR}/ctr_drbg.c
    ${MBEDTLS_SOURCE_DIR}/des.c
    ${MBEDTLS_SOURCE_DIR}/dhm.c
    ${MBEDTLS_SOURCE_DIR}/ecdh.c
    ${MBEDTLS_SOURCE_DIR}/ecdsa.c
    ${MBEDTLS_SOURCE_DIR}/ecjpake.c
    ${MBEDTLS_SOURCE_DIR}/ecp.c
    ${MBEDTLS_SOURCE_DIR}/ecp_curves.c
    ${MBEDTLS_SOURCE_DIR}/entropy.c
    ${MBEDTLS_SOURCE_DIR}/entropy_poll.c
    ${MBEDTLS_SOURCE_DIR}/error.c
    ${MBEDTLS_SOURCE_DIR}/gcm.c
    ${MBEDTLS_SOURCE_DIR}/havege.c
    #${MBEDTLS_SOURCE_DIR}/hkdf.c
    ${MBEDTLS_SOURCE_DIR}/hmac_drbg.c
    ${MBEDTLS_SOURCE_DIR}/md.c
    ${MBEDTLS_SOURCE_DIR}/md2.c
    ${MBEDTLS_SOURCE_DIR}/md4.c
    ${MBEDTLS_SOURCE_DIR}/md5.c
    ${MBEDTLS_SOURCE_DIR}/md_wrap.c
    ${MBEDTLS_SOURCE_DIR}/memory_buffer_alloc.c
    #${MBEDTLS_SOURCE_DIR}/nist_kw.c
    ${MBEDTLS_SOURCE_DIR}/oid.c
    ${MBEDTLS_SOURCE_DIR}/padlock.c
    ${MBEDTLS_SOURCE_DIR}/pem.c
    ${MBEDTLS_SOURCE_DIR}/pk.c
    ${MBEDTLS_SOURCE_DIR}/pk_wrap.c
    ${MBEDTLS_SOURCE_DIR}/pkcs12.c
    ${MBEDTLS_SOURCE_DIR}/pkcs5.c
    ${MBEDTLS_SOURCE_DIR}/pkparse.c
    ${MBEDTLS_SOURCE_DIR}/pkwrite.c
    ${MBEDTLS_SOURCE_DIR}/platform.c
    ${MBEDTLS_SOURCE_DIR}/platform_util.c
    #${MBEDTLS_SOURCE_DIR}/poly1305.c
    ${MBEDTLS_SOURCE_DIR}/ripemd160.c
    ${MBEDTLS_SOURCE_DIR}/rsa.c
    ${MBEDTLS_SOURCE_DIR}/rsa_internal.c
    ${MBEDTLS_SOURCE_DIR}/sha1.c
    ${MBEDTLS_SOURCE_DIR}/sha256.c
    ${MBEDTLS_SOURCE_DIR}/sha512.c
    ${MBEDTLS_SOURCE_DIR}/threading.c
    ${MBEDTLS_SOURCE_DIR}/timing.c
    ${MBEDTLS_SOURCE_DIR}/version.c
    ${MBEDTLS_SOURCE_DIR}/version_features.c
    ${MBEDTLS_SOURCE_DIR}/xtea.c
)
add_library(mbedcrypto STATIC ${MBEDCRYPTO_SOURCES})

set(MBEDTLS_SOURCE_DIR ${SDK_SRCS_DIR}/lib/third_party/mbedtls/library)

set(MBEDTLS_SOURCES
    ${MBEDTLS_SOURCE_DIR}/debug.c
    ${MBEDTLS_SOURCE_DIR}/net_sockets.c
    ${MBEDTLS_SOURCE_DIR}/ssl_cache.c
    ${MBEDTLS_SOURCE_DIR}/ssl_ciphersuites.c
    ${MBEDTLS_SOURCE_DIR}/ssl_cli.c
    ${MBEDTLS_SOURCE_DIR}/ssl_cookie.c
    ${MBEDTLS_SOURCE_DIR}/ssl_srv.c
    ${MBEDTLS_SOURCE_DIR}/ssl_ticket.c
    ${MBEDTLS_SOURCE_DIR}/ssl_tls.c
)
add_library(mbedtls STATIC ${MBEDTLS_SOURCES})

set(MBEDTLS_SOURCE_DIR ${SDK_SRCS_DIR}/lib/third_party/mbedtls/library)

set(MBEDX509_SOURCES
    ${MBEDTLS_SOURCE_DIR}/certs.c
    #${MBEDTLS_SOURCE_DIR}/pkcs11.c
    ${MBEDTLS_SOURCE_DIR}/x509.c
    ${MBEDTLS_SOURCE_DIR}/x509_create.c
    ${MBEDTLS_SOURCE_DIR}/x509_crl.c
    ${MBEDTLS_SOURCE_DIR}/x509_crt.c
    ${MBEDTLS_SOURCE_DIR}/x509_csr.c
    ${MBEDTLS_SOURCE_DIR}/x509write_crt.c
    ${MBEDTLS_SOURCE_DIR}/x509write_csr.c
)

add_library(mbedx509 STATIC ${MBEDX509_SOURCES})

###############################################################################
### Common Board Library

set(SDK_LIBS_LIST freertos board_bsp drv_sci drv_byteq drv_flash kvstore_flash_impl drv_wifi mbedcrypto mbedtls mbedx509)

add_dependencies(PLATFORM_LIBS ${SDK_LIBS_LIST})

# link all libraries into one compleate SDK library
target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${SDK_LIBS_LIST} -Wl,--no-whole-archive)
