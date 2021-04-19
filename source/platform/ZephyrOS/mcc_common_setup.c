// ----------------------------------------------------------------------------
// Copyright (c) 2021 Pelion. All rights reserved.
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

///////////
// INCLUDES
///////////
#include "mcc_common_setup.h"

#include <zephyr.h>

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////

#if 0
#include <stdio.h>

#undef DEBUG_PRINT
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#undef DEBUG_PRINT
#define DEBUG_PRINT(...)
#endif


////////////////////////////////
// SETUP_COMMON.H IMPLEMENTATION
////////////////////////////////

int mcc_platform_init_connection(void) {
    DEBUG_PRINT("mcc_platform_init_connection\r\n");

    return 0;
}

int mcc_platform_run_program(main_t mainFunc)
{
    DEBUG_PRINT("mcc_platform_run_program\r\n");

    mainFunc();

    return 0;
}

void* mcc_platform_get_network_interface(void) {
    DEBUG_PRINT("mcc_platform_get_network_interface\r\n");

    return NULL;
}

int mcc_platform_close_connection(void) {
    DEBUG_PRINT("mcc_platform_close_connection\r\n");

    return 0;
}

int mcc_platform_interface_connect(void) {
    DEBUG_PRINT("mcc_platform_interface_connect\r\n");

    return 0;
}

int mcc_platform_interface_close(void) {
    DEBUG_PRINT("mcc_platform_interface_close\r\n");

    return 0;
}

void* mcc_platform_interface_get(void) {
    DEBUG_PRINT("mcc_platform_interface_get\r\n");

    return NULL;
}

void mcc_platform_interface_init(void) {
    DEBUG_PRINT("mcc_platform_interface_init\r\n");
}

int mcc_platform_reformat_storage(void)
{
    DEBUG_PRINT("mcc_platform_reformat_storage\r\n");

    return 0;
}

int mcc_platform_storage_init(void)
{
    DEBUG_PRINT("mcc_platform_storage_init\r\n");

    return 0;
}

int mcc_platform_init(void)
{
    DEBUG_PRINT("mcc_platform_init\r\n");

    return 0;
}

void mcc_platform_do_wait(int timeout_ms)
{
    DEBUG_PRINT("mcc_platform_do_wait\r\n");
    k_msleep(timeout_ms);
}

void mcc_platform_sw_build_info(void) {
    DEBUG_PRINT("Application ready. Build at: " __DATE__ " " __TIME__ "\r\n");
}

void mcc_platform_reboot(void) {
    NVIC_SystemReset();
}
