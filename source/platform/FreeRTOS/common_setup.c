/*
 * Copyright (c) 2015-2018 ARM Limited. All rights reserved.
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

///////////
// INCLUDES
///////////
#include <stdio.h>
#include <unistd.h>
#include "common_setup.h"
#include "ethernetif.h"

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////
static void *network_interface = 0;
extern void* mcc_platform_GetNetWorkInterfaceContext();

static int init_FreeRTOS = 0;
extern int mcc_platform_initFreeRTOSPlatform();

////////////////////////////////
// SETUP_COMMON.H IMPLEMENTATION
////////////////////////////////
int mcc_platform_init_connection() {
    network_interface = mcc_platform_GetNetWorkInterfaceContext();
    return 0;
}

int mcc_platform_close_connection() {

    // XXX: Ignore the fact that this is not implemented.
    return 0;
}

void* mcc_platform_get_network_interface() {
    return network_interface;
}

int mcc_platform_reformat_storage()
{
    return 0;
}

int mcc_platform_storage_init() {
    return 0;
}

int mcc_platform_init()
{   
    if(init_FreeRTOS) {
        return 0;
    }
    init_FreeRTOS = 1;
    if(mcc_platform_initFreeRTOSPlatform()) {
        return 0;
    }
    else {
        return -1;
    }
}

void mcc_platform_do_wait(int timeout_ms)
{
    vTaskDelay(timeout_ms);
}

void mcc_platform_sw_build_info() {
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\n");
}
