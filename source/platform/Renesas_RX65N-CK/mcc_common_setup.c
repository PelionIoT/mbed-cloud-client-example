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

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "pal_plat_rtos.h"
#include "mcc_common_setup.h"


int mcc_platform_run_program(main_t mainFunc)
{
    BaseType_t status = xTaskCreate((TaskFunction_t)mainFunc, "_main_", (uint32_t)1024*8, NULL, tskIDLE_PRIORITY + 1, NULL);

    if (status == pdPASS)
    {
        vTaskStartScheduler();
    }
    return 1;
}

void mcc_platform_do_wait(int timeout_ms)
{
    vTaskDelay(pdMS_TO_TICKS(timeout_ms));
}

void mcc_platform_sw_build_info(void)
{
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\r\n");
}

int mcc_platform_reformat_storage(void)
{
    // printf("mcc_platform_reformat_storage does not support FreeRTOS !!!\n");

    // XXX: this returns zero, which means success. This is needed to keep
    // some of the tests running, which ask for formatted storage, even though
    // they do not need it.
    return 0;
}

int mcc_platform_storage_init(void)
{
    // printf("mcc_platform_storage_init()\n");
    
    // XXX: some of the existing users of this module actually call mcc_platform_storage_init()
    // BEFORE mcc_platform_init().
    //
    // Before all the callers are fixed, one needs to emulate the behavior of other platforms,
    // which do not care about the order of these.
    return 0;
}

void mcc_platform_reboot(void)
{
    pal_plat_osReboot();
}
