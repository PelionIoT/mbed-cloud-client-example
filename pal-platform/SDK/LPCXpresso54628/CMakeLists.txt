# CROSS COMPILER SETTINGS
cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)
set(DEVICE_NAME "LPC54628")

include(${PROJECT_BINARY_DIR}/autogen.cmake)
if ("${MBED_CLOUD_CLIENT_NATIVE_SDK}" STREQUAL "True")
    SET(SDK_FOLDER ${CMAKE_SOURCE_DIR}/../../../../../)
else()
    SET(SDK_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/SDK_LPCXpresso54628)
endif()

# Include CMSIS
include_directories (${SDK_FOLDER}/CMSIS/Include)

#Include FreeRTOS
include_directories(${SDK_FOLDER}/rtos/amazon-freertos/freertos_kernel/include)
include_directories(${SDK_FOLDER}/rtos/amazon-freertos/freertos_kernel/portable/GCC/ARM_CM4F)

# Include for LWIP Ethernet port
include_directories(${SDK_FOLDER}/middleware/lwip/port)
include_directories(${SDK_FOLDER}/middleware/lwip/port/arch)
# board support
include_directories(${SDK_FOLDER}/components)
include_directories(${SDK_FOLDER}/components/phylan8720a)

include_directories(${SDK_FOLDER}/devices)
include_directories(${SDK_FOLDER}/devices/${DEVICE_NAME})
include_directories(${SDK_FOLDER}/devices/${DEVICE_NAME}/drivers)
include_directories(${SDK_FOLDER}/devices/${DEVICE_NAME}/gcc)
include_directories(${SDK_FOLDER}/devices/${DEVICE_NAME}/utilities)

#  LWIP directories to look for CMakeLists.txt
include_directories(${SDK_FOLDER}/middleware/lwip/src)
set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${SDK_FOLDER}/middleware/lwip")

SET(RTOS_SOURCE_DIR ${SDK_FOLDER}/rtos/amazon-freertos/freertos_kernel)
SET(LWIP_PORT_SOURCE_DIR ${SDK_FOLDER}/middleware/lwip/port)
SET(BOARD_DIR ${SDK_FOLDER}/boards/lpcxpresso54628)
if ("${MBED_CLOUD_CLIENT_NATIVE_SDK}" STREQUAL "True")
   SET(BOARD_BSP_DIR ${CMAKE_SOURCE_DIR}/../)
   SET(BOARD_EXAMPLE_DIR ${CMAKE_SOURCE_DIR}/../)
else()
   SET(BOARD_BSP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated/)
   SET(BOARD_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pelion_enet/)
endif()
SET(DEVICE_DIR ${SDK_FOLDER}/devices/LPC54628)
SET(BOARD_UTIL_DIR ${DEVICE_DIR}/utilities)
SET(BOARD_DRIVERS_DIR ${DEVICE_DIR}/drivers)
SET(BOARD_COMPONENTS_DIR ${SDK_FOLDER}/components)
SET(BOARD_COMPILER_DIR ${DEVICE_DIR}/gcc)
SET(KSDK_SOURCE_DIR ${SDK_FOLDER}/middleware/mbedtls/port/ksdk)
SET(MMCAU_SOURCE_DIR ${SDK_FOLDER}/middleware/mmcau)

SET(FreeRTOS_SRCS
        ${RTOS_SOURCE_DIR}/event_groups.c
        ${RTOS_SOURCE_DIR}/list.c
        ${RTOS_SOURCE_DIR}/queue.c
        ${RTOS_SOURCE_DIR}/tasks.c
        ${RTOS_SOURCE_DIR}/timers.c
        ${RTOS_SOURCE_DIR}/portable/MemMang/heap_3.c
        ${RTOS_SOURCE_DIR}/portable/GCC/ARM_CM4F/port.c
        ${LWIP_PORT_SOURCE_DIR}/enet_ethernetif.c
        ${LWIP_PORT_SOURCE_DIR}/enet_ethernetif_lpc.c
        ${LWIP_PORT_SOURCE_DIR}/sys_arch.c
        ${BOARD_COMPILER_DIR}/startup_LPC54628.S
        ${BOARD_UTIL_DIR}/fsl_sbrk.c
        ${BOARD_UTIL_DIR}/debug_console/fsl_debug_console.c
        ${BOARD_DRIVERS_DIR}/fsl_usart.c
        ${BOARD_DRIVERS_DIR}/fsl_clock.c
        ${BOARD_DRIVERS_DIR}/fsl_common.c
        ${BOARD_DRIVERS_DIR}/fsl_gpio.c
        ${BOARD_COMPONENTS_DIR}/phylan8720a/fsl_phy.c
        ${BOARD_DRIVERS_DIR}/fsl_enet.c
        ${BOARD_DRIVERS_DIR}/fsl_reset.c
        ${BOARD_DRIVERS_DIR}/fsl_rtc.c
        ${DEVICE_DIR}/system_LPC54628.c
        ${BOARD_BSP_DIR}/board.c
        ${BOARD_BSP_DIR}/pin_mux.c
        ${BOARD_BSP_DIR}/clock_config.c
        ${BOARD_EXAMPLE_DIR}/rtos_startup.c
        ${KSDK_SOURCE_DIR}/ksdk_mbedtls.c
        ${KSDK_SOURCE_DIR}/aes_alt.c
        ${KSDK_SOURCE_DIR}/des_alt.c
        ${KSDK_SOURCE_DIR}/ecp_alt.c
        ${KSDK_SOURCE_DIR}/ecp_curves_alt.c
        ${KSDK_SOURCE_DIR}/ecp_alt_ksdk.c
)

add_library(_power STATIC IMPORTED)
set_target_properties(_power PROPERTIES IMPORTED_LOCATION ${BOARD_COMPILER_DIR}/libpower_hardabi.a)

add_library(NXP_FreeRTOS STATIC ${FreeRTOS_SRCS})
add_dependencies(NXP_FreeRTOS _power)
target_link_libraries(NXP_FreeRTOS _power)

SET(LWIP_PORT_SRCS
        ${LWIP_PORT_SOURCE_DIR}/enet_ethernetif.c
        ${LWIP_PORT_SOURCE_DIR}/enet_ethernetif_lpc.c
        ${LWIP_PORT_SOURCE_DIR}/sys_arch.c
)

add_library(lwip_port STATIC ${LWIP_PORT_SRCS})

include_directories (${BOARD_EXAMPLE_DIR})

SET(BOARD_SOURCES
        ${BOARD_COMPILER_DIR}/startup_LPC54628.S
        ${BOARD_COMPONENTS_DIR}/serial_manager/serial_manager.c
        ${BOARD_COMPONENTS_DIR}/serial_manager/serial_port_uart.c
        ${BOARD_COMPONENTS_DIR}/uart/usart_adapter.c
        ${DEVICE_DIR}/system_LPC54628.c
        ${BOARD_DRIVERS_DIR}/fsl_clock.c
        ${BOARD_DRIVERS_DIR}/fsl_common.c
        ${BOARD_DRIVERS_DIR}/fsl_enet.c
        ${BOARD_DRIVERS_DIR}/fsl_flexcomm.c
        ${BOARD_DRIVERS_DIR}/fsl_iap.c
        ${BOARD_DRIVERS_DIR}/fsl_gpio.c
        ${BOARD_DRIVERS_DIR}/fsl_rtc.c
        ${BOARD_DRIVERS_DIR}/fsl_usart.c
        ${BOARD_DRIVERS_DIR}/fsl_spifi.c
        ${BOARD_BSP_DIR}/board.c
        ${BOARD_BSP_DIR}/clock_config.c
        ${BOARD_BSP_DIR}/pin_mux.c
        ${BOARD_EXAMPLE_DIR}/rtos_startup.c
        ${BOARD_UTIL_DIR}/debug_console/fsl_debug_console.c
        ${BOARD_UTIL_DIR}/fsl_sbrk.c
        ${BOARD_UTIL_DIR}/str/fsl_str.c
)

add_library(board STATIC ${BOARD_SOURCES})

include_directories (${SDK_FOLDER}/middleware/mbedtls)
include_directories (${SDK_FOLDER}/middleware/mbedtls/include)
include_directories (${SDK_FOLDER}/middleware/mbedtls/include/mbedtls)
include_directories (${SDK_FOLDER}/middleware/mbedtls/port/ksdk)

set(MBEDTLS_SOURCE_DIR ${SDK_FOLDER}/middleware/mbedtls/library)

set(MBEDCRYPTO_SOURCES
    ${MBEDTLS_SOURCE_DIR}/aes.c
    ${MBEDTLS_SOURCE_DIR}/aesni.c
    ${MBEDTLS_SOURCE_DIR}/arc4.c
    ${MBEDTLS_SOURCE_DIR}/aria.c
    ${MBEDTLS_SOURCE_DIR}/asn1parse.c
    ${MBEDTLS_SOURCE_DIR}/asn1write.c
    ${MBEDTLS_SOURCE_DIR}/base64.c
    ${MBEDTLS_SOURCE_DIR}/bignum.c
    ${MBEDTLS_SOURCE_DIR}/blowfish.c
    ${MBEDTLS_SOURCE_DIR}/camellia.c
    ${MBEDTLS_SOURCE_DIR}/ccm.c
    ${MBEDTLS_SOURCE_DIR}/chacha20.c
    ${MBEDTLS_SOURCE_DIR}/chachapoly.c
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
    ${MBEDTLS_SOURCE_DIR}/hkdf.c
    ${MBEDTLS_SOURCE_DIR}/hmac_drbg.c
    ${MBEDTLS_SOURCE_DIR}/md.c
    ${MBEDTLS_SOURCE_DIR}/md2.c
    ${MBEDTLS_SOURCE_DIR}/md4.c
    ${MBEDTLS_SOURCE_DIR}/md5.c
    ${MBEDTLS_SOURCE_DIR}/md_wrap.c
    ${MBEDTLS_SOURCE_DIR}/memory_buffer_alloc.c
    ${MBEDTLS_SOURCE_DIR}/nist_kw.c
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
    ${MBEDTLS_SOURCE_DIR}/poly1305.c
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

set(MBEDTLS_SOURCE_DIR ${SDK_FOLDER}/middleware/mbedtls/library)

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

set(MBEDX509_SOURCES
    ${MBEDTLS_SOURCE_DIR}/certs.c
    ${MBEDTLS_SOURCE_DIR}/pkcs11.c
    ${MBEDTLS_SOURCE_DIR}/x509.c
    ${MBEDTLS_SOURCE_DIR}/x509_create.c
    ${MBEDTLS_SOURCE_DIR}/x509_crl.c
    ${MBEDTLS_SOURCE_DIR}/x509_crt.c
    ${MBEDTLS_SOURCE_DIR}/x509_csr.c
    ${MBEDTLS_SOURCE_DIR}/x509write_crt.c
    ${MBEDTLS_SOURCE_DIR}/x509write_csr.c
)

add_library(mbedx509 STATIC ${MBEDX509_SOURCES})

set(KSDK_SOURCE_DIR ${SDK_FOLDER}/middleware/mbedtls/port/ksdk)

set(KSDK_SOURCES
    ${KSDK_SOURCE_DIR}/ksdk_mbedtls.c
    ${KSDK_SOURCE_DIR}/aes_alt.c
    ${KSDK_SOURCE_DIR}/des_alt.c
    ${KSDK_SOURCE_DIR}/ecp_alt.c
    ${KSDK_SOURCE_DIR}/ecp_curves_alt.c
    ${KSDK_SOURCE_DIR}/ecp_alt_ksdk.c
)

# The following lwip block is a workaround for a cmake bug in SDK_2.7.0_LPCXpresso54628/middleware/lwip/src/Filelists.cmake:271
# where SDK_2.7.0_LPCXpresso54628/middleware/lwip/src/netif/ppp/ecp.c is included to be compiled but the file doesn't exist.
# This block can be removed once the SDK cmake bug is fixed by NXP. In addition to removing this block, uncomment the following line:
# set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "${LPC54_NXP_FOLDER}/middleware/${PAL_NETWORK_LIB}") in LPCXpresso54628.cmake.

set(LWIP_DIR ${SDK_FOLDER}/middleware/lwip)

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
)
set(lwipcore4_SRCS
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
set(lwipppp_SRCS
    ${LWIP_DIR}/src/netif/ppp/auth.c
    ${LWIP_DIR}/src/netif/ppp/ccp.c
    ${LWIP_DIR}/src/netif/ppp/chap-md5.c
    ${LWIP_DIR}/src/netif/ppp/chap_ms.c
    ${LWIP_DIR}/src/netif/ppp/chap-new.c
    ${LWIP_DIR}/src/netif/ppp/demand.c
    ${LWIP_DIR}/src/netif/ppp/eap.c
    ${LWIP_DIR}/src/netif/ppp/lwip_ecp.c
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
    ${lwipppp_SRCS}
)

add_library(ksdk STATIC ${KSDK_SOURCES})

add_library(lwipcore STATIC ${lwipnoapps_SRCS})

add_library(PLATFORM_LIBS STATIC ${FreeRTOS_SRCS} ${LWIP_PORT_SRCS} ${BOARD_SOURCES} ${MBEDCRYPTO_SOURCES} ${MBEDTLS_SOURCES} ${MBEDX509_SOURCES} ${KSDK_SOURCES})
add_dependencies(PLATFORM_LIBS lwipcore pdmc_port)
target_link_libraries(PLATFORM_LIBS lwipcore pdmc_port)
