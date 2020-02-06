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
#include "atcacert/atcacert_def.h"

/*This file used for default MCHP device credentials. 
  To use custom credentials, this file should be overwritten by cust_def_2_device.c file that is
  created by MCHP generator tool*/

/*Device certificate structure - g_cert_def_2_device will be repointed during atca initialization to relevant tng templates */
const atcacert_def_t g_cert_def_2_device = {
    .cert_elements = NULL,
    .cert_elements_count = 0,
    .cert_template = NULL,
    .cert_template_size = 0
};
#endif //MBED_CONF_APP_SECURE_ELEMENT_ATCA_SUPPORT

