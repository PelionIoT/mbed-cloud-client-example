// ----------------------------------------------------------------------------
// Copyright 2016-2021 Pelion.
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

#ifndef PDMC_EXAMPLE_H
#define PDMC_EXAMPLE_H

#include "m2minterface.h"

// Starting with 5s, after every iteration waiting time is multiplied by 2 -> max waiting time is with 9 is 1280s (~21min)
// it is this long as with bad cellular rssi register and plmn selection might take 5mins so we wan't to give it a few tries.
#define MAX_PDMC_CLIENT_CONNECTION_ERROR_COUNT 9

void pdmc_init();

void pdmc_close();

bool pdmc_connect();

void pdmc_resume();

bool pdmc_registered();

bool pdmc_paused();

bool pdmc_register_called();

M2MObjectList *pdmc_get_object_list();

bool create_pdmc_resources();

#endif // PDMC_EXAMPLE_H
