/*
 * Copyright (c) 2015-2018 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

///////////
// INCLUDES
///////////
#include <stdio.h>
#include "common_setup.h"
#include "common_config.h"
#include "factory_configurator_client.h"

int mcc_platform_reset_storage(void)
{
    printf("Resets storage to an empty state.\n");
    int status = fcc_storage_delete();
    if (status != FCC_STATUS_SUCCESS) {
        printf("Failed to delete storage - %d\n", status);
    }
    return status;
}

int mcc_platform_fcc_init() {
    int status = fcc_init();
    if(status != FCC_STATUS_SUCCESS) {
        printf("fcc_init failed with status %d! - exit\n", status);
        return status;
    }

#if RESET_STORAGE
    status=mcc_platform_reset_storage();
#endif

    return status;
}
