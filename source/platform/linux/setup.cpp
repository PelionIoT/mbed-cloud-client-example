// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
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


#ifdef __linux__

///////////
// INCLUDES
///////////
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "setup.h"
#include "simplem2mclient.h"
#include "application_init.h"
#include "pal.h"
#include "mbed-trace/mbed_trace.h"

#define TRACE_GROUP "init"

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////

//This has to be "./pal" for now as this is the default which is picked by ESFS.
// If you want to pass another folder name , you need to do it through ESFS API otherwise
// mounting of folder will fail.
#define DEFAULT_FOLDER_NAME         "./pal"

// Internal function prototypes
void init_screen();
typedef void (*signalhandler_t)(int);
static void handle_signal(void);
void *network_interface = 0;

//  PAL_NET_DEFAULT_INTERFACE 0xFFFFFFFF
static unsigned int network = 0xFFFFFFFF;

pthread_t resource_thread;
static volatile bool button_pressed = false;

static void handle_signal(void)
{
    pthread_detach(resource_thread);
    exit(0);
}

void *button_thread(void *)
{
    for(;;) {
        getchar();
        button_pressed = true;
    }
    return NULL;
}

/////////////////////////
// SETUP.H IMPLEMENTATION
/////////////////////////
int initPlatform()
{
    init_screen();
    pthread_create(&resource_thread, NULL, &button_thread, NULL);
    signal(SIGTERM, (signalhandler_t)handle_signal);
    return NULL;
}

int reformat_storage()
{
    return 0;
}

bool rmFirmwareImages()
{
    printf("rmFirmwareImages is not supported on linux!\n");
    return false;
}

int run_application(int(*function)(void))
{
    // application_init() runs the following initializations:
    //  1. trace initialization
    //  2. platform initialization
    //  3. print memory statistics if MBED_HEAP_STATS_ENABLED is defined
    //  4. FCC initialization.
    if (!application_init()) {
        printf("Initialization failed, exiting application!\n");
        return 1;
    }
    return function();
}

bool init_connection()
{
    network_interface = &network;
    return true;
}

void* get_network_interface()
{
    return network_interface;
}

int create_default_storage_folder(void)
{
    palStatus_t status = PAL_SUCCESS;
    char path[PAL_MAX_FILE_AND_FOLDER_LENGTH];

#if (PAL_NUMBER_OF_PARTITIONS == 1)
    tr_debug("In single-partition mode.");
#elif (PAL_NUMBER_OF_PARTITIONS == 2)
    tr_debug("In dual-partition mode.");
#else
    tr_err("Undefined partition mode.");
    assert(0);
#endif

    // Get default mount point.
    status = pal_fsGetMountPoint(PAL_FS_PARTITION_PRIMARY, PAL_MAX_FILE_AND_FOLDER_LENGTH, path);
    if(status != PAL_SUCCESS)
    {
        tr_err("Fetching of PAL_FS_PARTITION_PRIMARY path %s failed", path);
        return -1;
    }

    // Make the sub-path
    tr_debug("Creating path %s", path);
    int res = mkdir(path,0744);
    if(res)
    {
        // Ignore error if it exists
        if( errno != EEXIST)
        {
            tr_err("Creation of PAL_FS_PARTITION_PRIMARY path %s failed with errno %d", path,res);
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
    tr_debug("Creating path %s", path);
    res = mkdir(path,0744);
    if(res)
    {
        // Ignore error if it exists
        if( errno != EEXIST)
        {
            tr_err("Creation of PAL_FS_PARTITION_PRIMARY path %s failed with errno %d", path,res);
            return -1;
        }
    }
#endif
    return 0;
}

void toggle_led(void)
{
    printf("Virtual LED toggled\n");
}

uint8_t button_clicked(void)
{
    if (button_pressed) {
        button_pressed = false;
        return true;
    }
    return false;
}

void do_wait(int timeout_ms)
{
    usleep(timeout_ms * 1000);
}

void led_off(void) {}
void init_screen() {}
void print_to_screen(int x, int y, const char* buffer) {}
void clear_screen() {}
void print_heap_stats() {}
void print_m2mobject_stats() {}
void create_m2mobject_test_set(M2MObjectList* /*object_list*/) {}

#endif // __linux__
