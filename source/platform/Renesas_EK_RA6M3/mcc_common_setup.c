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

///////////
// INCLUDES
///////////
#include "mcc_common_setup.h"
#include "pal.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////
static void *network_interface = 0;
extern void* mcc_platform_GetNetWorkInterfaceContext();

static int init_FreeRTOS = 0;
extern int mcc_platform_initFreeRTOSPlatform();

extern int networkInit(void);

/*!
 * @brief fileSystemMountDrive - mount the SD card to PAL_FS_PARTITION_PRIMARY
 * @param void
 * @return void
 */
extern int fileSystemMountDrive();


////////////////////////////////
// SETUP_COMMON.H IMPLEMENTATION
////////////////////////////////

int mcc_platform_init_connection(void) {
    return mcc_platform_interface_connect();
}

void* mcc_platform_get_network_interface(void) {
    return mcc_platform_interface_get();
}

int mcc_platform_close_connection(void) {
    return mcc_platform_interface_close();
}

int mcc_platform_interface_connect(void) {
    network_interface = mcc_platform_GetNetWorkInterfaceContext();
    return networkInit();
}

int mcc_platform_interface_close(void) {
    // XXX: Ignore the fact that this is not implemented.
    return 0;
}

void* mcc_platform_interface_get(void) {
    return network_interface;
}

void mcc_platform_interface_init(void) {}

int mcc_platform_reformat_storage(void)
{
    printf("mcc_platform_reformat_storage does not support FreeRTOS !!!\n");

    // XXX: this returns zero, which means success. This is needed to keep
    // some of the tests running, which ask for formatted storage, even though
    // they do not need it.
    return 0;
}

int mcc_platform_storage_init(void)
{
    // XXX: some of the existing users of this module actually call mcc_platform_storage_init()
    // BEFORE mcc_platform_init().
    //
    // Before all the callers are fixed, one needs to emulate the behavior of other platforms,
    // which do not care about the order of these.
    if (mcc_platform_init() == 0) {

        return fileSystemMountDrive();
    } else {
        return -1;
    }
}

int mcc_platform_init(void)
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
    vTaskDelay(pdMS_TO_TICKS(timeout_ms));
}

void mcc_platform_sw_build_info(void) {
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\n");
}

void mcc_platform_reboot(void) {
    pal_osReboot();
}

int mcc_platform_rot_generate(void) {
    return 0;
}

