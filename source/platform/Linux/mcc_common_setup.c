// ----------------------------------------------------------------------------
// Copyright 2015-2019 ARM Ltd.
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
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "mcc_common_setup.h"
#include "mcc_common_config.h"
#include "pal.h"

#include "mcc_common_button_and_led.h"

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////
static void *network_interface;

// PAL_NET_DEFAULT_INTERFACE 0xFFFFFFFF
static unsigned int network=0xFFFFFFFF;

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

int mcc_platform_interface_connect() {
    network_interface = &network;
    return 0;
}

int mcc_platform_interface_close() {
    network_interface = NULL;
    return 0;
}

void* mcc_platform_interface_get() {
    return network_interface;
}

void mcc_platform_interface_init(void) {}

// Desktop Linux
// In order for tests to pass for all partition configurations we need to simulate the case of multiple
// partitions using a single path. We do this by creating one or two different sub-paths, depending on
// the configuration.
int mcc_platform_storage_init(void)
{
    palStatus_t status = PAL_SUCCESS;
    char path[PAL_MAX_FILE_AND_FOLDER_LENGTH];

#if (PAL_NUMBER_OF_PARTITIONS == 1)
    printf("In single-partition mode.\n");
#elif (PAL_NUMBER_OF_PARTITIONS == 2)
    printf("In dual-partition mode.\n");
#else
    printf("Undefined partition mode.\n");
    assert(0);
#endif

    // Get default mount point.
    status = pal_fsGetMountPoint(PAL_FS_PARTITION_PRIMARY, PAL_MAX_FILE_AND_FOLDER_LENGTH, path);
    if(status != PAL_SUCCESS)
    {
        printf("Fetching of PAL_FS_PARTITION_PRIMARY path %s failed.\n", path);
        return -1;
    }

    // Make the sub-path
    printf("Creating path %s\n", path);
    int res = mkdir(path,0744);
    if(res)
    {
        // Ignore error if it exists
        if( errno != EEXIST)
        {
            printf("Creation of PAL_FS_PARTITION_PRIMARY path %s failed.\n", path);
            return -1;
        }
    }

#if (PAL_NUMBER_OF_PARTITIONS == 2)
    // Get default mount point.
    status = pal_fsGetMountPoint(PAL_FS_PARTITION_SECONDARY, PAL_MAX_FILE_AND_FOLDER_LENGTH, path);

    if(status != PAL_SUCCESS)
    {
        return -1;
    }

    // Make the sub-path
    printf("Creating path %s\n", path);
    res = mkdir(path,0744);
    if(res)
    {
        // Ignore error if it exists
        if( errno != EEXIST)
        {
            printf("Creation of PAL_FS_PARTITION_SECONDARY path %s failed.\n", path);
            return -1;
        }
    }
#endif
    return 0;
}


int mcc_platform_init()
{
    int err;

    // The current PAL side is now using a realtime signal for the timer code.
    // All the threads need to have that signal blocked to avoid default
    // signal handlers catching the signal.
    // The pal_init() called by MbedCloudClient will setup the current
    // thread's signals correctly, but all the threads created before that
    // need this setup.
    // Note: this is needed due to mcc_platform_init_button_and_led()
    // which is creating a thread during setup. If the client code is not
    // creating its own threads before MbedCloudClient construction, this
    // preparation is not needed.
    // Use ifdef to keep code compiling even with older versions,
    // where the masking is not needed.
#ifdef PAL_TIMER_SIGNAL
    sigset_t blocked;

    sigemptyset(&blocked);
    sigaddset(&blocked, PAL_TIMER_SIGNAL);

    err = pthread_sigmask(SIG_BLOCK, &blocked, NULL);
#else
    err = 0;
#endif
    return err;
}

int mcc_platform_reformat_storage(void)
{
// cleanup folders
// to do:
// PAL_FS_MOUNT_POINT_PRIMARY
// PAL_FS_MOUNT_POINT_SECONDARY
    printf("mcc_platform_reformat_storage does not support Linux!!!\n");
    return 0;
}

void mcc_platform_do_wait(int timeout_ms)
{
    struct timespec start_time;
    struct timespec remaining_time; // this will return how much sleep time still left in case of interrupted sleep
    int stat;

    remaining_time.tv_sec =  timeout_ms / 1000;
    remaining_time.tv_nsec = (timeout_ms - ((long)remaining_time.tv_sec * 1000)) * 1000000;
    do {
        start_time.tv_sec = remaining_time.tv_sec;
        start_time.tv_nsec = remaining_time.tv_nsec;
        stat = nanosleep(&start_time, &remaining_time);
    } while ((-1 == stat) && (EINTR == errno)) ;
}

int mcc_platform_run_program(main_t mainFunc)
{
    mainFunc();

    return 1;
}

void mcc_platform_sw_build_info() {
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\n");
}
