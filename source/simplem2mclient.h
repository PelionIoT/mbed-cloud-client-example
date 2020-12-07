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


#ifndef SIMPLEM2MCLIENT_H
#define SIMPLEM2MCLIENT_H
#include <stdio.h>
#include <inttypes.h>

#include "mbed-cloud-client/MbedCloudClient.h"
#include "m2mdevice.h"
#include "mcc_common_setup.h"
#include "m2mresource.h"
#include "mbed-client/m2minterface.h"
#include "key_config_manager.h"
#include "resource.h"
#include "application_init.h"
#include "factory_configurator_client.h"

#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
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

#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
static void external_update(uint32_t start_address, uint32_t size)
{
    printf("Application received external update, start address: %" PRIX32 ", size: %" PRIu32 " bytes\n", start_address, size);
}
#endif // MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE

class SimpleM2MClient {

public:

    SimpleM2MClient() :
        _registered(false),
        _register_called(false),
        _error_count(0)
#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
        , _paused(false)
#endif
    {
    }

    bool call_register()
    {

        _cloud_client.on_registered(this, &SimpleM2MClient::client_registered);
        _cloud_client.on_registration_updated(this, &SimpleM2MClient::client_registration_updated);
        _cloud_client.on_unregistered(this, &SimpleM2MClient::client_unregistered);
        _cloud_client.on_error(this, &SimpleM2MClient::error);

#ifdef MBED_CLOUD_CLIENT_SUPPORT_MULTICAST_UPDATE
        _cloud_client.on_external_update(external_update);
#endif

        bool setup = _cloud_client.setup(mcc_platform_get_network_interface());
        _register_called = true;
        if (!setup) {
            printf("Client setup failed\n");
            return false;
        }

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
        callback_handler cb(this, &SimpleM2MClient::sleep_callback_function);
        _cloud_client.set_queue_sleep_handler(cb);
#endif

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
        /* Set callback functions for authorizing updates and monitoring progress.
           Code is implemented in update_ui_example.cpp
           Both callbacks are completely optional. If no authorization callback
           is set, the update process will procede immediately in each step.
        */
        update_ui_set_cloud_client(&_cloud_client);
        _cloud_client.set_update_authorize_priority_handler(update_authorize_priority_handler);
        _cloud_client.set_update_progress_handler(update_progress);
#endif
        return true;
    }

    void init()
    {
        _cloud_client.init();
    }
    void close()
    {
        _cloud_client.close();
    }

    void register_update()
    {
        _cloud_client.register_update();
    }

    void client_registered()
    {
        _registered = true;
        printf("Client registered\r\n");
        _error_count = 0;
        static const ConnectorClientEndpointInfo *endpoint = NULL;
        if (endpoint == NULL) {
            endpoint = _cloud_client.endpoint_info();
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
    }

    void client_registration_updated()
    {
        printf("Client registration updated\n");
        _error_count = 0;
    }

    void client_unregistered()
    {
#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
        _paused = false;
#endif
        _registered = false;
        _register_called = false;
        printf("Client unregistered - Exiting application\n");
#ifdef MEMORY_TESTS_HEAP
        print_heap_stats();
#endif
    }

    void error(int error_code)
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
        printf("Error details : %s\r\n", _cloud_client.error_description());

// Feature is disabled if MAX_ERROR_COUNT is 0.
#if (MAX_ERROR_COUNT > 0)
        if (error_code == MbedCloudClient::ConnectNetworkError ||
                error_code == MbedCloudClient::ConnectDnsResolvingFailed ||
                error_code == MbedCloudClient::ConnectSecureConnectionFailed) {
            if (++_error_count == MAX_ERROR_COUNT) {
                printf("Max error count %d reached, rebooting.\n\n", MAX_ERROR_COUNT);
                mcc_platform_do_wait(1 * 1000);
                mcc_platform_reboot();
            }
        }
#endif
    }

    bool is_client_registered()
    {
        return _registered;
    }

    bool is_register_called()
    {
        return _register_called;
    }

    void register_and_connect()
    {
#if defined (MEMORY_TESTS_HEAP) && !defined(MCC_MINIMAL)
        // Add some test resources to measure memory consumption.
        // This code is activated only if MEMORY_TESTS_HEAP is defined.
        create_m2mobject_test_set(_obj_list);
#endif
        _cloud_client.add_objects(_obj_list);

        // Start registering to the cloud.
        call_register();

        // Print memory statistics if the MEMORY_TESTS_HEAP is defined.
#ifdef MEMORY_TESTS_HEAP
        printf("Register being called\r\n");
        print_heap_stats();
#endif
    }

    MbedCloudClient &get_cloud_client()
    {
        return _cloud_client;
    }

    M2MObjectList *get_m2m_obj_list()
    {
        return &_obj_list;
    }

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
    void sleep_callback_function()
    {
        printf("Pelion client is going to sleep - Pausing the client\r\n");
        _paused = true;
        _cloud_client.pause();
        mcc_platform_interface_close();
    }

    bool is_client_paused()
    {
        return _paused;
    }

    void client_resumed()
    {
        int timeout_ms = 1000;
        while (-1 == mcc_platform_interface_connect()){
            // Will try to connect using mcc_platform_interface_connect forever. 
            // wait timeout is always doubled
            printf("Network did not recover after pause. Try again after %d milliseconds.\n",timeout_ms);
            mcc_platform_do_wait(timeout_ms);
            timeout_ms *= 2;
        }
        _paused = false;
        _cloud_client.resume(mcc_platform_get_network_interface());
        
    }

#endif

    M2MResource *add_cloud_resource(uint16_t object_id, uint16_t instance_id,
                                    uint16_t resource_id, const char *resource_type,
                                    M2MResourceInstance::ResourceType data_type,
                                    M2MBase::Operation allowed, const char *value,
                                    bool observable, void *cb, void *message_status_cb)
    {
        return add_resource(&_obj_list, object_id, instance_id, resource_id, resource_type, data_type,
                            allowed, value, observable, cb, message_status_cb);

    }

private:
    M2MObjectList       _obj_list;
    MbedCloudClient     _cloud_client;
    bool                _registered;
    bool                _register_called;
    int                 _error_count;
#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
    volatile bool       _paused;
#endif

};

#endif // SIMPLEM2MCLIENT_H
