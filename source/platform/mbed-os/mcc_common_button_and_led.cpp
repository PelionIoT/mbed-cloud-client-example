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

#include "mbed.h"
#include "mcc_common_config.h"
#include "mcc_common_button_and_led.h"
#include <stdint.h>

#if PLATFORM_ENABLE_BUTTON
// Make button and led definitions optional
#ifndef MBED_CONF_APP_BUTTON_PINNAME
#define MBED_CONF_APP_BUTTON_PINNAME NC
#endif
#endif

#if PLATFORM_ENABLE_LED
// Define led on/off
#ifdef TARGET_STM
#define LED_ON (true)
#else // #ifdef TARGET_STM
#define LED_ON (false)
#endif // #ifdef TARGET_STM

#define LED_OFF (!LED_ON)

#ifndef MBED_CONF_APP_LED_PINNAME
#define MBED_CONF_APP_LED_PINNAME NC
#endif

// Button and led
static DigitalOut led(MBED_CONF_APP_LED_PINNAME, LED_OFF);
#endif

#if PLATFORM_ENABLE_BUTTON
static InterruptIn button(MBED_CONF_APP_BUTTON_PINNAME);
static bool button_pressed = false;
static void button_press(void);

static void button_press(void)
{
    button_pressed = true;
}
#endif

uint8_t mcc_platform_button_clicked(void)
{
#if PLATFORM_ENABLE_BUTTON
    if (button_pressed) {
        button_pressed = false;
        return true;
    }
#endif
    return false;
}

uint8_t mcc_platform_init_button_and_led(void)
{
#if PLATFORM_ENABLE_BUTTON
   if(MBED_CONF_APP_BUTTON_PINNAME != NC) {
        button.fall(&button_press);
    }
#endif
    return 0;
}

void mcc_platform_toggle_led(void)
{
#if PLATFORM_ENABLE_LED
    if (MBED_CONF_APP_LED_PINNAME != NC) {
        led = !led;
    }
    else {
        printf("Virtual LED toggled\n");
    }
#endif
}

void mcc_platform_led_off(void)
{
#if PLATFORM_ENABLE_LED
    if (MBED_CONF_APP_LED_PINNAME != NC) {
        led = LED_OFF;
    }
    else {
        printf("Virtual LED off\n");
    }
#endif
}
