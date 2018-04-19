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

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "mbed.h"
#include "common_setup.h"
// these defines needed before including easy connect
#define MBED_CONF_APP_ESP8266_TX MBED_CONF_APP_WIFI_TX
#define MBED_CONF_APP_ESP8266_RX MBED_CONF_APP_WIFI_RX
#include "easy-connect/easy-connect.h"
#include "pal.h"
#include "common_config.h"
#include "storage-selector/storage-selector.h"
#if (MBED_CONF_STORAGE_SELECTOR_STORAGE  == SD_CARD)
#include "MBRBlockDevice.h"

#ifndef PRIMARY_PARTITION_NUMBER
#define PRIMARY_PARTITION_NUMBER 1
#endif

#ifndef PRIMARY_PARTITION_START
#define PRIMARY_PARTITION_START 0
#endif

#ifndef PRIMARY_PARTITION_SIZE
#define PRIMARY_PARTITION_SIZE 1024*1024*1024 // default partition size 1GB
#endif

#ifndef SECONDARY_PARTITION_NUMBER
#define SECONDARY_PARTITION_NUMBER 2
#endif

#ifndef SECONDARY_PARTITION_START
#define SECONDARY_PARTITION_START PRIMARY_PARTITION_SIZE 
#endif

#ifndef SECONDARY_PARTITION_SIZE
#define SECONDARY_PARTITION_SIZE 1024*1024*1024 // default partition size 1GB
#endif

#ifndef NUMBER_OF_PARTITIONS
#define NUMBER_OF_PARTITIONS PAL_NUMBER_OF_PARTITIONS
#endif

#ifndef MOUNT_POINT_PRIMARY
#define MOUNT_POINT_PRIMARY PAL_FS_MOUNT_POINT_PRIMARY
#endif

#ifndef MOUNT_POINT_SECONDARY
#define MOUNT_POINT_SECONDARY PAL_FS_MOUNT_POINT_SECONDARY
#endif
#endif


/* local help functions. */
static int mcc_platform_reformat_partition(FileSystem *fs, BlockDevice* part);
static int mcc_platform_test_filesystem(FileSystem *fs, BlockDevice* part);
static int mcc_platform_init_and_mount_partition(FileSystem **fs, BlockDevice** part, int number_of_partition, const char* mount_point);
#if (MCC_PLATFORM_AUTO_PARTITION == 1)
static int mcc_platform_create_partitions();
#endif

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////
static NetworkInterface* network_interface;

// Some boards specific sanity checks, better stop early.
#if defined(TARGET_UBLOX_EVK_ODIN_W2) && defined(DEVICE_EMAC) && defined(MBED_CONF_APP_NETWORK_INTERFACE) && defined (ETHERNET) && (MBED_CONF_APP_NETWORK_INTERFACE == ETHERNET)
    #error "UBLOX_EVK_ODIN_W2 - does not work with Ethernet if you have EMAC on! Please fix your mbed_app.json."
#endif
#if defined(TARGET_UBLOX_EVK_ODIN_W2) && !defined(DEVICE_EMAC) && defined(MBED_CONF_APP_NETWORK_INTERFACE) && defined (WIFI_ODIN) && (MBED_CONF_APP_NETWORK_INTERFACE == WIFI_ODIN)
    #error "UBLOX_EVK_ODIN_W2 - does not work with WIFI_ODIN if you have disabled EMAC! Please fix your mbed_app.json."
#endif

static BlockDevice* bd = NULL;
#ifdef ARM_UC_USE_PAL_BLOCKDEVICE
BlockDevice* arm_uc_blockdevice = storage_selector();
#endif

#if (MBED_CONF_STORAGE_SELECTOR_STORAGE  == SD_CARD)
#if (NUMBER_OF_PARTITIONS > 0)
static BlockDevice *part1 = NULL;
static FileSystem *fs1 = NULL;
#if (NUMBER_OF_PARTITIONS == 2)
static BlockDevice *part2 = NULL;
static FileSystem *fs2 = NULL;
#endif
#if (NUMBER_OF_PARTITIONS > 2)
#error "Invalid number of partitions!!!"
#endif
#endif
#endif

////////////////////////////////
// SETUP_COMMON.H IMPLEMENTATION
////////////////////////////////
int mcc_platform_init_connection() {
    network_interface = easy_connect(true);
    if(network_interface == NULL) {
        return -1;
    }
    return 0;
}

int mcc_platform_close_connection() {

    if (network_interface) {
        const nsapi_error_t err = network_interface->disconnect();
        if (err == NSAPI_ERROR_OK) {
            network_interface = NULL;
            return 0;
        }
    }
    return -1;
}

void* mcc_platform_get_network_interface() {
    return network_interface;
}

/* help function format partition. */
static int mcc_platform_reformat_partition(FileSystem *fs, BlockDevice* part) {
    int status;

    printf("mcc_platform_reformat_partition\n");
    status = fs->reformat(part);
    if (status != 0) {
        printf("Reformat partition failed with error %d\n", status);
    }

    return status;
}

int mcc_platform_reformat_storage() {
   int status = -1;

   printf("Reformat the storage.\n");
   if (bd) {
#if (NUMBER_OF_PARTITIONS > 0)
        status = mcc_platform_reformat_partition(fs1, part1);
        if (status != 0) {
            printf("Formating primary partition failed with 0x%X !!!\n", status);
            return status;
        }
#if (NUMBER_OF_PARTITIONS == 2)
        status = mcc_platform_reformat_partition(fs2, part2);
        if (status != 0) {
            printf("Formating secondary partition failed with 0x%X !!!\n", status);
            return status;
        }
#endif
#if (NUMBER_OF_PARTITIONS > 2)
#error "Invalid number of partitions!!!"
#endif
#endif
    }
   return status;
}

/* help function for testing filesystem availbility by umount and
 * mount filesystem again.
 * */
static int mcc_platform_test_filesystem(FileSystem *fs, BlockDevice* part) {
    // unmount
    int status = fs->unmount();
    if (status != 0) {
        printf("mcc_platform_test_filesystem() - unmount fail %d.\n", status);
        return -1;
    }
    // mount again
    status = fs->mount(part);
    if (status != 0) {
        printf("mcc_platform_test_filesystem() - mount fail %d.\n", status);
        return -1;
    }
    return status;
}

// bd must be initialized before this function.
static int mcc_platform_init_and_mount_partition(FileSystem **fs, BlockDevice** part, int number_of_partition, const char* mount_point) {
    int status;

    // Init fs only once.
    if (&(**fs) == NULL) {
        if (&(**part) == NULL) {
            *part = new MBRBlockDevice(bd, number_of_partition);
        }
        status = (**part).init();
        if (status != 0) {
            printf("Init of partition %d fail !!!\n", number_of_partition);
            return status;
        }

        *fs = filesystem_selector(mount_point, &(**part),  number_of_partition);  /* this also mount fs. */
     }
     // re-init and format.
     else {
        status = (**part).init();
        if (status != 0) {
            printf("Init of partition %d fail !!!\n", number_of_partition);
            return status;
        }

        printf("Formating partition %d ...\n", number_of_partition);
        status = mcc_platform_reformat_partition(&(**fs), &(**part));
        if (status != 0) {
            printf("Formating partition %d fail 0x%X !!!\n", number_of_partition, status);
            return status;
        }
    }

    status = mcc_platform_test_filesystem(&(**fs), &(**part));
    if (status != 0) {
        printf("Formating partition %d ...\n", number_of_partition);
        status = mcc_platform_reformat_partition(&(**fs), &(**part));
        if (status != 0) {
            printf("Formating partition %d fail 0x%X !!!\n", number_of_partition, status);
            return status;
        }
    }

    return status;
}

// create partitions, initialize and mount partitions
#if (MCC_PLATFORM_AUTO_PARTITION == 1)
static int mcc_platform_create_partitions() {
    int status;

#if (NUMBER_OF_PARTITIONS > 0)
    status = MBRBlockDevice::partition(bd, PRIMARY_PARTITION_NUMBER, 0x83, PRIMARY_PARTITION_START, PRIMARY_PARTITION_START + PRIMARY_PARTITION_SIZE);
    printf("Creating primary partition ...\n");
    if (status != 0) {
        printf("Creating primary partition fail 0x%X !!!\n", status);
        return status;
    }

    // init and format partition 1
    status = mcc_platform_init_and_mount_partition(&fs1, &part1, PRIMARY_PARTITION_NUMBER, ((const char*) MOUNT_POINT_PRIMARY+1));
    if (status != 0) {
        return status;
    }
#if (NUMBER_OF_PARTITIONS == 2)
    // use cast (uint64_t) for fixing compile warning.
    status = MBRBlockDevice::partition(bd, SECONDARY_PARTITION_NUMBER, 0x83, SECONDARY_PARTITION_START, (uint64_t) SECONDARY_PARTITION_START + (uint64_t) SECONDARY_PARTITION_SIZE);
    printf("Creating secondary partition ...\n");
    if (status != 0) {
        printf("Creating secondary partition fail 0x%X !!!\n", status);
        return status;
    }

    // init and format partition 2
    status = mcc_platform_init_and_mount_partition(&fs2, &part2, SECONDARY_PARTITION_NUMBER, ((const char*) MOUNT_POINT_SECONDARY+1));
    if (status != 0) {
        return status;
    }
#endif
#if (NUMBER_OF_PARTITIONS > 2)
#error "Invalid number of partitions!!!"
#endif
#endif
    return status;
}
#endif

int mcc_platform_storage_init() {
    static bool init_done=false;
    int status=0;

    if(!init_done) {
        bd = storage_selector();
        if (bd) {
            status = bd->init();

            if (status != BD_ERROR_OK) {
                printf("mcc_platform_storage_init() - bd->init() failed with %d\n", status);
                return -1;
            }

            printf("mcc_platform_storage_init() - BlockDevice init OK.\n");
        }

#if (MBED_CONF_STORAGE_SELECTOR_STORAGE  == SD_CARD)
#if (NUMBER_OF_PARTITIONS > 0)
        status = mcc_platform_init_and_mount_partition(&fs1, &part1, PRIMARY_PARTITION_NUMBER, ((const char*) MOUNT_POINT_PRIMARY+1));
        if (status != 0) {
#if (MCC_PLATFORM_AUTO_PARTITION == 1)
            status = mcc_platform_create_partitions();
            if (status != 0) {
                return status;
            }
#else
            printf("mcc_platform_storage_init() - primary partition init fail!!!\n");
            return status;
#endif
        }

#if (NUMBER_OF_PARTITIONS == 2)
        status = mcc_platform_init_and_mount_partition(&fs2, &part2, SECONDARY_PARTITION_NUMBER, ((const char*) MOUNT_POINT_SECONDARY+1));
        if (status != 0) {
#if (MCC_PLATFORM_AUTO_PARTITION == 1)
            status = mcc_platform_create_partitions();
            if (status != 0) {
                return status;
            }
#else
            printf("mcc_platform_storage_init() - secondary partition init fail!!!\n");
            return status;
#endif
        }
#endif
#if (NUMBER_OF_PARTITIONS > 2)
#error "Invalid number of partitions!!!"
#endif
#endif
#endif
        init_done=true;
    }
    else {
        printf("mcc_platform_storage_init() - init already done.\n");
    }

    return status;
}

int mcc_platform_init()
{
    return 0;
}

void mcc_platform_do_wait(int timeout_ms)
{
    wait_ms(timeout_ms);
}

int mcc_platform_run_program(main_t mainFunc)
{
    mainFunc();

    return 1;
}

void mcc_platform_sw_build_info() {
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\n");
    
    // The Mbed OS' master branch does not define the version numbers at all, so we need
    // some ifdeffery to keep compilations running.
#if defined(MBED_MAJOR_VERSION) && defined(MBED_MINOR_VERSION) && defined(MBED_PATCH_VERSION)
    printf("Mbed OS version %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#else
    printf("Mbed OS version <UNKNOWN>\n");
#endif
}
