/*
 * Copyright (c) 2016-2018 ARM Limited. All rights reserved.
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
 
#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

/* #define PLATFORM_ENABLE_BUTTON 1 for enabling button.*/
#ifndef PLATFORM_ENABLE_BUTTON
#define PLATFORM_ENABLE_BUTTON 0
#endif 

/* #define PLATFORM_ENABLE_LED 1 for enabling led.*/
#ifndef PLATFORM_ENABLE_LED
#define PLATFORM_ENABLE_LED 0 
#endif

// Resets storage to an empty state.
// Use this function when you want to clear storage from all the factory-tool generated data and user data.
// After this operation device must be injected again by using factory tool or developer certificate.
#ifndef RESET_STORAGE
#define RESET_STORAGE 0
#endif

#ifndef MCC_PLATFORM_AUTO_PARTITION
#define MCC_PLATFORM_AUTO_PARTITION 0
#endif

#endif /* COMMON_CONFIG_H */ 

