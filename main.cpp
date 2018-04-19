// ----------------------------------------------------------------------------
// Copyright 2016-2018 ARM Ltd.
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

#include "simplem2mclient.h"
#ifdef TARGET_LIKE_MBED
#include "mbed.h"
#endif
#include "application_init.h"
#include "common_button_and_led.h"

static void main_application(void);

int main(void)
{
    mcc_platform_run_program(main_application);
}

// Pointers to the resources that will be created in main_application().
static M2MResource* button_res;
static M2MResource* pattern_res;
static M2MResource* blink_res;

// Pointer to mbedClient, used for calling close function.
static SimpleM2MClient *client;

void pattern_updated(const char *)
 {
    printf("PUT received, new value: %s\n", pattern_res->get_value_string().c_str());
}

void blink_callback(void *) {
    String pattern_string = pattern_res->get_value_string();
    const char *pattern = pattern_string.c_str();
    printf("LED pattern = %s\n", pattern);
    // The pattern is something like 500:200:500, so parse that.
    // LED blinking is done while parsing.
    mcc_platform_toggle_led();
    while (*pattern != '\0') {
        // Wait for requested time.
        mcc_platform_do_wait(atoi(pattern));
        mcc_platform_toggle_led();
        // Search for next value.
        pattern = strchr(pattern, ':');
        if(!pattern) {
            break; // while
        }
        pattern++;
    }
    mcc_platform_led_off();
}

void button_notification_status_callback(const M2MBase& object, const NoticationDeliveryStatus status)
{
    switch(status) {
        case NOTIFICATION_STATUS_BUILD_ERROR:
            printf("Notification callback: (%s) error when building CoAP message\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_RESEND_QUEUE_FULL:
            printf("Notification callback: (%s) CoAP resend queue full\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_SENT:
            printf("Notification callback: (%s) Notification sent to server\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_DELIVERED:
            printf("Notification callback: (%s) Notification delivered\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_SEND_FAILED:
            printf("Notification callback: (%s) Notification sending failed\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_SUBSCRIBED:
            printf("Notification callback: (%s) subscribed\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_UNSUBSCRIBED:
            printf("Notification callback: (%s) subscription removed\n", object.uri_path());
            break;
        default:
            break;
    }
}

// This function is called when a POST request is received for resource 5000/0/1.
void unregister(void *)
{
    printf("Unregister resource executed\n");
    client->close();
}

// This function is called when a POST request is received for resource 5000/0/2.
void factory_reset(void *)
{
    printf("Factory reset resource executed\n");
    client->close();
    kcm_status_e kcm_status = kcm_factory_reset();
    if (kcm_status != KCM_STATUS_SUCCESS) {
        printf("Failed to do factory reset - %d\n", kcm_status);
    } else {
        printf("Factory reset completed. Now restart the device\n");
    }
}

void main_application(void)
{
    mcc_platform_sw_build_info();
    // run_application() will first initialize the program and then call main_application()

    // application_init() runs the following initializations:
    //  1. trace initialization
    //  2. platform initialization
    //  3. print memory statistics if MBED_HEAP_STATS_ENABLED is defined
    //  4. FCC initialization.
    if (!application_init()) {
        printf("Initialization failed, exiting application!\n");
        return;
    }

    // SimpleClient is used for registering and unregistering resources to a server.
    SimpleM2MClient mbedClient;

    // Save pointer to mbedClient so that other functions can access it.
    client = &mbedClient;

#ifdef MBED_HEAP_STATS_ENABLED
    printf("Client initialized\r\n");
    print_heap_stats();
#endif

    // Create resource for button count. Path of this resource will be: 3200/0/5501.
    button_res = mbedClient.add_cloud_resource(3200, 0, 5501, "button_resource", M2MResourceInstance::INTEGER,
                              M2MBase::GET_ALLOWED, 0, true, NULL, (void*)button_notification_status_callback);

    // Create resource for led blinking pattern. Path of this resource will be: 3201/0/5853.
    pattern_res = mbedClient.add_cloud_resource(3201, 0, 5853, "pattern_resource", M2MResourceInstance::STRING,
                               M2MBase::GET_PUT_ALLOWED, "500:500:500:500", false, (void*)pattern_updated, NULL);

    // Create resource for starting the led blinking. Path of this resource will be: 3201/0/5850.
    blink_res = mbedClient.add_cloud_resource(3201, 0, 5850, "blink_resource", M2MResourceInstance::STRING,
                             M2MBase::POST_ALLOWED, "", false, (void*)blink_callback, NULL);

    // Create resource for unregistering the device. Path of this resource will be: 5000/0/1.
    mbedClient.add_cloud_resource(5000, 0, 1, "unregister", M2MResourceInstance::STRING,
                 M2MBase::POST_ALLOWED, NULL, false, (void*)unregister, NULL);

    // Create resource for running factory reset for the device. Path of this resource will be: 5000/0/2.
    mbedClient.add_cloud_resource(5000, 0, 2, "factory_reset", M2MResourceInstance::STRING,
                 M2MBase::POST_ALLOWED, NULL, false, (void*)factory_reset, NULL);

    mbedClient.register_and_connect();

    // Check if client is registering or registered, if true sleep and repeat.
    while (mbedClient.is_register_called()) {
        static int button_count = 0;
        mcc_platform_do_wait(100);
        if (mcc_platform_button_clicked()) {
            button_res->set_value(++button_count);
        }
    }

    // Client unregistered, exit program.
}
