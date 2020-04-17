// ----------------------------------------------------------------------------
// Copyright 2018-2020 ARM Ltd.
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
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_RTT.h"

#if PORT_DEBUG
#define PORT_LOG_DEBUG configPRINTF
#else
#define PORT_LOG_DEBUG(...)
#endif

extern uint32_t ulRand();

BaseType_t xApplicationGetRandomNumber( uint32_t *pulNumber )
{
    if (!pulNumber)
        return pdFALSE;

    *pulNumber = ulRand();
    return pdTRUE;
}

void __attribute__((weak)) vApplicationStackOverflowHook( TaskHandle_t xTask, signed portCHAR *pcTaskName )
{
    SEGGER_RTT_printf(0, "Stack overflow in task %s\n", pcTaskName);
    volatile int debug = 1;
    // LATER: Control all LEDs to blink 5s
    while (debug) ;
}

void* realloc(void* ptr, size_t new_size)
{
    void* new_ptr = malloc(new_size);
    if(new_ptr)
    {
        if(ptr)
            memcpy(new_ptr, ptr, new_size);
        free(ptr);
    }
    return new_ptr;
}

void* calloc(size_t count, size_t size)
{
    void* ptr = NULL;
    ptr = malloc(count*size);
    if(ptr)
        memset(ptr, 0, count*size);
    return ptr;
}

void* malloc(size_t size)
{
    void* ptr = NULL;
    PORT_LOG_DEBUG("%s(%d) called.\n", __FUNCTION__, size);
    if(size > 0) {
        ptr = pvPortMalloc(size);
    }
    return ptr;
}

void free(void* ptr)
{
    PORT_LOG_DEBUG("%s(0x%08X) called.\n", __FUNCTION__, ptr);
    if (ptr) {
        vPortFree(ptr);
    }
}

uint32_t sys_jiffies(void)
{
    return xTaskGetTickCount();
}

