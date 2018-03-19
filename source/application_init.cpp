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


#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "factory_configurator_client.h"
#include "setup.h"
#include "application_init.h"

static bool application_init_mbed_trace(void)
{
    // Create mutex for tracing to avoid broken lines in logs
    if(!mbed_trace_helper_create_mutex()) {
        printf("ERROR - Mutex creation for mbed_trace failed!\n");
        return 1;
    }

    // Initialize mbed trace
    mbed_trace_init();
    mbed_trace_mutex_wait_function_set(mbed_trace_helper_mutex_wait);
    mbed_trace_mutex_release_function_set(mbed_trace_helper_mutex_release);

    return 0;
}

static void reset_storage(void)
{
    printf("Resets storage to an empty state.\n");
    fcc_status_e delete_status = fcc_storage_delete();
    if (delete_status != FCC_STATUS_SUCCESS) {
        printf("Failed to delete storage - %d\n", delete_status);
    }
}

static bool application_init_fcc(void)
{
    fcc_status_e status = fcc_init();
    if(status != FCC_STATUS_SUCCESS) {
        printf("fcc_init failed with status %d! - exit\n", status);
        return 1;
    }

    // This is designed to simplify user-experience by auto-formatting the
    // primary storage if no valid certificates exist.
    // This should never be used for any kind of production devices.
#ifndef MBED_CONF_APP_MCC_NO_AUTO_FORMAT
    status = fcc_verify_device_configured_4mbed_cloud();
    if (status != FCC_STATUS_SUCCESS) {
        if (reformat_storage() != 0) {
            return 1;
        }
    reset_storage();
    }
#endif

    // Resets storage to an empty state.
    // Use this function when you want to clear storage from all the factory-tool generated data and user data.
    // After this operation device must be injected again by using factory tool or developer certificate.
#ifdef RESET_STORAGE
    reset_storage();
#endif

    // Deletes existing firmware images from storage.
    // This deletes any existing firmware images during application startup.
    // This compilation flag is currently implemented only for mbed OS.
#ifdef RESET_FIRMWARE
    bool status_erase = rmFirmwareImages();
    if(status_erase == false) {
        return 1;
    }
#endif

#if MBED_CONF_APP_DEVELOPER_MODE == 1
    printf("Start developer flow\n");
    status = fcc_developer_flow();
    if (status == FCC_STATUS_KCM_FILE_EXIST_ERROR) {
        printf("Developer credentials already exists\n");
    } else if (status != FCC_STATUS_SUCCESS) {
        printf("Failed to load developer credentials - exit\n");
        return 1;
    }
#endif
    status = fcc_verify_device_configured_4mbed_cloud();
    if (status != FCC_STATUS_SUCCESS) {
        printf("Device not configured for mbed Cloud - exit\n");
        return 1;
    }

    return 0;
}

bool application_init(void)
{
    if (application_init_mbed_trace() != 0) {
        printf("Failed initializing mbed trace\n" );
        return false;
    }

    if (create_default_storage_folder() != 0) {
        printf("Failed to initialize storage\n" );
        return false;
    }

    if(initPlatform() != 0) {
       printf("ERROR - initPlatform() failed!\n");
       return false;
    }

    // Print some statistics of the object sizes and heap memory consumption
    // if the MBED_HEAP_STATS_ENABLED is defined.
    print_m2mobject_stats();
    print_heap_stats();

    printf("Start simple mbed Cloud Client\n");

    if (application_init_fcc() != 0) {
        printf("Failed initializing FCC\n" );
        return false;
    }

    return true;
}
