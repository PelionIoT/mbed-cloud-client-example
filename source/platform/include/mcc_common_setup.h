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

#ifndef MCC_COMMON_SETUP_H
#define MCC_COMMON_SETUP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*main_t)(void);

// Initialize platform
// related platform specific initializations required.
//
// @returns
//   0 for success, anything else for error
int mcc_platform_init(void);

// Initialize network connection
int mcc_platform_init_connection(void);

// Close network connection
int mcc_platform_close_connection(void);

// Return network interface.
void *mcc_platform_get_network_interface(void);

// Format storage (DEPRECATED)
int mcc_platform_reformat_storage(void);

// initialize common details for storage for storing KCM data etc.
// creates default folders, reformat.
int mcc_platform_storage_init(void);

// Wait
void mcc_platform_do_wait(int timeout_ms);

// for printing sW build info
void mcc_platform_sw_build_info(void);

/*!
 * @brief mcc_platform_run_program - Start the OS with the main function
 * @param testMain_t mainTestFunc  - main function to run
 * @return void
 */
int  mcc_platform_run_program(main_t mainFunc);
#ifdef __cplusplus
}
#endif

#endif // #ifndef MCC_COMMON_SETUP_H
