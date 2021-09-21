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

#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#endif

#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "factory_configurator_client.h"
#include "mcc_common_setup.h"
#include "mcc_common_button_and_led.h"
#include "application_init.h"

#if defined (MEMORY_TESTS_HEAP)
#include "memory_tests.h"
#endif

#if defined (MBED_CONF_APP_ENABLE_DS_CUSTOM_METRICS_EXAMPLE)
#include "ds_custom_metrics_app.h"
#endif

#ifdef MBED_CONF_MBED_CLOUD_CLIENT_SECURE_ELEMENT_SUPPORT
#include "mcc_se_init.h"
#endif

#if MBED_CONF_APP_DEVELOPER_MODE == 1
#ifdef PAL_USER_DEFINED_CONFIGURATION
#include PAL_USER_DEFINED_CONFIGURATION
#endif
#endif // #if MBED_CONF_APP_DEVELOPER_MODE == 1

// Include this only for Developer mode and device which doesn't have in-built TRNG support
#if MBED_CONF_APP_DEVELOPER_MODE == 1
#ifdef PAL_USER_DEFINED_CONFIGURATION
#define FCC_ROT_SIZE                       16
const uint8_t MBED_CLOUD_DEV_ROT[FCC_ROT_SIZE] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
#if !PAL_USE_HW_TRNG
#define FCC_ENTROPY_SIZE                   48
const uint8_t MBED_CLOUD_DEV_ENTROPY[FCC_ENTROPY_SIZE] = { 0xf6, 0xd6, 0xc0, 0x09, 0x9e, 0x6e, 0xf2, 0x37, 0xdc, 0x29, 0x88, 0xf1, 0x57, 0x32, 0x7d, 0xde, 0xac, 0xb3, 0x99, 0x8c, 0xb9, 0x11, 0x35, 0x18, 0xeb, 0x48, 0x29, 0x03, 0x6a, 0x94, 0x6d, 0xe8, 0x40, 0xc0, 0x28, 0xcc, 0xe4, 0x04, 0xc3, 0x1f, 0x4b, 0xc2, 0xe0, 0x68, 0xa0, 0x93, 0xe6, 0x3a };
#endif // PAL_USE_HW_TRNG = 0
#endif // PAL_USER_DEFINED_CONFIGURATION
#endif // #if MBED_CONF_APP_DEVELOPER_MODE == 1

static int reset_storage(void);
static int fcc_initialize(void);
static int sotp_initialize(void);

static void print_fcc_status(int fcc_status)
{
#ifndef PDMC_EXAMPLE_MINIMAL
#ifndef DISABLE_ERROR_DESCRIPTION
    const char *error;
    switch (fcc_status) {
        case FCC_STATUS_SUCCESS:
            return;
        case FCC_STATUS_ERROR :
            error = "Operation ended with an unspecified error.";
            break;
        case FCC_STATUS_MEMORY_OUT:
            error = "An out-of-memory condition occurred.";
            break;
        case FCC_STATUS_INVALID_PARAMETER:
            error = "A parameter provided to the function was invalid.";
            break;
        case FCC_STATUS_STORE_ERROR:
            error = "Storage internal error.";
            break;
        case FCC_STATUS_INTERNAL_ITEM_ALREADY_EXIST:
            error = "Current item already exists in storage.";
            break;
        case FCC_STATUS_CA_ERROR:
            error = "CA Certificate already exist in storage (currently only bootstrap CA)";
            break;
        case FCC_STATUS_ROT_ERROR:
            error = "ROT already exist in storage";
            break;
        case FCC_STATUS_ENTROPY_ERROR:
            error = "Entropy already exist in storage";
            break;
        case FCC_STATUS_FACTORY_DISABLED_ERROR:
            error = "FCC flow was disabled - denial of service error.";
            break;
        case FCC_STATUS_INVALID_CERTIFICATE:
            error = "Invalid certificate found.";
            break;
        case FCC_STATUS_INVALID_CERT_ATTRIBUTE:
            error = "Operation failed to get an attribute.";
            break;
        case FCC_STATUS_INVALID_CA_CERT_SIGNATURE:
            error = "Invalid ca signature.";
            break;
        case FCC_STATUS_EXPIRED_CERTIFICATE:
            error = "LWM2M or Update certificate is expired.";
            break;
        case FCC_STATUS_INVALID_LWM2M_CN_ATTR:
            error = "Invalid CN field of certificate.";
            break;
        case FCC_STATUS_KCM_ERROR:
            error = "KCM basic functionality failed.";
            break;
        case FCC_STATUS_KCM_STORAGE_ERROR:
            error = "KCM failed to read, write or get size of item from/to storage.";
            break;
        case FCC_STATUS_KCM_FILE_EXIST_ERROR:
            error = "KCM tried to create existing storage item.";
            break;
        case FCC_STATUS_KCM_CRYPTO_ERROR:
            error = "KCM returned error upon cryptographic check of an certificate or key.";
            break;
        case FCC_STATUS_NOT_INITIALIZED:
            error = "FCC failed or did not initialized.";
            break;
        case FCC_STATUS_BUNDLE_ERROR:
            error = "Protocol layer general error.";
            break;
        case FCC_STATUS_BUNDLE_RESPONSE_ERROR:
            error = "Protocol layer failed to create response buffer.";
            break;
        case FCC_STATUS_BUNDLE_UNSUPPORTED_GROUP:
            error = "Protocol layer detected unsupported group was found in a message.";
            break;
        case FCC_STATUS_BUNDLE_INVALID_GROUP:
            error = "Protocol layer detected invalid group in a message.";
            break;
        case FCC_STATUS_BUNDLE_INVALID_SCHEME:
            error = "The scheme version of a message in the protocol layer is wrong.";
            break;
        case FCC_STATUS_ITEM_NOT_EXIST:
            error = "Current item wasn't found in the storage";
            break;
        case FCC_STATUS_EMPTY_ITEM:
            error = "Current item's size is 0";
            break;
        case FCC_STATUS_WRONG_ITEM_DATA_SIZE:
            error = "Current item's size is different then expected";
            break;
        case FCC_STATUS_URI_WRONG_FORMAT:
            error = "Current URI is different than expected.";
            break;
        case FCC_STATUS_FIRST_TO_CLAIM_NOT_ALLOWED:
            error = "Can't use first to claim without bootstrap or with account ID";
            break;
        case FCC_STATUS_BOOTSTRAP_MODE_ERROR:
            error = "Wrong value of bootstrapUse mode.";
            break;
        case FCC_STATUS_OUTPUT_INFO_ERROR:
            error = "The process failed in output info creation.";
            break;
        case FCC_STATUS_WARNING_CREATE_ERROR:
            error = "The process failed in output info creation.";
            break;
        case FCC_STATUS_UTC_OFFSET_WRONG_FORMAT:
            error = "Current UTC is wrong.";
            break;
        case FCC_STATUS_CERTIFICATE_PUBLIC_KEY_CORRELATION_ERROR:
            error = "Certificate's public key failed do not matches to corresponding private key";
            break;
        case FCC_STATUS_BUNDLE_INVALID_KEEP_ALIVE_SESSION_STATUS:
            error = "The message status is invalid.";
            break;
        default:
            error = "UNKNOWN";
    }
    printf("\nFactory Configurator Client [ERROR]: %s\r\n", error);
#endif
#endif
}

#if defined(__SXOS__) || defined(__RTX)

extern "C"
void trace_printer(const char *str)
{
    printf("%s\r\n", str);
}

#endif

bool application_init_mbed_trace(void)
{
#ifndef PDMC_EXAMPLE_MINIMAL
    // Create mutex for tracing to avoid broken lines in logs
    if (!mbed_trace_helper_create_mutex()) {
        printf("ERROR - Mutex creation for mbed_trace failed!\n");
        return false;
    }
#endif

    // Initialize mbed trace
    if (mbed_trace_init() != 0) {
        printf("ERROR - mbed_trace_init failed!\n");
        return false;
    }

#ifndef PDMC_EXAMPLE_MINIMAL
    mbed_trace_mutex_wait_function_set(mbed_trace_helper_mutex_wait);
    mbed_trace_mutex_release_function_set(mbed_trace_helper_mutex_release);
#endif

#if defined(__SXOS__) || defined(__RTX)
    mbed_trace_print_function_set(trace_printer);
#endif


    return true;
}

static bool verify_cloud_configuration()
{
    int status;
    bool result = 0;

#if MBED_CONF_APP_DEVELOPER_MODE == 1
    printf("Starting developer flow\r\n");
    status = fcc_developer_flow();
    if (status == FCC_STATUS_KCM_FILE_EXIST_ERROR) {
        printf("Developer credentials already exist, continuing..\r\n");
        result = 0;
    } else if (status != FCC_STATUS_SUCCESS) {
        printf("Failed to load developer credentials\r\n");
        result = 1;
    }
#endif
#ifndef PDMC_EXAMPLE_MINIMAL
#if MBED_CONF_APP_DEVELOPER_MODE == 1
    status = fcc_verify_device_configured_4mbed_cloud();
    print_fcc_status(status);
    if (status != FCC_STATUS_SUCCESS && status != FCC_STATUS_EXPIRED_CERTIFICATE) {
        result = 1;
    }
#endif
#endif
    return result;
}

static bool initialize_fcc(void)
{
    int status;
    status = fcc_initialize();
    if (status != FCC_STATUS_SUCCESS) {
        printf("initialize_fcc fcc_init failed with status %d! - exit\r\n", status);
        return 1;
    }
#if RESET_STORAGE
    status = reset_storage();
    if (status != FCC_STATUS_SUCCESS) {
        printf("initialize_fcc reset_storage failed with status %d! - exit\r\n", status);
        return 1;
    }
    // Reinitialize SOTP
    status = sotp_initialize();
    if (status != FCC_STATUS_SUCCESS) {
        printf("initialize_fcc sotp_init failed with status %d! - exit\r\n", status);
        return 1;
    }
#endif

#ifdef MBED_CONF_MBED_CLOUD_CLIENT_SECURE_ELEMENT_SUPPORT
    //Initialize secure element
    status = mcc_se_init();
    if (status != 0) {
        printf("Failed to initialize secure element\n");
        return 1;
    }
#endif
    status = verify_cloud_configuration();
    if (status != 0) {
#ifndef PDMC_EXAMPLE_MINIMAL
        // This is designed to simplify user-experience by auto-formatting the
        // primary storage if no valid certificates exist.
        // This should never be used for any kind of production devices.
#ifndef MBED_CONF_APP_MCC_NO_AUTO_FORMAT
        status = reset_storage();
        if (status != FCC_STATUS_SUCCESS) {
            return 1;
        }
        status = sotp_initialize();
        if (status != FCC_STATUS_SUCCESS) {
            return 1;
        }
        status = verify_cloud_configuration();
        if (status != 0) {
            return 1;
        }
#else
        return 1;
#endif
#endif
    }
    return 0;
}

bool application_init(void)
{
    // The function always returns 0.
    (void) mcc_platform_init_button_and_led();

    // Print some statistics of current heap memory consumption, useful for finding
    // out where the memory goes.
#ifdef MEMORY_TESTS_HEAP
    print_heap_stats();
#endif

#if defined (MBED_CONF_APP_ENABLE_DS_CUSTOM_METRICS_EXAMPLE)
    mcce_ds_custom_metric_callback_set();
#endif

    printf("Start Device Management Client\r\n");

    if (initialize_fcc() != 0) {
        printf("Failed initializing FCC\r\n");
        return false;
    }

    return true;
}

static int reset_storage(void)
{
#if MBED_CONF_APP_DEVELOPER_MODE == 1
    printf("Resets storage to an empty state.\r\n");
    int status = fcc_storage_delete();
    if (status != FCC_STATUS_SUCCESS) {
        printf("Failed to delete storage - %d\r\n", status);
    }
    return status;
#else
    return FCC_STATUS_SUCCESS;
#endif
}

static int fcc_initialize(void)
{
#if MBED_CONF_APP_DEVELOPER_MODE == 1
    int status = fcc_init();
    // Ignore pre-existing RoT/Entropy in SOTP
    if (status != FCC_STATUS_SUCCESS && status != FCC_STATUS_ENTROPY_ERROR && status != FCC_STATUS_ROT_ERROR) {
        printf("fcc_initialize failed with status %d! - exit\r\n", status);
        return status;
    }
    status = sotp_initialize();
    if (status != FCC_STATUS_SUCCESS) {
        printf("fcc_initialize failed sotp_initialize() with status %d! - exit\r\n", status);
#if MBED_CONF_APP_DEVELOPER_MODE == 1
        (void)fcc_finalize();
#endif
    } else {
        // We can return SUCCESS here as preexisting RoT/Entropy is expected flow.
        status = FCC_STATUS_SUCCESS;
    }
    return status;
#else
    return FCC_STATUS_SUCCESS;
#endif
}

static int sotp_initialize(void)
{
    int status = FCC_STATUS_SUCCESS;
// Include this only for Developer mode and a device which doesn't have in-built TRNG support.
#if MBED_CONF_APP_DEVELOPER_MODE == 1
#if defined (PAL_USER_DEFINED_CONFIGURATION) && !PAL_USE_HW_TRNG
    status = fcc_entropy_set(MBED_CLOUD_DEV_ENTROPY, FCC_ENTROPY_SIZE);

    if (status != FCC_STATUS_SUCCESS && status != FCC_STATUS_ENTROPY_ERROR) {
        printf("fcc_entropy_set failed with status %d! - exit\r\n", status);
#if MBED_CONF_APP_DEVELOPER_MODE == 1
        (void)fcc_finalize();
#endif
        return status;
    }
#endif // PAL_USE_HW_TRNG = 0
    /* Include this only for Developer mode. The application will use fixed RoT to simplify user-experience with the application.
     * With this change the application be reflashed/SOTP can be erased safely without invalidating the application credentials.
     */
    status = fcc_rot_set(MBED_CLOUD_DEV_ROT, FCC_ROT_SIZE);

    if (status != FCC_STATUS_SUCCESS && status != FCC_STATUS_ROT_ERROR) {
        printf("fcc_rot_set failed with status %d! - exit\r\n", status);
#if MBED_CONF_APP_DEVELOPER_MODE == 1
        (void)fcc_finalize();
#endif
    } else {
        // We can return SUCCESS here as preexisting RoT/Entropy is expected flow.
        printf("Using hardcoded Root of Trust, not suitable for production use.\r\n");
        status = FCC_STATUS_SUCCESS;
    }
#endif // #if MBED_CONF_APP_DEVELOPER_MODE == 1
    return status;
}
