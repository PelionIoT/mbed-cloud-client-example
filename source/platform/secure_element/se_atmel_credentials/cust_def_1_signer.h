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
#ifndef CERT_DEF_1_SIGNER_H
#define CERT_DEF_1_SIGNER_H

#include "atcacert/atcacert_def.h"

extern const uint8_t g_cert_ca_public_key_1_signer[];
extern const atcacert_def_t g_cert_def_1_signer;
#endif
#endif //MBED_CONF_APP_SECURE_ELEMENT_ATCA_SUPPORT

