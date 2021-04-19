// ----------------------------------------------------------------------------
// Copyright 2020-2021 Pelion
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

#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE

#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#endif

#include <inttypes.h>

#if defined(MBED_CLOUD_CLIENT_FOTA_ENABLE) || defined(MBED_CLOUD_CLIENT_SUPPORT_UPDATE)

#warning "Please run manifest-tool init ... to generate proper update certificates"

#ifdef MBED_CLOUD_DEV_UPDATE_ID
const uint8_t arm_uc_vendor_id[16] = { 0 };
const uint16_t arm_uc_vendor_id_size = sizeof(arm_uc_vendor_id);

const uint8_t arm_uc_class_id[16]  = { 0 };
const uint16_t arm_uc_class_id_size = sizeof(arm_uc_class_id);
#endif

#ifdef MBED_CLOUD_DEV_UPDATE_CERT
const uint8_t arm_uc_default_fingerprint[32] = { 0 };
const uint16_t arm_uc_default_fingerprint_size =
    sizeof(arm_uc_default_fingerprint);

const uint8_t arm_uc_default_certificate[1] = { 0 };
const uint16_t arm_uc_default_certificate_size =
    sizeof(arm_uc_default_certificate);
#endif

#ifdef MBED_CLOUD_DEV_UPDATE_RAW_PUBLIC_KEY
const uint8_t arm_uc_update_public_key[] = { "public_key" };
#endif

#ifdef MBED_CLOUD_DEV_UPDATE_PSK
const uint8_t arm_uc_default_psk[1] = { 0 };
const uint8_t arm_uc_default_psk_size = sizeof(arm_uc_default_psk);
const uint16_t arm_uc_default_psk_bits = sizeof(arm_uc_default_psk) * 8;

const uint8_t arm_uc_default_psk_id[1] = { 0 };
const uint8_t arm_uc_default_psk_id_size = sizeof(arm_uc_default_psk_id);
#endif

#endif // MBED_CLOUD_CLIENT_FOTA_ENABLE or MBED_CLOUD_CLIENT_SUPPORT_UPDATE
