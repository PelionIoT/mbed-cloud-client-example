// ----------------------------------------------------------------------------
// Copyright 2016-2021 Pelion.
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

// Needed for PRIu64 on FreeRTOS
#include <stdio.h>

// Note: this macro is needed on armcc to get the the limit macros like UINT16_MAX
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#if defined (__RTX)
#include "pdmc_main.h"
#endif

#ifdef TARGET_LIKE_MBED
#include "mbed.h"
#endif

#if defined(MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS) && \
 (MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS == 1) && \
 defined(MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION) && \
 (MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION == 1)
#define USE_EVENT_QUEUE
#include "nanostack-event-loop/eventOS_scheduler.h"
#endif

#ifdef MEMORY_TESTS_HEAP
#include "memory_tests.h"
#endif

#include "pdmc_example.h"
#include "application_init.h"
#include "mcc_common_setup.h"

static void main_application(void);

#if defined MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER && \
 (MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER == 1)
#include "NetworkManager.h"
static NetworkManager network_manager;
#endif

#if defined(MBED_CLOUD_APPLICATION_NONSTANDARD_ENTRYPOINT)
extern "C"
int mbed_cloud_application_entrypoint(void)
#else
int main(void)
#endif //MBED_CLOUD_APPLICATION_NONSTANDARD_ENTRYPOINT
{
    return mcc_platform_run_program(main_application);
}

void main_application(void)
{
#if defined(__linux__) && (MBED_CONF_MBED_TRACE_ENABLE == 0)
    // make sure the line buffering is on as non-trace builds do
    // not produce enough output to fill the buffer
    setlinebuf(stdout);
#endif

    // Initialize trace-library first
    if (!application_init_mbed_trace()) {
        printf("Failed initializing mbed trace\r\n");
        return;
    }

    // Initialize storage
    if (mcc_platform_storage_init() != 0) {
        printf("Failed to initialize storage\r\n");
        return;
    }

    // Initialize platform-specific components
    if (mcc_platform_init() != 0) {
        printf("ERROR - platform_init() failed!\r\n");
        return;
    }

    // Print some statistics of the object sizes and their heap memory consumption.
    // NOTE: This *must* be done before creating MbedCloudClient, as the statistic calculation
    // creates and deletes M2MSecurity and M2MDevice singleton objects, which are also used by
    // the MbedCloudClient.
#ifdef MEMORY_TESTS_HEAP
    print_m2mobject_stats();
#endif

    /*
     * Pre-initialize network stack and client library.
     *
     * Specifically for nanostack mesh networks on Mbed OS platform it is important to initialize
     * the components in correct order to avoid out-of-memory issues in Device Management Client initialization.
     * The order for these use cases should be:
     * 1. Initialize network stack using `nsapi_create_stack()` (Mbed OS only). // Implemented in `mcc_platform_interface_init()`.
     * 2. Initialize Device Management Client using `init()`.                   // Implemented in `mbedClient.init()`.
     * 3. Connect to network interface using 'connect()`.                       // Implemented in `mcc_platform_interface_connect()`.
     * 4. Connect Device Management Client to service using `setup()`.          // Implemented in `mbedClient.register_and_connect)`.
     */
    (void) mcc_platform_interface_init();

    // application_init() runs the following initializations:
    //  1. platform initialization
    //  2. print memory statistics if MEMORY_TESTS_HEAP is defined
    //  3. FCC initialization.
    if (!application_init()) {
        printf("Initialization failed, exiting application!\r\n");
        return;
    }

    pdmc_init();

#if defined MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER &&\
 (MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER == 1)
    if (network_manager.configure_factory_mac_address(NetworkInterface::get_default_instance()) != NM_ERROR_NONE) {
        printf("Failed: configure_factory_mac_address\n");
        return;
    }
    printf("Configuring Interface\r\n");
    if (network_manager.reg_and_config_iface(NetworkInterface::get_default_instance()) != NM_ERROR_NONE) {
        printf("Failed to register and configure Interface\r\n");
        return;
    }
#endif

    // Print platform information
    mcc_platform_sw_build_info();

    // Initialize network
    int timeout_ms = 5000;
    int retry_counter = 0;
    while (-1 == mcc_platform_interface_connect()) {
        // Will try to connect using mcc_platform_interface_connect forever.
        // wait timeout is always doubled
        printf("Network connect failed. Try again after %d milliseconds.\n", timeout_ms);
        mcc_platform_do_wait(timeout_ms);
        timeout_ms *= 2;

        if (++retry_counter == MAX_PDMC_CLIENT_CONNECTION_ERROR_COUNT) {
            printf("Max error count %d reached, rebooting.\n\n", MAX_PDMC_CLIENT_CONNECTION_ERROR_COUNT);
            mcc_platform_do_wait(1000);
            mcc_platform_reboot();
        }
    }
    printf("Network initialized, registering...\r\n");

#ifdef MEMORY_TESTS_HEAP
    printf("Client initialized\r\n");
    print_heap_stats();
#endif

#if defined MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER &&\
 (MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER == 1)
    network_manager.create_resource(pdmc_get_object_list());
#endif

#ifndef PDMC_EXAMPLE_MINIMAL
    if (!create_pdmc_resources()) {
        printf("Failed to create resources\r\n");
        return;
    }
#endif

    pdmc_connect();

#ifdef USE_EVENT_QUEUE
    printf("Starting mbed eventloop...\r\n");

    eventOS_scheduler_mutex_wait();

    EventQueue *queue = mbed::mbed_event_queue();
    queue->dispatch_forever();
#else

#if defined MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER &&\
 (MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER == 1)
    // Wait untill client is registered.
    while (pdmc_registered() == false) {
        mcc_platform_do_wait(100);
    }
    network_manager.nm_cloud_client_connect_indication();
#endif

    // Check if client is registering or registered, if true sleep and repeat.
    while (pdmc_register_called()) {
        mcc_platform_do_wait(100);
    }

    // Client unregistered, disconnect and exit program.
    mcc_platform_interface_close();
#endif
}
