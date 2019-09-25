// ----------------------------------------------------------------------------
// Copyright 2019 ARM Ltd.
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

#ifndef PSA_STORAGE_USER_CONFIG_H
#define PSA_STORAGE_USER_CONFIG_H

/**
* \def PSA_STORAGE_FILE_C_STORAGE_PREFIX
*
* Define the path to the directory for Internal Trusted Storage
* (PSA ITS) files representing persisted objects. For example,
* to store files in "/home/username" define
* PSA_STORAGE_FILE_C_STORAGE_PREFIX "/home/username/"
* (note the appended "/").
*/
#ifdef PSA_STORAGE_FILE_C_STORAGE_PREFIX
    #undef PSA_STORAGE_FILE_C_STORAGE_PREFIX
#endif

#define PSA_STORAGE_FILE_C_STORAGE_PREFIX "psa/"

#endif /* PSA_STORAGE_USER_CONFIG_H */
