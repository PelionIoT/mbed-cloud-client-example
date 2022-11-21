// ----------------------------------------------------------------------------
// Copyright 2016-2022 Pelion.
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

#ifndef MIGRATE_KVSTORE_H
#define MIGRATE_KVSTORE_H

/* migrate_kvstore      Change the bootstrap server address.
                        NOTE! Supports only Mbed OS / KVStore and production mode.

    There are two sets of keys that need to be updated.
    Working set and backup set. They both need to be deleted and re-recreated with the new value.

*/
#if defined __MBED__
int migrate_kvstore(const char *new_value);
#else
    #if  defined MBED_CLOUD_CLIENT_MIGRATE_BOOTSTRAP
        #error "No migration_kvstore implementation for other than Mbed OS using KVStore."
    #endif
// Dummy implementation for other OSes
int migrate_kvstore(const char *new_value) {
    (void) new_value;
    return 0;
}
#endif

#endif //MIGRATE_KVSTORE_H

