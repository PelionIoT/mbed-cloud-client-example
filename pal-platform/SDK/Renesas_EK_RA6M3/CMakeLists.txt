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

# CROSS COMPILER SETTINGS
cmake_minimum_required (VERSION 2.8)
SET(CMAKE_SYSTEM_NAME Generic)

project(renesasFreeRTOS)

SET(PDMC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
SET(E2STUDIO_GEN_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/e2studio_gen)
SET(FSP_EXAMPLES_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/fsp_examples)
SET(FSP_FREERTOS_EXAMPLE_SRC_FOLDER ${FSP_EXAMPLES_FOLDER}//example_projects/ek_ra6m3/source/freertos/freertos_ek_ra6m3_ep/e2studio/src)
SET(PORTING_SRC_FOLDER ${PDMC_FOLDER}/source/platform/Renesas_EK_RA6M3)
#SET(LWIP_DIR ${FREERTOS_FOLDER}/libraries/3rdparty/lwip)
SET(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lwip)
# The LWIP is using "arch/cc.h" and it's only in lwip_osal
SET(LWIP_PORT_SOURCE_DIR ${FREERTOS_FOLDER}/libraries/3rdparty/lwip_osal/src)
set(MBEDTLS_SOURCE_DIR ${FREERTOS_FOLDER}/libraries/3rdparty/mbedtls/library)
set(FLASH_DRIVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pdmc-bsp)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/rm_freertos_port)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_5/CMSIS/Core/Include)
ADD_GLOBALDIR(${RENESAS_RA_FSP_FOLDER}/ra/board/ra6m3_ek)
ADD_GLOBALDIR(${RENESAS_RA_FSP_FOLDER}/ra/fsp/inc)
ADD_GLOBALDIR(${RENESAS_RA_FSP_FOLDER}/ra/fsp/inc/api)
ADD_GLOBALDIR(${RENESAS_RA_FSP_FOLDER}/ra/fsp/inc/instances)
ADD_GLOBALDIR(${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/rm_freertos_port)
#ADD_GLOBALDIR(${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/rm_freertos_plus_tcp)
ADD_GLOBALDIR(${E2STUDIO_GEN_FOLDER}/ra_cfg/fsp_cfg)
# Use the source/platform/Renesas_FreeRTOS_build_test/FreeRTOS*Config.h files instead.
#ADD_GLOBALDIR(${E2STUDIO_GEN_FOLDER}/ra_cfg/aws)
#ADD_GLOBALDIR(${E2STUDIO_GEN_FOLDER}/ra_cfg/fsp_cfg/bsp)
ADD_GLOBALDIR(${E2STUDIO_GEN_FOLDER}/ra_gen)
ADD_GLOBALDIR(${FREERTOS_FOLDER}/freertos_kernel/include)
#ADD_GLOBALDIR(${FREERTOS_FOLDER}/libraries/freertos_plus/standard/freertos_plus_tcp/include)
ADD_GLOBALDIR(${FREERTOS_FOLDER}/libraries/3rdparty/lwip/src/include)
ADD_GLOBALDIR(${FREERTOS_FOLDER}/libraries/3rdparty/lwip/src/include/lwip)
ADD_GLOBALDIR(${FREERTOS_FOLDER}/libraries/3rdparty/lwip_osal/include)
ADD_GLOBALDIR(${FREERTOS_FOLDER}/libraries/3rdparty/mbedtls/include/)
ADD_GLOBALDIR(${FREERTOS_FOLDER}/libraries/3rdparty/mbedtls/include/mbedtls)
ADD_GLOBALDIR(${FREERTOS_FOLDER}/libraries/abstractions/pkcs11/mbedtls)
ADD_GLOBALDIR(${PDMC_FOLDER}/mbed-cloud-client/mbed-client-pal/Source/Port/Reference-Impl/OS_Specific/Renesas_EK_RA6M3/Board_Specific/TARGET_EK-RA6M3)
ADD_GLOBALDIR(${LWIP_DIR}/src/include)
ADD_GLOBALDIR(${FSP_FREERTOS_EXAMPLE_SRC_FOLDER}/SEGGER_RTT)
include_directories(${FLASH_DRIVER_DIR}/common)


# Do not call include_directories from here. Instead, use define_${target}.txt file.
SET(Board_SRCS
        # Board specifics
        #${RENESAS_RA_FREERTOS_EXAMPLE_FOLDER}/script/ra6m3.ld
        ${FSP_FREERTOS_EXAMPLE_SRC_FOLDER}/hal_entry.c
        #${RENESAS_RA_FREERTOS_EXAMPLE_FOLDER}/src/net_thread_entry.c
        #${RENESAS_RA_FREERTOS_EXAMPLE_FOLDER}/ra_gen/main.c
        ${E2STUDIO_GEN_FOLDER}/ra_gen/common_data.c
        ${E2STUDIO_GEN_FOLDER}/ra_gen/net_thread.c
        ${E2STUDIO_GEN_FOLDER}/ra_gen/hal_data.c
        ${E2STUDIO_GEN_FOLDER}/ra_gen/pin_data.c
        ${E2STUDIO_GEN_FOLDER}/ra_gen/vector_data.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c
        ${RENESAS_RA_FSP_FOLDER}/ra/board/ra6m3_ek/board_init.c
        ${RENESAS_RA_FSP_FOLDER}/ra/board/ra6m3_ek/board_leds.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_clocks.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_common.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_delay.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_group_irq.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_io.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_irq.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_register_protection.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_rom_registers.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/bsp/mcu/all/bsp_sbrk.c
        # Ethernet
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/r_ether_phy/r_ether_phy.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/r_ether/r_ether.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/r_flash_hp/r_flash_hp.c
        ${E2STUDIO_GEN_FOLDER}/ra_gen/common_data.c
        # Not using FreeRTOS Plus but using LWIP.
        #${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/rm_freertos_plus_tcp/NetworkInterface.c
        # Other port code
        ${CMAKE_CURRENT_SOURCE_DIR}/rm_freertos_port/port.c
        ${RENESAS_RA_FSP_FOLDER}/ra/fsp/src/r_ioport/r_ioport.c
        ${CMAKE_CURRENT_SOURCE_DIR}/port/port.c
        ${CMAKE_CURRENT_SOURCE_DIR}/port/port.cpp
        # Print
        ${FSP_FREERTOS_EXAMPLE_SRC_FOLDER}/SEGGER_RTT/SEGGER_RTT.c
        ${FSP_FREERTOS_EXAMPLE_SRC_FOLDER}/SEGGER_RTT/SEGGER_RTT_printf.c
)

SET(FreeRTOS_SRCS
        # We want small FreeRTOS heap scheme without linker heap size determination.
        # Also FreeRTOS heap function has a few FreeRTOS specific utils.
        ${FREERTOS_FOLDER}/freertos_kernel/portable/MemMang/heap_4.c
        ${FREERTOS_FOLDER}/freertos_kernel/event_groups.c
        ${FREERTOS_FOLDER}/freertos_kernel/list.c
        ${FREERTOS_FOLDER}/freertos_kernel/queue.c
        ${FREERTOS_FOLDER}/freertos_kernel/stream_buffer.c
        ${FREERTOS_FOLDER}/freertos_kernel/tasks.c
        ${FREERTOS_FOLDER}/freertos_kernel/timers.c
)

set(lwipcore_SRCS
    ${LWIP_DIR}/src/core/init.c
    ${LWIP_DIR}/src/core/def.c
    ${LWIP_DIR}/src/core/dns.c
    ${LWIP_DIR}/src/core/inet_chksum.c
    ${LWIP_DIR}/src/core/ip.c
    ${LWIP_DIR}/src/core/mem.c
    ${LWIP_DIR}/src/core/memp.c
    ${LWIP_DIR}/src/core/netif.c
    ${LWIP_DIR}/src/core/pbuf.c
    ${LWIP_DIR}/src/core/raw.c
    ${LWIP_DIR}/src/core/stats.c
    ${LWIP_DIR}/src/core/sys.c
    ${LWIP_DIR}/src/core/altcp.c
    ${LWIP_DIR}/src/core/altcp_alloc.c
    ${LWIP_DIR}/src/core/altcp_tcp.c
    ${LWIP_DIR}/src/core/tcp.c
    ${LWIP_DIR}/src/core/tcp_in.c
    ${LWIP_DIR}/src/core/tcp_out.c
    ${LWIP_DIR}/src/core/timeouts.c
    ${LWIP_DIR}/src/core/udp.c
    # FYI. This hook uses PolarSSL (lwipppp_Polar_SRCS).
    ${LWIP_DIR}/contrib/addons/tcp_isn/tcp_isn.c
)
set(lwipcore4_SRCS
    ${LWIP_DIR}/src/core/ipv4/acd.c
    ${LWIP_DIR}/src/core/ipv4/autoip.c
    ${LWIP_DIR}/src/core/ipv4/dhcp.c
    ${LWIP_DIR}/src/core/ipv4/etharp.c
    ${LWIP_DIR}/src/core/ipv4/icmp.c
    ${LWIP_DIR}/src/core/ipv4/igmp.c
    ${LWIP_DIR}/src/core/ipv4/ip4_frag.c
    ${LWIP_DIR}/src/core/ipv4/ip4.c
    ${LWIP_DIR}/src/core/ipv4/ip4_addr.c
)
set(lwipcore6_SRCS
    ${LWIP_DIR}/src/core/ipv6/dhcp6.c
    ${LWIP_DIR}/src/core/ipv6/ethip6.c
    ${LWIP_DIR}/src/core/ipv6/icmp6.c
    ${LWIP_DIR}/src/core/ipv6/inet6.c
    ${LWIP_DIR}/src/core/ipv6/ip6.c
    ${LWIP_DIR}/src/core/ipv6/ip6_addr.c
    ${LWIP_DIR}/src/core/ipv6/ip6_frag.c
    ${LWIP_DIR}/src/core/ipv6/mld6.c
    ${LWIP_DIR}/src/core/ipv6/nd6.c
)

# APIFILES: The files which implement the sequential and socket APIs.
set(lwipapi_SRCS
    ${LWIP_DIR}/src/api/api_lib.c
    ${LWIP_DIR}/src/api/api_msg.c
    ${LWIP_DIR}/src/api/err.c
    ${LWIP_DIR}/src/api/if_api.c
    ${LWIP_DIR}/src/api/netbuf.c
    ${LWIP_DIR}/src/api/netdb.c
    ${LWIP_DIR}/src/api/netifapi.c
    ${LWIP_DIR}/src/api/sockets.c
    ${LWIP_DIR}/src/api/tcpip.c
)

# Files implementing various generic network interface functions
set(lwipnetif_SRCS
    ${LWIP_DIR}/src/netif/ethernet.c
    ${LWIP_DIR}/src/netif/bridgeif.c
    ${LWIP_DIR}/src/netif/bridgeif_fdb.c
    ${LWIP_DIR}/src/netif/slipif.c
)

# 6LoWPAN
set(lwipsixlowpan_SRCS
    ${LWIP_DIR}/src/netif/lowpan6_common.c
    ${LWIP_DIR}/src/netif/lowpan6.c
    ${LWIP_DIR}/src/netif/lowpan6_ble.c
    ${LWIP_DIR}/src/netif/zepif.c
)

# PPP
set(lwipppp_Core_SRCS
    ${LWIP_DIR}/src/netif/ppp/auth.c
    ${LWIP_DIR}/src/netif/ppp/ccp.c
    ${LWIP_DIR}/src/netif/ppp/chap-md5.c
    ${LWIP_DIR}/src/netif/ppp/chap_ms.c
    ${LWIP_DIR}/src/netif/ppp/chap-new.c
    ${LWIP_DIR}/src/netif/ppp/demand.c
    ${LWIP_DIR}/src/netif/ppp/eap.c
    ${LWIP_DIR}/src/netif/ppp/ecp.c
    ${LWIP_DIR}/src/netif/ppp/eui64.c
    ${LWIP_DIR}/src/netif/ppp/fsm.c
    ${LWIP_DIR}/src/netif/ppp/ipcp.c
    ${LWIP_DIR}/src/netif/ppp/ipv6cp.c
    ${LWIP_DIR}/src/netif/ppp/lcp.c
    ${LWIP_DIR}/src/netif/ppp/magic.c
    ${LWIP_DIR}/src/netif/ppp/mppe.c
    ${LWIP_DIR}/src/netif/ppp/multilink.c
    ${LWIP_DIR}/src/netif/ppp/ppp.c
    ${LWIP_DIR}/src/netif/ppp/pppapi.c
    ${LWIP_DIR}/src/netif/ppp/pppcrypt.c
    ${LWIP_DIR}/src/netif/ppp/pppoe.c
    ${LWIP_DIR}/src/netif/ppp/pppol2tp.c
    ${LWIP_DIR}/src/netif/ppp/pppos.c
    ${LWIP_DIR}/src/netif/ppp/upap.c
    ${LWIP_DIR}/src/netif/ppp/utils.c
    ${LWIP_DIR}/src/netif/ppp/vj.c
)

set(lwipppp_Polar_SRCS
    ${LWIP_DIR}/src/netif/ppp/polarssl/arc4.c
    ${LWIP_DIR}/src/netif/ppp/polarssl/des.c
    ${LWIP_DIR}/src/netif/ppp/polarssl/md4.c
    ${LWIP_DIR}/src/netif/ppp/polarssl/md5.c
    ${LWIP_DIR}/src/netif/ppp/polarssl/sha1.c
)

# SNMPv3 agent
set(lwipsnmp_SRCS
    ${LWIP_DIR}/src/apps/snmp/snmp_asn1.c
    ${LWIP_DIR}/src/apps/snmp/snmp_core.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_icmp.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_interfaces.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_ip.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_snmp.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_system.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_tcp.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_udp.c
    ${LWIP_DIR}/src/apps/snmp/snmp_snmpv2_framework.c
    ${LWIP_DIR}/src/apps/snmp/snmp_snmpv2_usm.c
    ${LWIP_DIR}/src/apps/snmp/snmp_msg.c
    ${LWIP_DIR}/src/apps/snmp/snmpv3.c
    ${LWIP_DIR}/src/apps/snmp/snmp_netconn.c
    ${LWIP_DIR}/src/apps/snmp/snmp_pbuf_stream.c
    ${LWIP_DIR}/src/apps/snmp/snmp_raw.c
    ${LWIP_DIR}/src/apps/snmp/snmp_scalar.c
    ${LWIP_DIR}/src/apps/snmp/snmp_table.c
    ${LWIP_DIR}/src/apps/snmp/snmp_threadsync.c
    ${LWIP_DIR}/src/apps/snmp/snmp_traps.c
)

# HTTP server + client
set(lwiphttp_SRCS
    ${LWIP_DIR}/src/apps/http/altcp_proxyconnect.c
    ${LWIP_DIR}/src/apps/http/fs.c
    ${LWIP_DIR}/src/apps/http/http_client.c
    ${LWIP_DIR}/src/apps/http/httpd.c
)

# MAKEFSDATA HTTP server host utility
set(lwipmakefsdata_SRCS
    ${LWIP_DIR}/src/apps/http/makefsdata/makefsdata.c
)

# IPERF server
set(lwipiperf_SRCS
    ${LWIP_DIR}/src/apps/lwiperf/lwiperf.c
)

# SMTP client
set(lwipsmtp_SRCS
    ${LWIP_DIR}/src/apps/smtp/smtp.c
)

# SNTP client
set(lwipsntp_SRCS
    ${LWIP_DIR}/src/apps/sntp/sntp.c
)

# MDNS responder
set(lwipmdns_SRCS
    ${LWIP_DIR}/src/apps/mdns/mdns.c
)

# NetBIOS name server
set(lwipnetbios_SRCS
    ${LWIP_DIR}/src/apps/netbiosns/netbiosns.c
)

# TFTP server files
set(lwiptftp_SRCS
    ${LWIP_DIR}/src/apps/tftp/tftp_server.c
)

# MQTT client files
set(lwipmqtt_SRCS
    ${LWIP_DIR}/src/apps/mqtt/mqtt.c
)

# ARM MBEDTLS related files of lwIP rep
set(lwipmbedtls_SRCS
    ${LWIP_DIR}/src/apps/altcp_tls/altcp_tls_mbedtls.c
    ${LWIP_DIR}/src/apps/altcp_tls/altcp_tls_mbedtls_mem.c
    ${LWIP_DIR}/src/apps/snmp/snmpv3_mbedtls.c
)

# All LWIP files without apps
set(lwipnoapps_SRCS
    ${lwipcore_SRCS}
    ${lwipcore4_SRCS}
    ${lwipcore6_SRCS}
    ${lwipapi_SRCS}
    ${lwipnetif_SRCS}
    ${lwipsixlowpan_SRCS}
    ${lwipppp_Core_SRCS}
    ${lwipppp_Polar_SRCS}
)

SET(LWIP_PORT_SRCS
    ${PORTING_SRC_FOLDER}/lwip/src/netif/ethernetif.c
    ${LWIP_PORT_SOURCE_DIR}/sys_arch.c
    # This is using old deprecated API. Don't use it for now.
    #${LWIP_DIR}/contrib/ports/freertos/sys_arch.c
)

# Some files are commented out due to not available in the current AWS FreeRTOS repo.
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

# Some files are commented out due to not available in the current AWS FreeRTOS repo.
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

# Flash IAP driver
set(FLASH_DRIVER_SOURCES
    ${FLASH_DRIVER_DIR}/common/FlashIAP.cpp
    ${FLASH_DRIVER_DIR}/common/flash_api.c
    ${FLASH_DRIVER_DIR}/target/renesas/ra6m3/ra6m3_storage.c
)

SET(PLATFORM_LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.c
)

add_library(Board STATIC ${Board_SRCS})
add_library(FreeRTOS STATIC ${FreeRTOS_SRCS})
add_dependencies(FreeRTOS Board)

add_library(lwipcore STATIC ${lwipnoapps_SRCS})
add_library(lwipport STATIC ${LWIP_PORT_SRCS})
add_library(mbedcrypto STATIC ${MBEDCRYPTO_SOURCES})
add_library(mbedtls STATIC ${MBEDTLS_SOURCES})
add_library(mbedx509 STATIC ${MBEDX509_SOURCES})
add_library(kvstore_flash_impl STATIC ${FLASH_DRIVER_SOURCES})
add_library(PLATFORM_LIBS STATIC ${PLATFORM_LIBS_SRCS})

# Make sure dependent libraries are built before this target.
SET(PLATFORM_LIBS_list Board FreeRTOS lwipcore lwipport kvstore_flash_impl mbedcrypto mbedtls mbedx509)
add_dependencies(PLATFORM_LIBS ${PLATFORM_LIBS_list})
# This is the one-for-all library.
# Please note that it's important to put '--whole-archive' scope in order to resolve
# BSP's weak/strong definitions within a static library.
target_link_libraries(PLATFORM_LIBS -Wl,--whole-archive ${PLATFORM_LIBS_list} -Wl,--no-whole-archive)

# RA6M3 TCP/Ethernet Example Build
#include(${CMAKE_CURRENT_SOURCE_DIR}/RA6M3_Example_App.cmake)
