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

#include <stdio.h>

#include "pdmc_example.h"
#include "mcc_common_setup.h"
#include "MbedCloudClient.h"
#include "m2mresource.h"
#include "m2minterfacefactory.h"
#include "key_config_manager.h"
#include "factory_configurator_client.h"
#include "mbed-client/m2minterface.h"

#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
#include "certificate_enrollment_user_cb.h"
#endif

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
#include "update_ui_example.h"
#endif

#if defined (MEMORY_TESTS_HEAP)
#include "memory_tests.h"
#endif

// This defines how many consecutive errors can occur before application will perform reboot as recovery.
// Set to 0 to disable the feature.
#ifndef MAX_ERROR_COUNT
#define MAX_ERROR_COUNT 5
#endif

// Resource callback functions
static void button_counter_updated(const char *);
static void blink_pattern_updated(const char *);
static void blink_cb(void *);
static void factory_reset_cb(void *);
static void delivery_status_cb(const M2MBase &object, const M2MBase::MessageDeliveryStatus status, const M2MBase::MessageType type);
static void large_res_sent_cb(const M2MBase &base, const M2MBase::MessageDeliveryStatus status, const M2MBase::MessageType type);
static coap_response_code_e large_res_read_requested(const M2MResourceBase &resource, uint8_t *&buffer, size_t &buffer_size, size_t &total_size, const size_t offset, void *client_args);

// PDMC callback functions
static void pdmc_error_handler(int error_code);
static void pdmc_status_handler(int status);

#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
static void pdmc_external_update_cb(uint32_t start_address, uint32_t size);
#endif // MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE

// Helper methods
static void reboot_if_threshold_value(int threshold);

// Global variables
#ifndef PDMC_EXAMPLE_MINIMAL
#include "blinky.h"
static Blinky blinky;
#endif

static MbedCloudClient pdmc_client;
static M2MObjectList object_list;
static bool register_called = false;
static bool registered = false;
static int error_count = 0;
volatile bool paused = false;
static uint8_t *large_res_data = NULL;
const static int16_t large_res_size = 2049;

// Pointers to the resources that will be created in main_application().
static M2MResource *button_res;
static M2MResource *pattern_res;
static M2MResource *blink_res;
static M2MResource *factory_reset_res;
static M2MResource *large_res;

void pdmc_init()
{
    pdmc_client.init();
}

void pdmc_close()
{
    pdmc_client.close();
}

void pdmc_resume()
{
    paused = false;
    int timeout_ms = 5000;
    while (-1 == mcc_platform_interface_connect()) {
        // Will try to connect using mcc_platform_interface_connect until error count and then does reboot if
        // not successful. Wait timeout is always doubled
        printf("Network did not recover after pause. Try again after %d milliseconds.\n", timeout_ms);
        mcc_platform_do_wait(timeout_ms);
        timeout_ms *= 2;
        reboot_if_threshold_value(MAX_PDMC_CLIENT_CONNECTION_ERROR_COUNT);
    }

    pdmc_client.resume(mcc_platform_get_network_interface());
}

bool pdmc_connect()
{
#if defined (MEMORY_TESTS_HEAP) && !defined(PDMC_EXAMPLE_MINIMAL)
    // Add some test resources to measure memory consumption.
    // This code is activated only if MEMORY_TESTS_HEAP is defined.
    create_m2mobject_test_set(object_list);
#endif

    pdmc_client.add_objects(object_list);

    pdmc_client.on_error(&pdmc_error_handler);
    pdmc_client.on_status_changed(&pdmc_status_handler);

#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
    pdmc_client.on_external_update(pdmc_external_update_cb);
#endif

#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
    pdmc_client.on_certificate_renewal(certificate_renewal_cb);
#endif

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
    /* Set callback functions for authorizing updates and monitoring progress.
       Code is implemented in update_ui_example.cpp
       Both callbacks are completely optional. If no authorization callback
       is set, the update process will procede immediately in each step.
    */
    update_ui_set_cloud_client(&pdmc_client);
    pdmc_client.set_update_authorize_priority_handler(update_authorize_priority_handler);
    pdmc_client.set_update_progress_handler(update_progress);
#endif

    bool setup = pdmc_client.setup(mcc_platform_get_network_interface());
    register_called = true;
    if (!setup) {
        printf("PDMC setup failed\n");
        return false;
    }

#ifndef PDMC_EXAMPLE_MINIMAL
    blinky.init(button_res);
    blinky.request_next_loop_event();
    blinky.request_automatic_increment_event();
#endif

#ifdef MEMORY_TESTS_HEAP
    printf("Register being called\r\n");
    print_heap_stats();
#endif

    return true;
}

M2MObjectList *pdmc_get_object_list()
{
    return &object_list;
}

bool pdmc_registered()
{
    return registered;
}

bool pdmc_paused()
{
    return paused;
}

bool pdmc_register_called()
{
    return register_called;
}

/** PDMC callback functions --> **/

static void pdmc_status_handler(int status)
{
    switch (status) {
        case MbedCloudClient::Registered:
            printf("Client registered\r\n");
            registered = true;
            error_count = 0;
            static const ConnectorClientEndpointInfo *endpoint = NULL;
            if (endpoint == NULL) {
                endpoint = pdmc_client.endpoint_info();
                if (endpoint) {
#if MBED_CONF_APP_DEVELOPER_MODE == 1
                    printf("Endpoint Name: %s\r\n", endpoint->internal_endpoint_name.c_str());
#else
                    printf("Endpoint Name: %s\r\n", endpoint->endpoint_name.c_str());
#endif
                    printf("Device ID: %s\r\n", endpoint->internal_endpoint_name.c_str());
                }
            }
#ifdef MEMORY_TESTS_HEAP
            print_heap_stats();
#endif
            break;

        case MbedCloudClient::Unregistered:
            paused = false;
            registered = false;
            register_called = false;
            printf("Client unregistered - Exiting application\n");
#ifdef MEMORY_TESTS_HEAP
            print_heap_stats();
#endif
            break;

        case MbedCloudClient::RegistrationUpdated:
            printf("Client registration updated\n");
            error_count = 0;
            break;

        case MbedCloudClient::Paused:
            break;

        case MbedCloudClient::AlertMode:
            break;

        case MbedCloudClient::Sleep:
            printf("Pelion client is going to sleep - Pausing the client\r\n");
            pdmc_client.pause();
            mcc_platform_interface_close();
            paused = true;
        default:
            break;
    }
}

static void pdmc_error_handler(int error_code)
{
    const char *error;
    switch (error_code) {
        case MbedCloudClient::ConnectErrorNone:
            error = "MbedCloudClient::ConnectErrorNone";
            break;
        case MbedCloudClient::ConnectAlreadyExists:
            error = "MbedCloudClient::ConnectAlreadyExists";
            break;
        case MbedCloudClient::ConnectBootstrapFailed:
            error = "MbedCloudClient::ConnectBootstrapFailed";
            break;
        case MbedCloudClient::ConnectInvalidParameters:
            error = "MbedCloudClient::ConnectInvalidParameters";
            break;
        case MbedCloudClient::ConnectNotRegistered:
            error = "MbedCloudClient::ConnectNotRegistered";
            break;
        case MbedCloudClient::ConnectTimeout:
            error = "MbedCloudClient::ConnectTimeout";
            break;
        case MbedCloudClient::ConnectNetworkError:
            error = "MbedCloudClient::ConnectNetworkError";
            break;
        case MbedCloudClient::ConnectResponseParseFailed:
            error = "MbedCloudClient::ConnectResponseParseFailed";
            break;
        case MbedCloudClient::ConnectUnknownError:
            error = "MbedCloudClient::ConnectUnknownError";
            break;
        case MbedCloudClient::ConnectMemoryConnectFail:
            error = "MbedCloudClient::ConnectMemoryConnectFail";
            break;
        case MbedCloudClient::ConnectNotAllowed:
            error = "MbedCloudClient::ConnectNotAllowed";
            break;
        case MbedCloudClient::ConnectSecureConnectionFailed:
            error = "MbedCloudClient::ConnectSecureConnectionFailed";
            break;
        case MbedCloudClient::ConnectDnsResolvingFailed:
            error = "MbedCloudClient::ConnectDnsResolvingFailed";
            break;
        case MbedCloudClient::ConnectorFailedToStoreCredentials:
            error = "MbedCloudClient::ConnectorFailedToStoreCredentials";
            break;
        case MbedCloudClient::ConnectorFailedToReadCredentials:
            error = "MbedCloudClient::ConnectorFailedToReadCredentials";
            break;
#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
        case MbedCloudClient::UpdateWarningCertificateNotFound:
            error = "MbedCloudClient::UpdateWarningCertificateNotFound";
            break;
        case MbedCloudClient::UpdateWarningIdentityNotFound:
            error = "MbedCloudClient::UpdateWarningIdentityNotFound";
            break;
        case MbedCloudClient::UpdateWarningCertificateInvalid:
            error = "MbedCloudClient::UpdateWarningCertificateInvalid";
            break;
        case MbedCloudClient::UpdateWarningSignatureInvalid:
            error = "MbedCloudClient::UpdateWarningSignatureInvalid";
            break;
        case MbedCloudClient::UpdateWarningVendorMismatch:
            error = "MbedCloudClient::UpdateWarningVendorMismatch";
            break;
        case MbedCloudClient::UpdateWarningClassMismatch:
            error = "MbedCloudClient::UpdateWarningClassMismatch";
            break;
        case MbedCloudClient::UpdateWarningDeviceMismatch:
            error = "MbedCloudClient::UpdateWarningDeviceMismatch";
            break;
        case MbedCloudClient::UpdateWarningURINotFound:
            error = "MbedCloudClient::UpdateWarningURINotFound";
            break;
        case MbedCloudClient::UpdateWarningRollbackProtection:
            error = "MbedCloudClient::UpdateWarningRollbackProtection";
            break;
        case MbedCloudClient::UpdateWarningUnknown:
            error = "MbedCloudClient::UpdateWarningUnknown";
            break;
        case MbedCloudClient::UpdateErrorWriteToStorage:
            error = "MbedCloudClient::UpdateErrorWriteToStorage";
            break;
        case MbedCloudClient::UpdateErrorInvalidHash:
            error = "MbedCloudClient::UpdateErrorInvalidHash";
            break;
        case MbedCloudClient::UpdateErrorConnection:
            error = "MbedCloudClient::UpdateErrorConnection";
            break;
        case MbedCloudClient::UpdateWarningAuthorizationRejected:
            error = "MbedCloudClient::UpdateWarningAuthorizationRejected";
            break;
        case MbedCloudClient::UpdateWarningAuthorizationUnavailable:
            error = "MbedCloudClient::UpdateWarningAuthorizationUnavailable";
            break;
        case MbedCloudClient::UpdateCertificateInsertion:
            error = "MbedCloudClient::UpdateCertificateInsertion";
            break;
#endif
#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
        case CE_STATUS_INIT_FAILED:
            error = "CE_STATUS_INIT_FAILED";
            break;
#endif // !MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT

        default:
            error = "UNKNOWN";
    }
    printf("\nError occurred : %s\r\n", error);
    printf("Error code : %d\r\n", error_code);
    printf("Error details : %s\r\n", pdmc_client.error_description());

#if defined(MAX_ERROR_COUNT) && (MAX_ERROR_COUNT > 0)
    if (error_code == MbedCloudClient::ConnectNetworkError ||
            error_code == MbedCloudClient::ConnectDnsResolvingFailed ||
            error_code == MbedCloudClient::ConnectSecureConnectionFailed ||
            error_code == MbedCloudClient::ConnectTimeout) {
        reboot_if_threshold_value(MAX_ERROR_COUNT);
    }
#endif
}

#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
static void pdmc_external_update_cb(uint32_t start_address, uint32_t size)
{
    printf("Application received external update, start address: %" PRIX32 ", size: %" PRIu32 " bytes\n", start_address, size);
}
#endif // MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE

/**  Helper methods --> **/

bool create_pdmc_resources()
{
    // Create resource for button count. Path of this resource will be: 3200/0/5501.
    button_res = M2MInterfaceFactory::create_resource(object_list, 3200, 0, 5501, M2MResourceInstance::INTEGER, M2MBase::GET_PUT_ALLOWED);
    if (button_res == NULL) {
        return false;
    }
    button_res->set_value(0);
    button_res->set_observable(true);
    button_res->set_message_delivery_status_cb((void(*)(const M2MBase &, const M2MBase::MessageDeliveryStatus, const M2MBase::MessageType, void *))delivery_status_cb, NULL);
    button_res->set_value_updated_function(button_counter_updated);

    // Create resource for led blinking pattern. Path of this resource will be: 3201/0/5853.
    pattern_res = M2MInterfaceFactory::create_resource(object_list, 3201, 0, 5853, M2MResourceInstance::STRING, M2MBase::GET_PUT_ALLOWED);
    if (!pattern_res) {
        return false;
    }
    pattern_res->set_value((const unsigned char *)"500:500:500:500", 15);
    pattern_res->set_observable(true);
    pattern_res->set_value_updated_function(blink_pattern_updated);
    pattern_res->set_message_delivery_status_cb((void(*)(const M2MBase &, const M2MBase::MessageDeliveryStatus, const M2MBase::MessageType, void *))delivery_status_cb, NULL);

    // Create resource for starting the led blinking. Path of this resource will be: 3201/0/5850.
    blink_res = M2MInterfaceFactory::create_resource(object_list, 3201, 0, 5850, M2MResourceInstance::STRING, M2MBase::POST_ALLOWED);
    if (!blink_res) {
        return false;
    }

    blink_res->set_execute_function(blink_cb);
    blink_res->set_message_delivery_status_cb((void(*)(const M2MBase &, const M2MBase::MessageDeliveryStatus, const M2MBase::MessageType, void *))delivery_status_cb, NULL);
    blink_res->set_delayed_response(true);

    // Create optional Device resource for running factory reset for the device. Path of this resource will be: 3/0/5.
    factory_reset_res = M2MInterfaceFactory::create_device()->create_resource(M2MDevice::FactoryReset);
    if (factory_reset_res) {
        factory_reset_res->set_execute_function(factory_reset_cb);
    } else {
        return false;
    }

    // Create an example resource for handling large resource payloads. Path of this resource will be: 5000/0/2.
    large_res = M2MInterfaceFactory::create_resource(object_list, 5000, 0, 2, M2MResourceInstance::STRING, M2MBase::GET_ALLOWED);
    if (!large_res) {
        return false;
    }

    large_res->set_message_delivery_status_cb((void(*)(const M2MBase &, const M2MBase::MessageDeliveryStatus, const M2MBase::MessageType, void *))large_res_sent_cb, NULL);
    large_res->set_read_resource_function(large_res_read_requested, NULL);

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
    button_res->set_auto_observable(true);
    pattern_res->set_auto_observable(true);
    blink_res->set_auto_observable(true);
    factory_reset_res->set_auto_observable(true);
#endif

    return true;
}

static void reboot_if_threshold_value(int threshold)
{
    if (++error_count == threshold) {
        printf("Max error count %d reached, rebooting.\n\n", threshold);
        mcc_platform_do_wait(1 * 1000);
        mcc_platform_reboot();
    }
}

/** Resource callback functions --> **/

static void button_counter_updated(const char *)
{
    // Converts uint64_t to a string to remove the dependency for int64 printf implementation.
    char buffer[20 + 1];
    (void) m2m::itoa_c(button_res->get_value_int(), buffer);
    printf("Counter resource set to %s\r\n", buffer);
}

static void blink_pattern_updated(const char *)
{
    printf("PUT received, new value: %s\r\n", pattern_res->get_value_string().c_str());
}

static void blink_cb(void *)
{
    String pattern_string = pattern_res->get_value_string();
    printf("POST executed\r\n");

    // The pattern is something like 500:200:500, so parse that.
    // LED blinking is done while parsing.
#ifndef PDMC_EXAMPLE_MINIMAL
    const bool restart_pattern = false;
    if (blinky.start((char *)pattern_res->value(), pattern_res->value_length(), restart_pattern) == false) {
        printf("out of memory error\r\n");
    }
#endif
    blink_res->send_delayed_post_response();
}

static void delivery_status_cb(const M2MBase &object,
                               const M2MBase::MessageDeliveryStatus status,
                               const M2MBase::MessageType /*type*/)
{
    switch (status) {
        case M2MBase::MESSAGE_STATUS_BUILD_ERROR:
            printf("Message status callback: (%s) error when building CoAP message\r\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_RESEND_QUEUE_FULL:
            printf("Message status callback: (%s) CoAP resend queue full\r\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_SENT:
            printf("Message status callback: (%s) Message sent to server\r\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_DELIVERED:
            printf("Message status callback: (%s) Message delivered\r\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_SEND_FAILED:
            printf("Message status callback: (%s) Message sending failed\r\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_SUBSCRIBED:
            printf("Message status callback: (%s) subscribed\r\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_UNSUBSCRIBED:
            printf("Message status callback: (%s) subscription removed\r\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_REJECTED:
            printf("Message status callback: (%s) server has rejected the message\r\n", object.uri_path());
            break;
        default:
            break;
    }
}

static void large_res_sent_cb(const M2MBase &base,
                              const M2MBase::MessageDeliveryStatus status,
                              const M2MBase::MessageType /*type*/)
{
    switch (status) {
        case M2MBase::MESSAGE_STATUS_DELIVERED:
            free(large_res_data);
            printf("5000/0/2 data sent to server, memory can be now released\r\n");
            break;

        case M2MBase::MESSAGE_STATUS_SEND_FAILED:
            printf("Failed to send 5000/0/2 data!\r\n");
            free(large_res_data);
            break;

        default:
            break;
    }
}

static void factory_reset_cb(void *)
{
    printf("Factory reset resource triggered\r\n");

    // First send response, so server won't be left waiting.
    // Factory reset resource is by default expecting explicit
    // delayed response sending.
    factory_reset_res->send_delayed_post_response();

    // Then run potentially long-taking factory reset routines.
    kcm_factory_reset();
}

static coap_response_code_e large_res_read_requested(const M2MResourceBase &resource,
                                                     uint8_t *&buffer,
                                                     size_t &buffer_size,
                                                     size_t &total_size,
                                                     const size_t offset,
                                                     void */*client_args*/)
{
    printf("GET request received for resource: %s\r\n", resource.uri_path());

    // Allocate buffer when first request comes in
    if (offset == 0) {
        large_res_data = (uint8_t *)malloc(large_res_size);
        memset(large_res_data, 0, large_res_size);
    }

    if (!large_res_data) {
        return COAP_RESPONSE_INTERNAL_SERVER_ERROR;
    }

    total_size = large_res_size;

    // Adjust last package size
    if (offset + buffer_size > total_size) {
        buffer_size = total_size - offset;
    }

    // Read data from offset
    buffer = (uint8_t *)large_res_data + offset;

    return COAP_RESPONSE_CONTENT;
}
