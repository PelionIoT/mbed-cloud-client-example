// ----------------------------------------------------------------------------
// Copyright 2015-2019 ARM Ltd.
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

///////////
// INCLUDES
///////////

#include "mcc_common_config.h"
#include "mcc_common_button_and_led.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#if PLATFORM_ENABLE_BUTTON
static volatile int button_pressed = 0;
#endif

// Internal function prototypes
typedef void (*signalhandler_t)(int);
static void handle_signal(void);

pthread_t resource_thread;

static void handle_signal(void)
{
#if PLATFORM_ENABLE_BUTTON
    pthread_detach(resource_thread);
#endif
    exit(0);
}

uint8_t mcc_platform_button_clicked(void)
{
#if PLATFORM_ENABLE_BUTTON
    if (button_pressed) {
        button_pressed = 0;
        return 1;
    }
#endif
    return 0;
}

#if PLATFORM_ENABLE_BUTTON
void *button_thread(void *arg)
{
    // EOF checking is required, otherwise this ends up as a busy loop
    // and consumes a lot of CPU. EOF will be return if there is no tty
    // or Ctrl+D is pressed.
    while (getchar() != EOF) {
        button_pressed = 1;
    }
    button_pressed = 0;
    return NULL;
}
#endif

uint8_t mcc_platform_init_button_and_led(void)
{
#if PLATFORM_ENABLE_BUTTON
    pthread_create(&resource_thread, NULL, &button_thread, NULL);
#endif
    signal(SIGTERM, (signalhandler_t)handle_signal);
    return 0;
}

void mcc_platform_toggle_led(void)
{
#if PLATFORM_ENABLE_LED
    printf("Virtual LED toggled\n");
#endif
}

void mcc_platform_led_off(void) {}
