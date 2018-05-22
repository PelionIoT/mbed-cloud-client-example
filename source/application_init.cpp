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
#include "common_setup.h"
#include "common_button_and_led.h"
#ifdef MBED_HEAP_STATS_ENABLED
#include "memory_tests.h"
#endif
#include "application_init.h"

static bool application_init_mbed_trace(void)
{
    // Create mutex for tracing to avoid broken lines in logs
    if(!mbed_trace_helper_create_mutex()) {
        printf("ERROR - Mutex creation for mbed_trace failed!\n");
        return 1;
    }

    // Initialize mbed trace
    (void) mbed_trace_init();
    mbed_trace_mutex_wait_function_set(mbed_trace_helper_mutex_wait);
    mbed_trace_mutex_release_function_set(mbed_trace_helper_mutex_release);

    return 0;
}

static bool application_init_verify_cloud_configuration()
{
    int status;

#if MBED_CONF_APP_DEVELOPER_MODE == 1
    printf("Start developer flow\n");
    status = fcc_developer_flow();
    if (status == FCC_STATUS_KCM_FILE_EXIST_ERROR) {
        printf("Developer credentials already exists\n");
    } else if (status != FCC_STATUS_SUCCESS) {
        printf("Failed to load developer credentials\n");
    }
#endif
    status = fcc_verify_device_configured_4mbed_cloud();
    if (status != FCC_STATUS_SUCCESS) {
        printf("Invalid or missing provisioning data!\n");
        return 1;
    } 
    return 0;
}

static bool application_init_fcc(void)
{
    int status =  mcc_platform_fcc_init();
    if(status != FCC_STATUS_SUCCESS) {
        printf("fcc_init failed with status %d! - exit\n", status);
        return 1;
    }

    status = application_init_verify_cloud_configuration();
    if (status != 0) {
    // This is designed to simplify user-experience by auto-formatting the
    // primary storage if no valid certificates exist.
    // This should never be used for any kind of production devices.
#ifndef MBED_CONF_APP_MCC_NO_AUTO_FORMAT
        if (mcc_platform_reformat_storage() != 0) {
            return 1;
        }
        status = mcc_platform_reset_storage();
        if (status != FCC_STATUS_SUCCESS) {
            return 1;
        }
        status = application_init_verify_cloud_configuration();
        if (status != 0) {
            return 1;
        }        
#else
        return 1;
#endif
    }

    return 0;
}

bool application_init(void)
{
    if (application_init_mbed_trace() != 0) {
        printf("Failed initializing mbed trace\n" );
        return false;
    }

    if (mcc_platform_storage_init() != 0) {
        printf("Failed to initialize storage\n" );
        return false;
    }

    if(mcc_platform_init() != 0) {
       printf("ERROR - platform_init() failed!\n");
       return false;
    }

    if(mcc_platform_init_button_and_led() != 0) {
       printf("ERROR - initButtonAndLed() failed!\n");
       return false;
    }

    // Print some statistics of the object sizes and heap memory consumption
    // if the MBED_HEAP_STATS_ENABLED is defined.
#ifdef MBED_HEAP_STATS_ENABLED
    print_m2mobject_stats();
    print_heap_stats();
#endif
    printf("Start simple mbed Cloud Client\n");

    if (application_init_fcc() != 0) {
        printf("Failed initializing FCC\n" );
        return false;
    }

    return true;
}
