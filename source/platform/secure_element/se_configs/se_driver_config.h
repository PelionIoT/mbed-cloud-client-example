// ----------------------------------------------------------------------------
// Copyright 2019-2020 ARM Ltd.
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
#ifndef __SE_DRIVER_CONFIG_H__
#define __SE_DRIVER_CONFIG_H__

#ifdef MBED_CONF_MBED_CLOUD_CLIENT_SECURE_ELEMENT_SUPPORT
#ifdef MBED_CONF_APP_SECURE_ELEMENT_ATCA_SUPPORT
#include "atecc608a_se.h"
#endif

#ifdef MBED_CONF_APP_SECURE_ELEMENT_PARSEC_TPM_SUPPORT
#include "parsec_se_driver.h"
#endif


#include <stdbool.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*This file defines SE driver lifetime and psa driver methods.*/

#ifdef MBED_CONF_APP_SECURE_ELEMENT_ATCA_SUPPORT
/*SE driver lifetime value*/
#define PSA_DRIVER_SE_DRIVER_LIFETIME_VALUE        PSA_ATECC608A_LIFETIME
/*SE driver methods*/
psa_drv_se_t *g_se_driver_info = &atecc608a_drv_info;
#endif

#ifdef MBED_CONF_APP_SECURE_ELEMENT_PARSEC_TPM_SUPPORT
/*SE driver lifetime value*/
#define PSA_DRIVER_SE_DRIVER_LIFETIME_VALUE        PARSEC_SE_DRIVER_LIFETIME
/*SE driver methods*/
psa_drv_se_t *g_se_driver_info = &PARSEC_SE_DRIVER;
#endif

#ifdef __cplusplus
}
#endif
#endif //#ifdef MBED_CONF_APP_SECURE_ELEMENT_SUPPORT
#endif //__SE_DRIVER_CONFIG_H__
