// ----------------------------------------------------------------------------
// Copyright 2015-2020 ARM Ltd.
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

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "mbed.h"
#include "mcc_common_setup.h"
#include "mcc_common_config.h"
#include "kv_config.h"
#include "mbed_trace.h"
#include "MbedCloudClientConfig.h"

#if MBED_MAJOR_VERSION > 5
#include "DeviceKey.h"
#include <chrono>
#endif


#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
#include "MeshInterfaceNanostack.h"
#include "multicast.h"

static int8_t get_mesh_iface_id();
#endif // MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE

#define TRACE_GROUP "plat"

#define SECONDS_TO_MS 1000  // to avoid using floats, wait() uses floats

#ifndef MCC_PLATFORM_WAIT_BEFORE_BD_INIT
#define MCC_PLATFORM_WAIT_BEFORE_BD_INIT 2
#endif

#if defined(MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS) && \
 (MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS == 1) && \
 defined(MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION) && \
 (MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION == 1)
#define MCC_USE_MBED_EVENTS
#endif

/* local help functions. */
const char *network_type(NetworkInterface *iface);

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////

static NetworkInterface *network_interface = NULL;

/*
 * Callback function which informs about status of connected NetworkInterface.
 */
static void network_status_callback(nsapi_event_t status, intptr_t param);

/*
 * Note: `interface_connected` flag is used both via callbacks, and via blocking API
 * calls. For well-behaving network stacks we should be able to relay on callbacks
 * alone, but some network stacks might not implement network callbacks correctly,
 * thus this ensures that also such stacks are handled correctly.
 */
static bool interface_connected = false;

#ifdef MCC_USE_MBED_EVENTS
static bool volatile async_connect = false;
#endif

////////////////////////////////
// SETUP_COMMON.H IMPLEMENTATION
////////////////////////////////

void mcc_platform_interface_init(void)
{
    network_interface = NetworkInterface::get_default_instance();
    if (network_interface == NULL) {
        printf("ERROR: No NetworkInterface found!\n");
        return;
    }
    if (!nsapi_create_stack(network_interface)) {
        printf("ERROR: nsapi_create_stack() failed!\n");
        return;
    }
}

int mcc_platform_init_connection(void)
{
    return mcc_platform_interface_connect();
}

void *mcc_platform_get_network_interface(void)
{
    return mcc_platform_interface_get();
}

int mcc_platform_close_connection(void)
{
    return mcc_platform_interface_close();
}

int mcc_platform_interface_connect(void)
{
// Perform number of retries if network init fails.
#ifndef MCC_PLATFORM_CONNECTION_RETRY_COUNT
#define MCC_PLATFORM_CONNECTION_RETRY_COUNT 5
#endif
#ifndef MCC_PLATFORM_CONNECTION_RETRY_TIMEOUT
#define MCC_PLATFORM_CONNECTION_RETRY_TIMEOUT 1000
#endif
    SocketAddress sa;
    nsapi_error_t err;

    printf("mcc_platform_interface_connect()\n");
    network_interface = NetworkInterface::get_default_instance();
    if (network_interface == NULL) {
        printf("ERROR: No NetworkInterface found!\n");
        return -1;
    }
    // Delete the callback first in case the API is being called multiple times to prevent creation of multiple callbacks.
    network_interface->remove_event_listener(mbed::callback(&network_status_callback));
    network_interface->add_event_listener(mbed::callback(&network_status_callback));
    printf("Connecting with interface: %s\n", network_type(network_interface));
    interface_connected = false;

#ifdef MCC_USE_MBED_EVENTS
    bool blocking_interface = false;

    if (network_interface->set_blocking(false) != NSAPI_ERROR_OK) {
        printf("WARN: Could not set non-blocking interface\n");
        blocking_interface = true;
    }

    if (!blocking_interface) {
        for (int i = 1; i <= MCC_PLATFORM_CONNECTION_RETRY_COUNT; i++) {
            err = network_interface->connect();
            printf("network_interface->connect(): %d\n", err);
            if (err == NSAPI_ERROR_IS_CONNECTED) {
                interface_connected = true;
            } else if (err != NSAPI_ERROR_OK) {
                return -1;
            } else {
                async_connect = true;
                mbed_event_queue()->dispatch_forever();
            }

            if (interface_connected) {
                err = network_interface->get_ip_address(&sa);
                if (err != NSAPI_ERROR_OK) {
                    printf("get_ip_address() - failed, status %d\n", err);
                    interface_connected = false;
                } else {
                    printf("IP: %s\n", (sa.get_ip_address() ? sa.get_ip_address() : "None"));
                    printf("MAC address: %s\n", (network_interface->get_mac_address() ? network_interface->get_mac_address() : "None"));
#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
                    int8_t iface_id = get_mesh_iface_id();
                    if (iface_id == -1) {
                        return -1;
                    }
                    arm_uc_multicast_interface_configure(iface_id);
#endif // MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
                    return 0;
                }
            }
            printf("Failed to connect! Retry %d/%d\n", i, MCC_PLATFORM_CONNECTION_RETRY_COUNT);
            err = network_interface->disconnect();
            printf("network_interface->disconnect(): %d\n", err);
            mcc_platform_do_wait(MCC_PLATFORM_CONNECTION_RETRY_TIMEOUT * i);
        }
    } else {
        for (int i = 1; i <= MCC_PLATFORM_CONNECTION_RETRY_COUNT; i++) {
            err = network_interface->connect();
            if (err == NSAPI_ERROR_OK || err == NSAPI_ERROR_IS_CONNECTED) {
                err = network_interface->get_ip_address(&sa);
                if (err != NSAPI_ERROR_OK) {
                    printf("get_ip_address() - failed, status %d\n", err);
                } else {
                    printf("IP: %s\n", (sa.get_ip_address() ? sa.get_ip_address() : "None"));
                    printf("MAC address: %s\n", (network_interface->get_mac_address() ? network_interface->get_mac_address() : "None"));
#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
                    int8_t iface_id = get_mesh_iface_id();
                    if (iface_id == -1) {
                        return -1;
                    }
                    arm_uc_multicast_interface_configure(iface_id);
#endif // MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
                    interface_connected = true;
                    return 0;
                }
            }
            printf("Failed to connect! error=%d. Retry %d/%d\n", err, i, MCC_PLATFORM_CONNECTION_RETRY_COUNT);
            err = network_interface->disconnect();
            printf("network_interface->disconnect(): %d\n", err);
            mcc_platform_do_wait(MCC_PLATFORM_CONNECTION_RETRY_TIMEOUT * i);
        }
    }
#else
    for (int i = 1; i <= MCC_PLATFORM_CONNECTION_RETRY_COUNT; i++) {
        err = network_interface->connect();
        if (err == NSAPI_ERROR_OK || err == NSAPI_ERROR_IS_CONNECTED) {
            err = network_interface->get_ip_address(&sa);
            if (err != NSAPI_ERROR_OK) {
                printf("get_ip_address() - failed, status %d\n", err);
            } else {
                printf("IP: %s\n", (sa.get_ip_address() ? sa.get_ip_address() : "None"));
                printf("MAC address: %s\n", (network_interface->get_mac_address() ? network_interface->get_mac_address() : "None"));
#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
                int8_t iface_id = get_mesh_iface_id();
                if (iface_id == -1) {
                    return -1;
                }
                arm_uc_multicast_interface_configure(iface_id);
#endif // MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
                interface_connected = true;
                return 0;
            }
        }
        printf("Failed to connect! error=%d. Retry %d/%d\n", err, i, MCC_PLATFORM_CONNECTION_RETRY_COUNT);
        err = network_interface->disconnect();
        printf("network_interface->disconnect(): %d\n", err);
        mcc_platform_do_wait(MCC_PLATFORM_CONNECTION_RETRY_TIMEOUT * i);
    }
#endif

    return -1;
}

int mcc_platform_interface_close(void)
{

    if (network_interface) {
        nsapi_error_t err = network_interface->disconnect();
        if (err == NSAPI_ERROR_OK) {
            network_interface->remove_event_listener(mbed::callback(&network_status_callback));
            network_interface = NULL;
            interface_connected = false;
            return 0;
        }
    }
    return -1;
}

void *mcc_platform_interface_get(void)
{
    if (interface_connected) {
        return network_interface;
    }
    return NULL;
}

void network_status_callback(nsapi_event_t status, intptr_t param)
{
    if (status == NSAPI_EVENT_CONNECTION_STATUS_CHANGE) {
        switch (param) {
            case NSAPI_STATUS_GLOBAL_UP:
                interface_connected = true;
#ifdef MCC_USE_MBED_EVENTS
                if (async_connect) {
                    async_connect = false;
                    mbed_event_queue()->break_dispatch();
                }
#endif
#if MBED_CONF_MBED_TRACE_ENABLE
                tr_info("NSAPI_STATUS_GLOBAL_UP");
#else
                printf("NSAPI_STATUS_GLOBAL_UP\n");
#endif
                break;
            case NSAPI_STATUS_LOCAL_UP:
#if MBED_CONF_MBED_TRACE_ENABLE
                tr_info("NSAPI_STATUS_LOCAL_UP");
#else
                printf("NSAPI_STATUS_LOCAL_UP\n");
#endif
                break;
            case NSAPI_STATUS_DISCONNECTED:
                interface_connected = false;
#if MBED_CONF_MBED_TRACE_ENABLE
                tr_info("NSAPI_STATUS_DISCONNECTED");
#else
                printf("NSAPI_STATUS_DISCONNECTED\n");
#endif
#ifdef MCC_USE_MBED_EVENTS
                if (async_connect) {
                    async_connect = false;
                    mbed_event_queue()->break_dispatch();
                }
#endif
                break;
            case NSAPI_STATUS_CONNECTING:
#if MBED_CONF_MBED_TRACE_ENABLE
                tr_info("NSAPI_STATUS_CONNECTING");
#else
                printf("NSAPI_STATUS_CONNECTING\n");
#endif
                break;
            case NSAPI_STATUS_ERROR_UNSUPPORTED:
            default:
#if MBED_CONF_MBED_TRACE_ENABLE
                tr_info("NSAPI_STATUS_ERROR_UNSUPPORTED");
#else
                printf("NSAPI_STATUS_ERROR_UNSUPPORTED\n");
#endif
                break;
        }
    }
}

#define xstr(s) str(s)
#define str(s) #s

const char *network_type(NetworkInterface *iface)
{
    if (iface->ethInterface()) {
        return "Ethernet";
    } else if (iface->wifiInterface()) {
        return "Wi-Fi " xstr(MBED_CONF_NSAPI_DEFAULT_WIFI_SSID);
    } else if (iface->meshInterface()) {
        return "Mesh";
    } else if (iface->cellularInterface()) {
        return "Cellular";
    } else if (iface->emacInterface()) {
        return "Emac";
    } else {
        return "Unknown";
    }
}

int mcc_platform_init(void)
{
    // On CortexM (3 and 4) the MCU has a write buffer, which helps in performance front,
    // but has a side effect of making data access faults imprecise.
    //
    // So, if one gets a Mbed OS crash dump with following content, a re-build with
    // "PLATFORM_DISABLE_WRITE_BUFFER=1" will help in getting the correct crash location.
    //
    // --8<---
    // Crash Info:
    // <..>
    //    Target and Fault Info:
    //          Forced exception, a fault with configurable priority has been escalated to HardFault
    //          Imprecise data access error has occurred
    // --8<---
    //
    // This can't be enabled by default as then we would test with different system setup than customer
    // and possible OS and driver issues might get pass the tests.
    //
#if defined(PLATFORM_DISABLE_WRITE_BUFFER) && (PLATFORM_DISABLE_WRITE_BUFFER==1)

#if defined(TARGET_CORTEX_M)

    SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;

    tr_info("mcc_platform_init: disabled CPU write buffer, expect reduced performance");
#else
    tr_info("mcc_platform_init: disabling CPU write buffer not possible or needed on this MCU");
#endif

#endif

    return 0;
}

void mcc_platform_do_wait(int timeout_ms)
{
#if MBED_MAJOR_VERSION > 5
    ThisThread::sleep_for(std::chrono::milliseconds(timeout_ms));
#else
    ThisThread::sleep_for(timeout_ms);
#endif
}

int mcc_platform_run_program(main_t mainFunc)
{
    mainFunc();
    return 1;
}

void mcc_platform_sw_build_info(void)
{
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\n");
    printf("PDMC version %d.%d.%d\n", PDMC_MAJOR_VERSION, PDMC_MINOR_VERSION, PDMC_PATCH_VERSION);

    // The Mbed OS' master branch does not define the version numbers at all, so we need
    // some ifdeffery to keep compilations running.
#if defined(MBED_MAJOR_VERSION) && defined(MBED_MINOR_VERSION) && defined(MBED_PATCH_VERSION)
    printf("Mbed OS version %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#else
    printf("Mbed OS version <UNKNOWN>\n");
#endif
}

int mcc_platform_storage_init(void)
{
    // This wait will allow the board more time to initialize
    mcc_platform_do_wait(MCC_PLATFORM_WAIT_BEFORE_BD_INIT * SECONDS_TO_MS);
    // Normally, TF-M V8M targets don't support KVStore/FLASHIAP and change to PSA Storage API instead.
#if DEVICE_FLASH
    int status = kv_init_storage_config();
    if (status != MBED_SUCCESS) {
#ifdef MBED_CONF_MBED_CLOUD_CLIENT_PSA_SUPPORT
        if (status == MBED_ERROR_UNSUPPORTED) {
            printf("PSA enabled, ignore kv_init_storage_config() is not supported\n");
        } else
#endif
        {
            printf("kv_init_storage_config() - failed, status %d\n", status);
            return status;
        }
    }
#endif
    return 0;
}

void mcc_platform_reboot(void)
{
    NVIC_SystemReset();
}

#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
static int8_t get_mesh_iface_id()
{
    NetworkInterface *mesh_interface = MeshInterface::get_default_instance();
    if (mesh_interface == NULL) {
        printf("Failed to get default MeshInterface\n");
        return -1;
    }

    InterfaceNanostack *nano_mesh_if = reinterpret_cast<InterfaceNanostack *>(mesh_interface);
    int8_t mesh_if_id = nano_mesh_if->get_interface_id();
    if (mesh_if_id < 0) {
        return -1;
    }

    printf("Mesh Interface ID: %d\n", mesh_if_id);
    return mesh_if_id;
}
#endif // MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
