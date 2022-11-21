// ----------------------------------------------------------------------------
// Copyright 2022 Pelion.
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

// Only compile in Mbed OS
#if defined __MBED__

#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#endif

#include "mbed.h"
#include "mbed-trace/mbed_trace.h"
#include "KVStore.h"
#include "kvstore_global_api.h"

using namespace mbed;

#define TRACE_GROUP "MIGR"
#define MAX_CONTENT_LEN 1024     // Max certificate length
#define backupBootstrapURI  "pelion_bCfgParam_mbed.BootstrapServerURI"
#define workingBootstrapURI "pelion_wCfgParam_mbed.BootstrapServerURI"
#define backupBootstrapCA   "pelion_bCrtae_mbed.BootstrapServerCACert"
#define workingBootstrapCA  "pelion_wCrtae_mbed.BootstrapServerCACert"
#define migrationKey        "pelion_wMIGR"

// Please note that migration state is NOT reset after a sucesfull migration.
// If you have a need to do multiple migrations, you should make a special SW
// update at some stage to disable migration and also remove the migrationKey
// at that stage OR alternatively change the name of the key (say pelion_wMIGR2)
// for 2nd migration etc. so that a unique state per migration is retained.
typedef struct rep_key_info {
    const char      *key;            // KVSStore key name
    const uint8_t   *value;          // New value to replace
    size_t          size;            // New value length/size
    bool            printable;       // Is entry printable string?
} rep_key_info;

/* Definitions for bootstrap cert info via the migrate_server_cert.h */

extern const uint8_t MIGRATE_BOOTSTRAP_CA_CERT[];
extern const uint32_t MIGRATE_BOOTSTRAP_CA_CERT_SIZE;

/* Internal function prototypes */
static int kvstore_locate_and_replace(rep_key_info *);
static int kvstore_clear_LwM2M_info(void);

/* migrate_kvstore      Change the bootstrap server address.
                        NOTE! Supports only Mbed OS / KVStore and production mode.

    There are two sets of keys that need to be updated.
    Working set and backup set. They both need to be deleted and re-recreated with the new value.

*/
int migrate_kvstore(const char *new_BS_server) {
    int status;
    kv_info_t info;
    rep_key_info bURI = { backupBootstrapURI, 
                        (const uint8_t *) new_BS_server,
                        strlen(backupBootstrapURI),
                        true,  // Printable
                        };
    rep_key_info wURI  = { workingBootstrapURI, 
                        (const uint8_t *) new_BS_server,
                        strlen(backupBootstrapURI),
                        true,  // Printable
                        };
    rep_key_info bCert  = { backupBootstrapCA, 
                        MIGRATE_BOOTSTRAP_CA_CERT,
                        MIGRATE_BOOTSTRAP_CA_CERT_SIZE,
                        false,  // Printable
                        };
    rep_key_info wCert  = { workingBootstrapCA, 
                        MIGRATE_BOOTSTRAP_CA_CERT,
                        MIGRATE_BOOTSTRAP_CA_CERT_SIZE,
                        false,  // Printable
                        };

    // Check migration status, skip if key is found.
    status = kv_get_info(migrationKey, &info);
    if (status == MBED_SUCCESS) {
        tr_info("Migration done already, %s exists.", migrationKey);
        return status;
    }
    // Migration NOT done, let's do it.
    // Always do locate and replace, it will only change the value if it's not
    // already what it should be. We get value back via bool changed if the value
    // is what it should be.
    status = kvstore_locate_and_replace(&bURI);
    status = status + kvstore_locate_and_replace(&wURI);
    status = status + kvstore_locate_and_replace(&bCert);
    status = status + kvstore_locate_and_replace(&wCert);
    status = status + kvstore_clear_LwM2M_info();
    if (status != MBED_SUCCESS) {
        // Failure to update any of the keys is pretty fatal, as
        // either connection or RFS will fail
        tr_error("ERROR - Migration failed, %d", status);
        return status;
    }
    // Create migration key to mark "all done".
    status = kv_set(migrationKey, &status, 1, 0);
    if (status != MBED_SUCCESS) {
        // Not failing whole operation, because the actual relevant keys
        // managed to be changed. Flag write failing is not fatal, it will
        // just cause unnecessary further LwM2M cert wiping.
        tr_error("ERROR - Can't write migration key %s - error, %d", migrationKey, status);
    }
    else {
        printf("Migration done.\n");
    }
    return status;
}

/*
    locate_and_replace      find the given key (with name),
                            verify if the names is matching migration value
                            and if not, set the new value to it.

                            remove() did not work for some reason.
*/
static int kvstore_locate_and_replace(rep_key_info *repKey) {

    size_t actual_size = 0;
    kv_info_t info;
    int res;
    char kv_value[MAX_CONTENT_LEN + 1];

    res = kv_get_info(repKey->key, &info);
    if (res != MBED_SUCCESS) {
        // Can't get key info - key likely not existing?
        tr_warn("WARNING - kv_get_info failed %d for %s", res, repKey->key);
    }
    else {
        // Does the value already match new value (i.e. our 2nd or n+ run here)
        if (MAX_CONTENT_LEN < info.size) {
            tr_error("ERROR - migrate - buffer len < info.size %d", info.size);
            return MBED_ERROR_INVALID_SIZE;
        }
        memset(kv_value, 0, sizeof(kv_value));
        res = kv_get(repKey->key, kv_value, info.size, &actual_size);
        if (res != MBED_SUCCESS) {
            tr_error("ERROR - kv_get failed with %d for %s", res, repKey->key);
            return res;
        }
        if (actual_size == info.size && 
            repKey->size == actual_size &&
            0 == memcmp(kv_value, repKey->value, repKey->size) ) {
            // We have a match, same string in both - just return with success
            tr_info("Migration for %s already done.", repKey->key);
            return MBED_SUCCESS;
        }
    }

    res = kv_set(repKey->key, repKey->value, repKey->size, info.flags);
    if (res != MBED_SUCCESS) {
        tr_error("ERROR - kv_set failed %d for %s", res, repKey->key);
        return res;
    }
    else {
        if (repKey->printable) {
            tr_info("kv_set %s succeeded, written new value %s", repKey->key, repKey->value);
        }
        else {
            tr_info("kv_set %s succeeded, written len %d bytes", repKey->key, repKey->size);
        }
    }
    return res;
}

static int kvstore_clear_LwM2M_info() {

    const char *keys[] = {"pelion_wCfgParam_mbed.LwM2MServerURI",
                          "pelion_wCrtae_mbed.LwM2MDeviceCert",
                          "pelion_wCrtae_mbed.LwM2MServerCACert",
                          "pelion_wPrvKey_mbed.LwM2MDevicePrivateKey",
                           };
    kv_info_t info;
    int index = 0;
    int failed = MBED_SUCCESS;
    int rounds = (sizeof(keys)/sizeof(char*));      // Nbr of keys to remove
    int res;

    while (index < rounds) {
        res = kv_get_info(keys[index], &info);
        if (res == MBED_ERROR_ITEM_NOT_FOUND) {
            // No such key.
            tr_info("Key %s does not exist (OK).", keys[index]);
            res = MBED_SUCCESS;
            index++;
            continue;
        }
        res = kv_remove(keys[index]);
        if (res != MBED_SUCCESS) {
            tr_error("ERROR - failed to remove %s, code %d", keys[index], res);
            failed = res;
        }
        else {
            tr_info("Removed %s as part of migration.", keys[index]);
        }
        index++;
    }

    if (failed != MBED_SUCCESS) {
        res = failed;
    }
    else {
        tr_warn("LwM2M credentials wiped to ensure bootstrap.");
    }
    return res;
}

#endif // __MBED__
