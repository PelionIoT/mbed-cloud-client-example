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
#if (defined(MBED_CLOUD_CLIENT_FOTA_ENABLE) && (MBED_CLOUD_CLIENT_FOTA_ENABLE))
	

#include "fota/fota_app_ifs.h"
#include "fota/fota_platform_hooks.h"
#if defined(TARGET_LIKE_LINUX) && (MBED_CLOUD_CLIENT_FOTA_SUB_COMPONENT_SUPPORT == 1)
#include "fota/fota_sub_component.h"
#endif

#if defined(FOTA_CUSTOM_PLATFORM)

static fota_component_desc_info_t external_component_info;


/* Callback examples */

#if defined(TARGET_LIKE_LINUX) && (MBED_CLOUD_CLIENT_FOTA_SUB_COMPONENT_SUPPORT == 1)
/*Subcomponent callbacks*/
int sub_component_rollback_handler(const char *comp_name, const char *sub_comp_name, const uint8_t *vendor_data, size_t vendor_data_size, void *app_ctx)
{
    printf("sub_component_rollback_handler sub_comp_name: %s, vendor_data: %.*s", sub_comp_name, (int)vendor_data_size, vendor_data);
    return FOTA_STATUS_SUCCESS;
}

int sub_component_finalize_handler(const char *comp_name, const char *sub_comp_name, const uint8_t *vendor_data, size_t vendor_data_size, fota_status_e fota_status, void *app_ctx)
{
    printf("sub_component_finalize_handler sub_comp_name: %s, vendor_data: %.*s", sub_comp_name, (int)vendor_data_size, vendor_data);
    return FOTA_STATUS_SUCCESS;
}
#endif //#if defined(TARGET_LIKE_LINUX) && (MBED_CLOUD_CLIENT_FOTA_SUB_COMPONENT_SUPPORT == 1)

static void print_component_info(const char *comp_name, const char *sub_comp_name, const uint8_t *vendor_data, size_t vendor_data_size)
{
    printf("%s component installed\n", comp_name);
    if (sub_comp_name) {
        printf("sub_comp_name: %s\n", sub_comp_name);
    }
    if (vendor_data) {
        printf("vendor_data: %.*s\n", (int)vendor_data_size, vendor_data);
    }
}

static int pdmc_component_verifier(const char *comp_name, const char *sub_comp_name, const uint8_t *vendor_data, size_t vendor_data_size, void* app_ctx)
{ 
    printf("pdmc_component_verifier called for %s\n", comp_name);
    print_component_info(comp_name, sub_comp_name, vendor_data, vendor_data_size);
    return FOTA_STATUS_SUCCESS;
}

#if !defined(TARGET_LIKE_LINUX)
static int pdmc_component_installer(const char *comp_name, const char *sub_comp_name, fota_comp_candidate_iterate_callback_info *info, const uint8_t *vendor_data, size_t vendor_data_size, void *app_ctx)
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
            print_component_info(comp_name, sub_comp_name, vendor_data, vendor_data_size);
            return FOTA_STATUS_SUCCESS;
        default:
            return FOTA_STATUS_INTERNAL_ERROR;
    }
    return FOTA_STATUS_SUCCESS;
}
#else
static int pdmc_component_installer(const char *comp_name, const char *sub_comp_name, const char *file_name, const uint8_t *vendor_data, size_t vendor_data_size, void *app_ctx)
{
    print_component_info(comp_name, sub_comp_name, vendor_data, vendor_data_size);
    return FOTA_STATUS_SUCCESS;
}
#endif

/* Platform hooks implementation */
int fota_platform_init_hook(bool after_upgrade)
{
    int ret = 0;

    external_component_info.install_alignment = 1;
    external_component_info.support_delta = false;
    external_component_info.need_reboot = true;
    external_component_info.component_verify_install_cb = NULL;
    external_component_info.component_verify_cb = pdmc_component_verifier;
#if !defined(TARGET_LIKE_LINUX)
    external_component_info.candidate_iterate_cb = NULL;
#endif    
    external_component_info.component_install_cb = pdmc_component_installer;
    external_component_info.component_finalize_cb = NULL;

    external_component_info.curr_fw_read = 0; // only needed if support_delta = true
    external_component_info.curr_fw_get_digest = 0; // only needed if support_delta = true

    ret = fota_component_add(&external_component_info, "METER", "0.0.0");

#if defined(TARGET_LIKE_LINUX) && (MBED_CLOUD_CLIENT_FOTA_SUB_COMPONENT_SUPPORT == 1)	
    printf("Add sub components\n");

    fota_sub_comp_info_t dummy_sub_component_desc = { 0 };
    dummy_sub_component_desc.finalize_cb = sub_component_finalize_handler;
    dummy_sub_component_desc.finalize_order = 1;
    dummy_sub_component_desc.install_cb= pdmc_component_installer;
    dummy_sub_component_desc.install_order = 1;
    dummy_sub_component_desc.rollback_cb = sub_component_rollback_handler;
    dummy_sub_component_desc.rollback_order = 2;
    dummy_sub_component_desc.verify_cb= pdmc_component_verifier;
    dummy_sub_component_desc.verify_order = 1;
    ret = fota_sub_component_add("MAIN","img1_id", &dummy_sub_component_desc); //Component MAIN registered by default during fota initialization, no need to call `fota_component_add` for MAIN component.
    if (ret != 0){
        return ret;
    }

    fota_sub_comp_info_t dummy_sub_component_desc2 = { 0 };
    dummy_sub_component_desc2.finalize_cb = sub_component_finalize_handler;
    dummy_sub_component_desc2.finalize_order = 2;
    dummy_sub_component_desc2.install_cb = pdmc_component_installer;
    dummy_sub_component_desc2.install_order = 2;
    dummy_sub_component_desc2.rollback_cb = sub_component_rollback_handler;
    dummy_sub_component_desc2.rollback_order = 1;
    dummy_sub_component_desc2.verify_cb = pdmc_component_verifier;
    dummy_sub_component_desc2.verify_order = 2;
    ret = fota_sub_component_add("MAIN","img2_id", &dummy_sub_component_desc2);//Component MAIN registered by default during fota initialization, no need to call `fota_component_add` for MAIN component.
#endif

    return ret;
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

#endif //#if defined(FOTA_CUSTOM_PLATFORM)
#endif //#if (defined(MBED_CLOUD_CLIENT_FOTA_ENABLE) && (MBED_CLOUD_CLIENT_FOTA_ENABLE))
