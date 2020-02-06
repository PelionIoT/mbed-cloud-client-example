// ----------------------------------------------------------------------------
// Copyright 2019-2020 ARM Ltd.
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
#ifdef MBED_CONF_APP_SECURE_ELEMENT_ATCA_SUPPORT
#ifndef MCC_ATCA_CREDENTIALS_INIT_H
#define MCC_ATCA_CREDENTIALS_INIT_H

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* === APIs === */
/** Initializes Atmel Secure Element peripheral and calls direct Atmel driver APIs to retrieve device certificate and endpoint name items.
* The retrieved items saved to the storage under fcc restrictions via Key Configuration Manager APIs.
*
* @returns ::KCM_STATUS_SUCCESS in case of success or one of the `::kcm_status_e` errors otherwise.
*/
int mcc_atca_credentials_init(void);
#ifdef __cplusplus
}
#endif
#endif /* MCC_ATCA_CREDENTIALS_INIT_H */
#endif // MBED_CONF_MBED_CLOUD_CLIENT_SECURE_ELEMENT_ATCA_SUPPORT

