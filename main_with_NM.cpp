// ----------------------------------------------------------------------------
// Copyright 2016-2020 ARM Ltd.
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
#if defined MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER && (MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER == 1)
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
#include "simplem2mclient.h"
#ifdef TARGET_LIKE_MBED
#include "mbed.h"
#endif
#include "application_init.h"
#include "mcc_common_button_and_led.h"
#include "blinky.h"
#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
#include "certificate_enrollment_user_cb.h"
#endif

#if defined(MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS) && \
 (MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS == 1) && \
 defined(MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION) && \
 (MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION == 1)
#include "nanostack-event-loop/eventOS_scheduler.h"
#endif

#include "mbed-trace/mbed_trace.h"             // Required for mbed_trace_*
#include "mesh_system.h"
#include "network_manager_api.h"

#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
#include "multicast.h"
#endif

#define TRACE_GROUP "app"

// event based LED blinker, controlled via pattern_resource
#ifndef MCC_MEMORY
static Blinky blinky;
#endif

// Pointers to the resources that will be created in main_application().
static M2MResource *button_res;
static M2MResource *pattern_res;
static M2MResource *blink_res;
static M2MResource *unregister_res;
static M2MResource *factory_reset_res;

// Pointer to mbedClient, used for calling close function.
static SimpleM2MClient *client;
void counter_updated(const char *);
void pattern_updated(const char *);
void notification_status_callback(const M2MBase &object,
                                  const M2MBase::MessageDeliveryStatus status,
                                  const M2MBase::MessageType /*type*/);
void sent_callback(const M2MBase &base,
                   const M2MBase::MessageDeliveryStatus status,
                   const M2MBase::MessageType /*type*/);
void unregister_triggered(void);
void factory_reset_triggered(void *);
// This function is called when a POST request is received for resource 5000/0/1.
static void app_client_register_and_connect(void);

static void blink_callback(void *)
{
    String pattern_string = pattern_res->get_value_string();
    printf("POST executed\r\n");

    // The pattern is something like 500:200:500, so parse that.
    // LED blinking is done while parsing.
#ifndef MCC_MEMORY
    const bool restart_pattern = false;
    if (blinky.start((char *)pattern_res->value(), pattern_res->value_length(), restart_pattern) == false) {
        printf("out of memory error\r\n");
    }
#endif
    blink_res->send_delayed_post_response();
}

/* Callback handler from nm interface */
void app_nm_indication_handler(uint8_t msg_type, void *msg)
{
    switch (msg_type) {
        /* This Event is posted from network_manager_api.cpp file when
         * All interfaces connect confirmation comes to network manager
         */
        case NM_CONNECTED:
            /*Set Mesh Interface is up*/
            if (msg != NULL) {
                mcc_platform_set_network_interface(msg);
            } else {
                tr_err("%s :: Network Interface could not found ", __func__);
            }
#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
            arm_uc_multicast_interface_configure(1);
#endif
            app_client_register_and_connect();
            break;
        /* This Event will be posted from network_manager_api.cpp file when
         * network manager is in running state.
         */
        case NM_INIT_CONF:
            printf("Network manager started\n");
            break;
        default :
            /* Unknown event posted */
            tr_info("Unknown msg_type received");
            break;
    }
}

void main_application_with_nm(void)
{
#if defined(__linux__) && (MBED_CONF_MBED_TRACE_ENABLE == 0)
    // make sure the line buffering is on as non-trace builds do
    // not produce enough output to fill the buffer
    setlinebuf(stdout);
#endif

    // Initialize trace-library first
    if (application_init_mbed_trace() != 0) {
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

    // SimpleClient is used for registering and unregistering resources to a server.
    SimpleM2MClient mbedClient;

    // Save pointer to mbedClient so that other functions can access it.
    client = &mbedClient;

    mesh_system_init();

    mbedClient.init();

    // application_init() runs the following initializations:
    //  1. platform initialization
    //  2. print memory statistics if MEMORY_TESTS_HEAP is defined
    //  3. FCC initialization.
    if (!application_init()) {
        printf("Initialization failed, exiting application!\r\n");
        return;
    }

    // Print platform information
    mcc_platform_sw_build_info();

#ifndef MCC_MEMORY
    // Create resource for button count. Path of this resource will be: 3200/0/5501.
    button_res = mbedClient.add_cloud_resource(3200, 0, 5501, "button_resource", M2MResourceInstance::INTEGER,
                                               M2MBase::GET_PUT_ALLOWED, 0, true, (void *)counter_updated, (void *)notification_status_callback);
    button_res->set_value(0);

    // Create resource for led blinking pattern. Path of this resource will be: 3201/0/5853.
    pattern_res = mbedClient.add_cloud_resource(3201, 0, 5853, "pattern_resource", M2MResourceInstance::STRING,
                                                M2MBase::GET_PUT_ALLOWED, "500:500:500:500", true, (void *)pattern_updated, (void *)notification_status_callback);

    // Create resource for starting the led blinking. Path of this resource will be: 3201/0/5850.
    blink_res = mbedClient.add_cloud_resource(3201, 0, 5850, "blink_resource", M2MResourceInstance::STRING,
                                              M2MBase::POST_ALLOWED, "", false, (void *)blink_callback, (void *)notification_status_callback);
    // Use delayed response
    blink_res->set_delayed_response(true);

    // Create resource for unregistering the device. Path of this resource will be: 5000/0/1.
    unregister_res = mbedClient.add_cloud_resource(5000, 0, 1, "unregister", M2MResourceInstance::STRING,
                                                   M2MBase::POST_ALLOWED, NULL, false, (void *)unregister_triggered, (void *)sent_callback);
    unregister_res->set_delayed_response(true);

    // Create optional Device resource for running factory reset for the device. Path of this resource will be: 3/0/5.
    factory_reset_res = M2MInterfaceFactory::create_device()->create_resource(M2MDevice::FactoryReset);
    if (factory_reset_res) {
        factory_reset_res->set_execute_function(factory_reset_triggered);
    }

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
    button_res->set_auto_observable(true);
    pattern_res->set_auto_observable(true);
    blink_res->set_auto_observable(true);
    unregister_res->set_auto_observable(true);
    factory_reset_res->set_auto_observable(true);
#endif

#endif

    /* Register callback to n/m manager to get notification from n/w manager to application */
    nm_application_cb(app_nm_indication_handler);

    M2MObjectList *m2m_obj_list = mbedClient.get_m2m_obj_list();
    /* Network manager initialization */
    /* It initialize mesh interface */
    nm_init(m2m_obj_list);

    nm_connect();

    while (mbedClient.is_client_registered() == false) {
        mcc_platform_do_wait(100);
    }

    tr_info("Notifying Network Manager: PDMC Connected");
    nm_cloud_client_connect_notification();

    // Check if client is registering or registered, if true sleep and repeat.
    while (mbedClient.is_register_called()) {
        mcc_platform_do_wait(100);
    }

    // Client unregistered, disconnect and exit program.
    mcc_platform_interface_close();
}

static void app_client_register_and_connect(void)
{
    /* Device registration to Device Management after the network formation. */
    client->register_and_connect();

#ifndef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
#ifndef MCC_MEMORY
    blinky.init(*client, button_res);
    blinky.request_next_loop_event();
    blinky.request_automatic_increment_event();
#endif
#endif


#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
    // Add certificate renewal callback
    client->get_cloud_client().on_certificate_renewal(certificate_renewal_cb);
#endif // MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT

#if defined(MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS) && \
     (MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS == 1) && \
     defined(MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION) && \
     (MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION == 1)
    printf("Starting mbed eventloop...\r\n");

    eventOS_scheduler_mutex_wait();

    EventQueue *queue = mbed::mbed_event_queue();
    queue->dispatch_forever();
#endif
}
#endif    //MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER && (MBED_CONF_MBED_CLOUD_CLIENT_NETWORK_MANAGER == 1)

