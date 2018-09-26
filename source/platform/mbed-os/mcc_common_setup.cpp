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
#include "mcc_common_setup.h"
#include "mcc_common_config.h"

// This is for single or dual partition mode. This is supposed to be used with storage for data e.g. SD card.
// Enable by 1/disable by 0.
#ifndef MCC_PLATFORM_PARTITION_MODE
#define MCC_PLATFORM_PARTITION_MODE 0
#endif

#include "pal.h"
#if (MCC_PLATFORM_PARTITION_MODE == 1)
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"

// Set to 1 for enabling automatic partitioning storage if required. This is effective only if MCC_PLATFORM_PARTITION_MODE is defined to 1.
// Partioning will be triggered only if initialization of available partitions fail.
#ifndef MCC_PLATFORM_AUTO_PARTITION
#define MCC_PLATFORM_AUTO_PARTITION 0
#endif

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

// for checking that PRIMARY_PARTITION_SIZE and SECONDARY_PARTITION_SIZE do not overflow.
static bd_size_t mcc_platform_storage_size = 0;
#endif // MCC_PLATFORM_PARTITION_MODE


/* local help functions. */
static int mcc_platform_reformat_partition(FileSystem *fs, BlockDevice* part);
static int mcc_platform_test_filesystem(FileSystem *fs, BlockDevice* part);
#if (MCC_PLATFORM_PARTITION_MODE == 1)
static int mcc_platform_init_and_mount_partition(FileSystem **fs, BlockDevice** part, int number_of_partition, const char* mount_point);
#if (MCC_PLATFORM_AUTO_PARTITION == 1)
static int mcc_platform_create_partitions();
#endif
#endif

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////
static NetworkInterface* network_interface=NULL;

static BlockDevice* bd = NULL;
#ifdef ARM_UC_USE_PAL_BLOCKDEVICE
// Can be moved extern reference under update src. No reason keep here because get_default_instance is mbed-os interface.
BlockDevice* arm_uc_blockdevice = BlockDevice::get_default_instance();
#endif

// blockdevice and filesystem pointers for storage
static BlockDevice *part1 = NULL;
static FileSystem *fs1 = NULL;

#if (MCC_PLATFORM_PARTITION_MODE == 1)
#if (NUMBER_OF_PARTITIONS == 2)
static BlockDevice *part2 = NULL;
static FileSystem *fs2 = NULL;
#endif
#if (NUMBER_OF_PARTITIONS > 2)
#error "Invalid number of partitions!!!"
#endif
#endif // MCC_PLATFORM_PARTITION_MODE

////////////////////////////////
// SETUP_COMMON.H IMPLEMENTATION
////////////////////////////////
int mcc_platform_init_connection(void) {
// Perform number of retries if network init fails.
#ifndef MCC_PLATFORM_CONNECTION_RETRY_COUNT
#define MCC_PLATFORM_CONNECTION_RETRY_COUNT 3
#endif
    printf("mcc_platform_init_connection()\n");

    network_interface = NetworkInterface::get_default_instance();
    if(network_interface == NULL) {
        printf("ERROR: No NetworkInterface found!\n");
        return -1;
    }
    for (int i=0; i < MCC_PLATFORM_CONNECTION_RETRY_COUNT; i++) {
        nsapi_error_t e;
        e = network_interface->connect();
        if (e == NSAPI_ERROR_OK) {
            return 0;
        }
        printf("Failed to connect! error=%d\n", e);
    }
    return -1;
}

int mcc_platform_close_connection(void) {

    if (network_interface) {
        const nsapi_error_t err = network_interface->disconnect();
        if (err == NSAPI_ERROR_OK) {
            network_interface = NULL;
            return 0;
        }
    }
    return -1;
}

void* mcc_platform_get_network_interface(void) {
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

int mcc_platform_reformat_storage(void) {
   int status = -1;

   printf("Reformat the storage.\n");
   if (bd) {
#if (NUMBER_OF_PARTITIONS > 0)
        status = mcc_platform_reformat_partition(fs1, part1);
        if (status != 0) {
            printf("Formatting primary partition failed with 0x%X !!!\n", status);
            return status;
        }
#if (NUMBER_OF_PARTITIONS == 2)
        status = mcc_platform_reformat_partition(fs2, part2);
        if (status != 0) {
            printf("Formatting secondary partition failed with 0x%X !!!\n", status);
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

#if (MCC_PLATFORM_PARTITION_MODE == 1)
FileSystem *FileSystem::get_default_instance()
{
    return fs1;
}
#endif

#if (MCC_PLATFORM_PARTITION_MODE == 1)
// bd must be initialized before calling this function.
static int mcc_platform_init_and_mount_partition(FileSystem **fs, BlockDevice** part, int number_of_partition, const char* mount_point) {
    int status;

    // Init fs only once.
    if (&(**fs) == NULL) {
        if (&(**part) == NULL) {
            *part = new MBRBlockDevice(bd, number_of_partition);
        }
        status = (**part).init();
        if (status != 0) {
            (**part).deinit();
            printf("Init of partition %d fail !!!\n", number_of_partition);
            return status;
        }
        /* This next change mean that filesystem will be FAT. */
        *fs = new FATFileSystem(mount_point, &(**part));  /* this also mount fs. */
     }
     // re-init and format.
     else {
        status = (**part).init();
        if (status != 0) {
            (**part).deinit();
            printf("Init of partition %d fail !!!\n", number_of_partition);
            return status;
        }

        printf("Formatting partition %d ...\n", number_of_partition);
        status = mcc_platform_reformat_partition(&(**fs), &(**part));
        if (status != 0) {
            printf("Formatting partition %d failed with 0x%X !!!\n", number_of_partition, status);
            return status;
        }
    }

    status = mcc_platform_test_filesystem(&(**fs), &(**part));
    if (status != 0) {
        printf("Formatting partition %d ...\n", number_of_partition);
        status = mcc_platform_reformat_partition(&(**fs), &(**part));
        if (status != 0) {
            printf("Formatting partition %d failed with 0x%X !!!\n", number_of_partition, status);
            return status;
        }
    }

    return status;
}
#endif

// create partitions, initialize and mount partitions
#if ((MCC_PLATFORM_PARTITION_MODE == 1) && (MCC_PLATFORM_AUTO_PARTITION == 1))
static int mcc_platform_create_partitions(void) {
    int status;

#if (NUMBER_OF_PARTITIONS > 0)
    if (mcc_platform_storage_size < PRIMARY_PARTITION_SIZE) {
        printf("mcc_platform_create_partitions PRIMARY_PARTITION_SIZE too large!!! Storage's size is %" PRIu64 \
                " and PRIMARY_PARTITION_SIZE is %" PRIu64 "\n",
                (uint64_t)mcc_platform_storage_size, (uint64_t)PRIMARY_PARTITION_SIZE);
        assert(0);
    }

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
    if (mcc_platform_storage_size < ((uint64_t)PRIMARY_PARTITION_SIZE + (uint64_t)SECONDARY_PARTITION_SIZE)) {
        printf("mcc_platform_create_partitions (PRIMARY_PARTITION_SIZE+SECONDARY_PARTITION_SIZE) too large!!! Storage's size is %" PRIu64 \
                " and (PRIMARY_PARTITION_SIZE+SECONDARY_PARTITION_SIZE) %" PRIu64 "\n",
                (uint64_t)mcc_platform_storage_size, (uint64_t)(PRIMARY_PARTITION_SIZE+SECONDARY_PARTITION_SIZE));
        assert(0);
    }

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
#endif // (NUMBER_OF_PARTITIONS > 0)
    return status;
}
#endif // ((MCC_PLATFORM_PARTITION_MODE == 1) && (MCC_PLATFORM_AUTO_PARTITION == 1))

int mcc_platform_storage_init(void) {
    static bool init_done=false;
    int status=0;

    if(!init_done) {
        bd = BlockDevice::get_default_instance();
        if (bd) {
            status = bd->init();

            if (status != BD_ERROR_OK) {
                printf("mcc_platform_storage_init() - bd->init() failed with %d\n", status);
                return -1;
            }

#if (MCC_PLATFORM_PARTITION_MODE == 1)
            // store partition size
            mcc_platform_storage_size = bd->size();
#endif
            printf("mcc_platform_storage_init() - bd->size() = %llu\n", bd->size());
            printf("mcc_platform_storage_init() - BlockDevice init OK.\n");
        }

#if (MCC_PLATFORM_PARTITION_MODE == 1)
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
#endif // (NUMBER_OF_PARTITIONS == 2)
#if (NUMBER_OF_PARTITIONS > 2)
#error "Invalid number of partitions!!!"
#endif
#endif // (NUMBER_OF_PARTITIONS > 0)
#else  // Else for #if (MCC_PLATFORM_PARTITION_MODE == 1)
    fs1 = FileSystem::get_default_instance();  /* this also mount fs. */
    part1 = bd;                   /* required for mcc_platform_reformat_storage */
    status = mcc_platform_test_filesystem(fs1, bd);
    if (status != 0) {
        printf("Formatting ...\n");
        status = mcc_platform_reformat_partition(fs1, bd);
        if (status != 0) {
            printf("Formatting failed with 0x%X !!!\n", status);
            return status;
        }
    }
#endif // MCC_PLATFORM_PARTITION_MODE
        init_done=true;
    }
    else {
        printf("mcc_platform_storage_init() - init already done.\n");
    }

    return status;
}

int mcc_platform_init(void)
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

void mcc_platform_sw_build_info(void) {
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\n");

    // The Mbed OS' master branch does not define the version numbers at all, so we need
    // some ifdeffery to keep compilations running.
#if defined(MBED_MAJOR_VERSION) && defined(MBED_MINOR_VERSION) && defined(MBED_PATCH_VERSION)
    printf("Mbed OS version %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#else
    printf("Mbed OS version <UNKNOWN>\n");
#endif
}
