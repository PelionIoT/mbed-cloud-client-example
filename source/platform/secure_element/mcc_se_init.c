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
#ifdef MBED_CONF_MBED_CLOUD_CLIENT_SECURE_ELEMENT_SUPPORT
#include "mcc_se_init.h"
#include "mbed-trace/mbed_trace.h"
#ifdef MBED_CONF_APP_SECURE_ELEMENT_ATCA_SUPPORT
#include "mcc_atca_credentials_init.h"
#endif
#define TRACE_GROUP "secm"


/*******************************************************************************
 * Code
 ******************************************************************************/

int mcc_se_init(void)
{
    int res = 0;

#ifdef MBED_CONF_APP_SECURE_ELEMENT_ATCA_SUPPORT
    /*The function decompresses SE device certificate chain and stores is to the device storage.*/
    res = mcc_atca_credentials_init();
    if (res != 0) {
        tr_error("mcc_atca_credentials_init failed");
    }
#endif

    return res;
}
#endif // MBED_CONF_MBED_CLOUD_CLIENT_SECURE_ELEMENT_SUPPORT
