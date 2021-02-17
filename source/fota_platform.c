// ----------------------------------------------------------------------------
// Copyright 2020 ARM Ltd.
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

#include "fota/fota_base.h"

#ifdef MBED_CLOUD_CLIENT_FOTA_ENABLE

#define TRACE_GROUP "FOTA"

#include "fota/fota_app_ifs.h"    // required for implementing custom install callback for Linux like targets
#include <stdio.h>
#include <assert.h>


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
        FOTA_APP_PRINT("%s component installed (example)\n", firmware_info->component_name);
    }
    return ret;
}
#endif // defined(TARGET_LIKE_LINUX) && !defined(USE_ACTIVATION_SCRIPT)

#endif  // MBED_CLOUD_CLIENT_FOTA_ENABLE
