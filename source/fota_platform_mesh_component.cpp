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

#include "mbed-cloud-client/MbedCloudClient.h"

#if defined(MBED_CLOUD_CLIENT_FOTA_MULTICAST_SUPPORT) && (MBED_CLOUD_CLIENT_FOTA_MULTICAST_SUPPORT != 0) && defined(FOTA_CUSTOM_PLATFORM) && (defined(TARGET_LIKE_MBED))
#include "fota_app_ifs.h"
#include "fota_platform_hooks.h"

static fota_component_desc_info_t external_component_info;

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

int fota_app_on_download_authorization(
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

static int install_iterate_handler(fota_candidate_iterate_callback_info *info)
{
    switch (info->status) {
        case FOTA_CANDIDATE_ITERATE_START:
            printf("fota candidate iterate start \n");
            return FOTA_STATUS_SUCCESS;
        case FOTA_CANDIDATE_ITERATE_FRAGMENT:
            printf(".");
            return FOTA_STATUS_SUCCESS;
        case FOTA_CANDIDATE_ITERATE_FINISH:
            printf("\nfota candidate iterate finish \n");
            printf("\nApplication received external update\n"); // Use same phrase than in UCHub case. Test case is polling this line.
            return FOTA_STATUS_SUCCESS;
        default:
            return FOTA_STATUS_INTERNAL_ERROR;
    }
    return 0;
}

static int pdmc_component_verifier(const char *comp_name, const fota_header_info_t *expected_header_info)
{
    printf("pdmc_component_verifier called for %s\n", comp_name);
    return FOTA_STATUS_SUCCESS;
}

int fota_platform_init_hook(bool after_upgrade)
{
    external_component_info.install_alignment = 1;
    external_component_info.support_delta = false;
    external_component_info.need_reboot = true;
    external_component_info.component_verify_install_cb = pdmc_component_verifier;
    external_component_info.curr_fw_read = 0; // only needed if support_delta = true
    external_component_info.curr_fw_get_digest = 0; // only needed if support_delta = true
    external_component_info.candidate_iterate_cb = install_iterate_handler;

    return fota_component_add(&external_component_info, "METER", "0.0.0");
}

int fota_platform_start_update_hook(const char *comp_name)
{
    return FOTA_STATUS_SUCCESS;
}

int fota_platform_finish_update_hook(const char *comp_name)
{
    return FOTA_STATUS_SUCCESS;
}

int fota_platform_abort_update_hook(const char *comp_name)
{
    return FOTA_STATUS_SUCCESS;
}
#endif
