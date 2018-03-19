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


#ifndef APPLICATION_INIT_H
#define APPLICATION_INIT_H

/*
 * application_init() runs the following initializations:
 *  1. trace initialization
 *  2. platform initialization
 *  3. print memory statistics if MBED_HEAP_STATS_ENABLED is defined
 *  4. FCC initialization.
 */

extern bool application_init(void);
extern bool rmFirmwareImages(void);
extern int reformat_storage(void);

#endif //APPLICATION_INIT_H

