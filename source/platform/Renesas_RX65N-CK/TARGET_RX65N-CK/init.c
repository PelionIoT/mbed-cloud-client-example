// ----------------------------------------------------------------------------
// Copyright 2020 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include "mcc_common_setup.h"
#include "sx_ulpgn_driver.h"

#include <stdio.h>
#include "serial_term_uart.h"
#include "pal_plat_rtos.h"

#define WIFI_SSID               "SSID"
#define WIFI_PASSWORD           "Password"
#define WIFI_SECURITY_TYPE      ULPGN_SECURITY_WPA2

#define WIFI_INIT_ATTEMPS       3                       ///< Attemps to init wi_fi dongle
#define WIFI_CONNECT_ATTEMPS    3                       ///< Attemps to connect to AP
#define WIFI_ATTEMP_PERIOD      5000                    ///< Period between attemps, milliseconds

int mcc_platform_init(void)
{
    // all initialization done in SDK own startup

    return 0;
}

void mcc_platform_interface_init(void)
{
    mcc_platform_init_connection();
}

int mcc_platform_init_connection(void)
{
    // Deprecated, but used in tests initialization

    uint8_t attemp_cnt = 0;

    while(attemp_cnt < WIFI_INIT_ATTEMPS)
    {
        if (sx_ulpgn_wifi_init() != 0)
        {
            printf("Failed to initialize Wi-Fi dongle, attempt #%d\r\n", (attemp_cnt + 1));

            pal_osDelay(WIFI_ATTEMP_PERIOD);

            attemp_cnt++;
        }
        else
        {
            printf("Wi-Fi dongle initialized\r\n");
            return mcc_platform_interface_connect();
        }
    }
    // reboot when error
    pal_osReboot();

    return -1;
}

int mcc_platform_close_connection(void)
{
    // Deprecated, but used in tests

    return mcc_platform_interface_close();
}

int mcc_platform_interface_connect(void)
{
    uint8_t attemp_cnt = 0;

    while(attemp_cnt < WIFI_CONNECT_ATTEMPS)
    {
        if (sx_ulpgn_wifi_connect(WIFI_SSID, WIFI_SECURITY_TYPE, WIFI_PASSWORD) != 0)
        {
            printf("Failed connect to AP, attempt #%d\r\n", (attemp_cnt + 1));

            pal_osDelay(WIFI_ATTEMP_PERIOD);

            attemp_cnt++;
        }
        else
        {
            printf("Connected to \"%s\" Wi-Fi network\r\n", WIFI_SSID);
            return 0;
        }
    }
    // reboot when error
    pal_osReboot();

    return -1;
}

int mcc_platform_interface_close(void)
{
    if (sx_ulpgn_wifi_disconnect() != 0)
    {
        printf("Failed disconnect from AP\r\n");
        return -1;
    }
    return 0;
}

void* mcc_platform_get_network_interface(void)
{
    return (void*)pal_plat_malloc(sizeof(uint32_t));   // interface instance stub
}



void *__dso_handle = 0; // we do not use DSO

/// Redirect standart output to debug uart
int write(int fd, const void *buf, size_t count)
{
    const char* out = (const char*) buf;

    for (size_t index = 0; index < count; index++) {
        uart_charput(out[index]);
    }
    return count;
}

/// Implement standart input using debug uart input
int read(int fd, void *buf, size_t cnt)
{
    return uart_string_scanf(buf, cnt);
}
