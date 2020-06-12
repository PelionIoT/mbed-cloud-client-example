/*
 * Copyright (c) 2020 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mcc_common_setup.h"
#include "mcc_common_config.h"
#include "RTE_Components.h"
#include "cmsis_os2.h"
#include "pal.h"

#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif

#ifdef RTE_IoT_Socket_MDK_Network
#include "rl_net.h"
#endif

#ifdef RTE_IoT_Socket_WiFi
#include "Driver_WiFi.h"
#endif

#include <string.h>
#include <stdio.h>

#define APP_MAIN_STK_SZ (0x2000)

uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

static void *network_interface = NULL;

#ifdef RTE_IoT_Socket_WiFi

#include "Driver_WiFi.h"
#include <string.h>
#include <stdio.h>
#define SSID            "SSID"
#define PASSWORD        "Password"
#define SECURITY_TYPE   ARM_WIFI_SECURITY_WPA2

extern ARM_DRIVER_WIFI Driver_WiFi0;
ARM_WIFI_CONFIG_t config;

#endif // RTE_IoT_Socket_WiFi

////////////////////////////////
// SETUP_COMMON.H IMPLEMENTATION
////////////////////////////////
int mcc_platform_init_connection(void)
{
#ifdef RTE_IoT_Socket_WiFi
    printf("Connecting to WiFi ...\r\n");

    if (Driver_WiFi0.Initialize  (NULL) != ARM_DRIVER_OK) {
        printf("Failed to initialize wifi!\n");
        return -1;
    }

    Driver_WiFi0.PowerControl(ARM_POWER_FULL);

    memset((void *)&config, 0, sizeof(config));

    config.ssid     = SSID;
    config.pass     = PASSWORD;
    config.security = SECURITY_TYPE;
    config.ch       = 0U;

    Driver_WiFi0.Activate(0U, &config);

    if (Driver_WiFi0.IsConnected() == 0U) {
        printf("WiFi network connection failed!\r\n");
    } else {
        printf("WiFi network connection succeeded!\r\n");

    }
#elif defined(RTE_Network_Socket_BSD)

    netStatus status = netInitialize();
    if (status != netOK) {
        return -1;
    }

    // Loop until link is up
    uint32_t addr;
    do {
       osDelay(1000U);
       netIF_GetOption(NET_IF_CLASS_ETH | 0,
                       netIF_OptionIP4_Address,
                      (uint8_t *)&addr, sizeof (addr));
    } while (addr == 0U);

    uint8_t ip4_addr[NET_ADDR_IP4_LEN];
    char    ip4_ascii[16];


    if (netIF_GetOption (NET_IF_CLASS_ETH | 0, netIF_OptionIP4_Address,
                                         ip4_addr, sizeof(ip4_addr)) == netOK) {

        netIP_ntoa (NET_ADDR_IP4, ip4_addr, ip4_ascii, sizeof(ip4_ascii));
        printf("IP4=%s\n", ip4_ascii);
    }
#endif

    return 0;
}

int mcc_platform_close_connection(void)
{
#ifdef RTE_Network_Socket_BSD
    netStatus status = netUninitialize();
    if (status != netOK) {
        return -1;
    }
#endif

    return 0;
}

void* mcc_platform_get_network_interface(void)
{
    return network_interface;
}

void mcc_platform_interface_init(void)
{
    mcc_platform_init_connection();
}

int mcc_platform_interface_close(void)
{
    return 0;
}

int mcc_platform_interface_connect(void)
{
    return 0;
}

int mcc_platform_storage_init(void)
{
    return 0;
}


int mcc_platform_init(void)
{
#ifdef RTE_Compiler_EventRecorder
    EventRecorderInitialize(EventRecordAll, 1);
#endif

    osKernelInitialize();

    return 0;
}

int mcc_platform_reformat_storage(void)
{
    return 0;
}

void mcc_platform_do_wait(int timeout_ms)
{
    osDelay(timeout_ms);
}

int mcc_platform_run_program(main_t mainFunc)
{
    mainFunc();
    return 0;
}

void mcc_platform_sw_build_info(void)
{
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\n");
}

void mcc_platform_reboot(void)
{
    pal_osReboot();
}

int mcc_platform_rot_generate(void)
{
    return 0;
}

