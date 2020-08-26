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
#ifndef __SE_DATA_USER_CONFIG_H__
#define __SE_DATA_USER_CONFIG_H__

#ifdef MBED_CONF_MBED_CLOUD_CLIENT_SECURE_ELEMENT_SUPPORT
#include "fcc_defs.h"
#include "se_slot_manager_defs.h"
#include <stdbool.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef MBED_CONF_MBED_CLOUD_CLIENT_NON_PROVISIONED_SECURE_ELEMENT
/*This file defines the number of Secure Element pre provisioned items and their information like kcm type, name and SE slot number.
  The item's information stored to the device during storage initialization */

#define SE_DATA_NUMBER_OF_PREPROVISIONED_ITEMS    1 //Total number of SE preprovisioned slots

sem_preprovisioned_item_data_s g_sem_preprovisioned_data[SE_DATA_NUMBER_OF_PREPROVISIONED_ITEMS] = {
    {//KCM type,KCM item name, SE slot number
        .kcm_item_type = KCM_PRIVATE_KEY_ITEM,
        .kcm_item_name = g_fcc_bootstrap_device_private_key_name,
        .se_slot_num = 0
    }
};
#endif

#ifdef __cplusplus
}
#endif
#endif //MBED_CONF_MBED_CLOUD_CLIENT_SECURE_ELEMENT_SUPPORT
#endif //__SE_DATA_USER_CONFIG_H__
