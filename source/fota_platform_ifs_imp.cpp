// ----------------------------------------------------------------------------
// Copyright 2021 Pelion Ltd.
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
#include <stdint.h>
#include <inttypes.h>

#ifdef MBED_CLOUD_CLIENT_FOTA_ENABLE

#define TRACE_GROUP "FOTA"

#include "fota/fota_app_ifs.h"    // required for implementing custom install callback for Linux like targets
#include <stdio.h>
#include <assert.h>

#if !(defined (FOTA_DEFAULT_APP_IFS) && FOTA_DEFAULT_APP_IFS==1)
int fota_app_on_complete(int32_t status)
{
    if (status == FOTA_STATUS_SUCCESS) {
        printf("FOTA was successfully completed\n");
    } else {
        printf("FOTA failed with status %" PRId32 "\n", status);
    }

    return FOTA_STATUS_SUCCESS;
}

void fota_app_on_download_progress(size_t downloaded_size, size_t current_chunk_size, size_t total_size)
{
    static const uint32_t  print_range_percent = 5;
    uint32_t progress = (downloaded_size + current_chunk_size) * 100 / total_size;
    uint32_t prev_progress = downloaded_size * 100 / total_size;

    if (downloaded_size == 0 || ((progress / print_range_percent) > (prev_progress / print_range_percent))) {
        printf("Downloading firmware. %" PRIu32 "%%\n", progress);
    }
}

int fota_app_on_install_authorization()
{
    printf("Firmware install authorized\n");
    fota_app_authorize();
    return FOTA_STATUS_SUCCESS;
}

int  fota_app_on_download_authorization(
    const manifest_firmware_info_t *candidate_info,
    fota_component_version_t curr_fw_version)
{
    char curr_semver[FOTA_COMPONENT_MAX_SEMVER_STR_SIZE] = { 0 };
    char new_semver[FOTA_COMPONENT_MAX_SEMVER_STR_SIZE] = { 0 };

    fota_component_version_int_to_semver(curr_fw_version, curr_semver);
    fota_component_version_int_to_semver(candidate_info->version, new_semver);

    printf("Firmware download requested (priority=%" PRIu32 ")\n", candidate_info->priority);
    printf(
        "Updating component %s from version %s to %s\n",
        candidate_info->component_name,
        curr_semver,
        new_semver
    );

    printf("Update priority %" PRIu32 "\n", candidate_info->priority);

    if (candidate_info->payload_format == FOTA_MANIFEST_PAYLOAD_FORMAT_DELTA) {
        printf(
            "Delta update. Patch size %zuB full image size %zuB\n",
            candidate_info->payload_size,
            candidate_info->installed_size
        );
    } else {
        printf("Update size %zuB\n", candidate_info->payload_size);
    }

    fota_app_authorize();

    return FOTA_STATUS_SUCCESS;
}
#endif //#if !(defined (FOTA_DEFAULT_APP_IFS) && FOTA_DEFAULT_APP_IFS==1)


#if defined(TARGET_LIKE_LINUX) && !defined(USE_ACTIVATION_SCRIPT)  // e.g. Yocto target have different update activation logic residing outside of the example
// Simplified Linux use case example.
// For MAIN component update the the binary file current process is running.
// Simulate component update by just printing its name.
// After the installation callback returns, FOTA will "reboot" by calling pal_osReboot().

int fota_app_on_install_candidate(const char *candidate_fs_name, const manifest_firmware_info_t *firmware_info)
{
    int ret = FOTA_STATUS_SUCCESS;
    if (0 == strncmp(FOTA_COMPONENT_MAIN_COMPONENT_NAME, firmware_info->component_name, FOTA_COMPONENT_MAX_NAME_SIZE)) {
        // installing MAIN component
        ret = fota_app_install_main_app(candidate_fs_name);
        if (FOTA_STATUS_SUCCESS == ret) {
            FOTA_APP_PRINT("Successfully installed MAIN component\n");
            // FOTA does support a case where installer method reboots the system.
        }
    } else {
        FOTA_APP_PRINT("fota_app_on_install_candidate deprecated for component %s, use component_install_cb\n", firmware_info->component_name);
    }
    return ret;
}
#endif // defined(TARGET_LIKE_LINUX) && !defined(USE_ACTIVATION_SCRIPT)

#endif  // MBED_CLOUD_CLIENT_FOTA_ENABLE

