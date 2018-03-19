// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
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

#ifndef SETUP_H
#define SETUP_H

#include <stdint.h>

//FORWARD DECLARATION
class M2MObject;
namespace m2m {
    template<class ObjectTemplate> class Vector;
}

using namespace m2m;

typedef Vector<M2MObject *> M2MObjectList;

#ifdef __cplusplus
extern "C" {
#endif

// Interval to update resource value in ms
#define INCREMENT_INTERVAL 25000

typedef void (*main_t)(void);

// Initialize platform
// This function initializes screen and any other non-network
// related platform specific initializations required.
//
// @returns
//   0 for success, anything else for error
extern int initPlatform();

// Initialize network connection
extern bool init_connection();

// Returns network interface.
extern void *get_network_interface();

// Creates default folder for storing KCM data
extern int create_default_storage_folder();

// Print text on the screen
extern void print_to_screen(int x, int y, const char* buffer);

// Clear screen
extern void clear_screen();

// Toggle led (if available)
extern void toggle_led(void);

// Put led off (if available)
extern void led_off(void);

// Check if button has been pressed (if available)
extern uint8_t button_clicked(void);

// Thread for updating resource value
extern void increment_resource_thread(void* client);

// Print heap allocations
extern void print_heap_stats();

// Print m2mobject sizes
extern void print_m2mobject_stats();

// Create set of objects to test size
extern void create_m2mobject_test_set(M2MObjectList *object_list);

// Wait
extern void do_wait(int timeout_ms);

int run_application(int(*function)(void));

extern bool runProgram(main_t mainFunc);

#ifdef __cplusplus
}
#endif

#endif
