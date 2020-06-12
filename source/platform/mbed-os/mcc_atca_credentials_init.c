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
#include "mcc_atca_credentials_init.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/oid.h"
#include "mbed-trace/mbed_trace.h"
#include "tng_root_cert.h"
#include "key_config_manager.h"
#include "atcacert.h"
#include "atca_status.h"
#include "tng_atca.h"
#include "atcacert_def.h"
#include "atcacert_client.h"
#include "atca_basic.h"
#include "atca_helpers.h"
#include "storage_kcm.h"
#include "fcc_defs.h"
#include "atecc608a_se.h"
#include "cust_def_1_signer.h"
#include "cust_def_2_device.h"
#include "tngtls_cert_def_1_signer.h"
#include "tngtls_cert_def_2_device.h"
#include "tnglora_cert_def_1_signer.h"
#include "tnglora_cert_def_2_device.h"
#define TRACE_GROUP "atml"

/*Global certificate structure pointers, should be set during mcc_atca_init 
 according to credentials and device types*/
atcacert_def_t *g_mcc_cert_def_1_signer = NULL;
atcacert_def_t *g_mcc_cert_def_2_device = NULL;
const uint8_t  *g_mcc_cert_ca_public_key_1_signer = NULL;

/* This file implements SE device certificate decompression.
The decompression steps are:
- Read the signer certificate data from SE using direct atca APIs
- Read the device certificate data from SE using direct atca APIs
- Retrieve the CN attribute of the device certificate and save it as Endpoint name in the device storage by using KCM functionality.
- Create device certificate chain and save it to the device storage by using KCM functionally.*/

/*******************************************************************************
* Definitions
******************************************************************************/
/* === Definitions and Prototypes === */
/*ATCA certificate chain size*/
#define MCC_ATCA_SIGNER_CHAIN_DEPTH     2
/*Signer public key size*/
#define SIGNER_PUBLIC_KEY_MAX_LEN       64

/*******************************************************************************
* Static functions
******************************************************************************/

/*Reads signer certificate from the device*/
static int mcc_atca_read_signer_cert(uint8_t *cert, size_t *cert_size_out)
{

    //read signer cert
    int atca_status = atcacert_read_cert((const atcacert_def_t*)g_mcc_cert_def_1_signer, g_mcc_cert_ca_public_key_1_signer, cert, cert_size_out);
    if (atca_status != ATCACERT_E_SUCCESS) {
        tr_error("atcacert_read_cert error (%" PRIu32 ")", (uint32_t)atca_status);
        return -1;
    }

    tr_debug("Read of signer certificate finished");
    return 0;
}

/*Get CN attribute of the certificate*/
static int mcc_atca_get_cn(const uint8_t *cert, size_t cert_size, uint8_t **cn_out, size_t *cn_size_out)
{
    uint8_t *cert_cn_data = NULL;
    int res = 0;
    char *cn_attribute_name = "CN";
    size_t cn_attribute_name_size = strlen(cn_attribute_name);
    mbedtls_x509_name *asn1_subject = NULL;
    const char *shortName = NULL;
    mbedtls_x509_crt *cert_handler = NULL;

    //Allocate mbedtls x509 certificate handler
    cert_handler = (mbedtls_x509_crt*)malloc(sizeof(mbedtls_x509_crt));
    if (cert_handler == NULL) {
        tr_error("failed to allocate mbedtls_x509_crt ");
        res = -1;
        goto Exit;
    }

    //Initialize certificate handler
    mbedtls_x509_crt_init(cert_handler);

    //Parse certificate
    res = mbedtls_x509_crt_parse_der(cert_handler, (const unsigned char*)cert, cert_size);
    if (res != 0) {
        tr_error("mbedtls_x509_crt_parse_der error (%" PRIu32 ")", (uint32_t)res);
        res = -1;
        goto Exit;
    }

    //Set certificate subject pointer
    asn1_subject = &cert_handler->subject;

    //Read the "CN" attribute
    while (asn1_subject) {

        //Get next oid field
        res = mbedtls_oid_get_attr_short_name(&asn1_subject->oid, &shortName);
        if (res != 0) {
            tr_error("mbedtls_oid_get_attr_short_name error (%" PRIu32 ")", (uint32_t)res);
            res = -1;
            goto Exit;
        }

        //Compare the name of the field to "CN" attribute name
        if (strncmp(shortName, cn_attribute_name, cn_attribute_name_size) == 0) {

            //Allocate memory for certificate attribute
            cert_cn_data = malloc(asn1_subject->val.len);
            if (cert_cn_data == NULL) {
                tr_error("Failed to allocate memory to accommodate CN attribute");
                res = -1;
                goto Exit;
            }

            //Copy the attribute data to allocated buffer
            memcpy(cert_cn_data, asn1_subject->val.p, asn1_subject->val.len);
            //Set output parameters
            *cn_size_out = asn1_subject->val.len;
            *cn_out = cert_cn_data;
            break;
        }
        //Get pointer of the next field
        asn1_subject = asn1_subject->next;
    }

Exit:
    if (res != 0) {
        free(cert_cn_data);
    }

    //Free allocated certificate handler internal data
    mbedtls_x509_crt_free(cert_handler);
    //Free allocated certificate header
    free(cert_handler);
    return res;
}

/*The function reads CN attribute of the device certificate and stores it to the device storage as endpoint item */
static int mcc_store_device_cert_cn(const uint8_t *device_cert, size_t device_cert_size)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    uint8_t *device_cn = NULL;
    size_t device_cn_size = 0;
    int res = 0;

    //Read cn attribute of the device certificate
    res = mcc_atca_get_cn(device_cert, device_cert_size, &device_cn, &device_cn_size);
    if (res != 0) {
        tr_error("psa_drv_atca_get_cn error (%" PRIu32 ")", (uint32_t)kcm_status);
        return -1;
    }

    // store the device certificate CN as a endpoint name config param that is not allowed for deleting
    kcm_status = storage_item_store((const uint8_t *)g_fcc_endpoint_parameter_name, strlen(g_fcc_endpoint_parameter_name),
                                    KCM_CONFIG_ITEM, true, STORAGE_ITEM_PREFIX_KCM, device_cn, device_cn_size, false);

    /*Free memory that was allocated for CN data*/
    free(device_cn); // caller must evacuate this buffer

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("kcm_item_store error (%" PRIu32 ")", (uint32_t)kcm_status);
        return -1;
    }

    tr_debug("Store of endpoint name finished");
    return 0;
}

/* Read device certificate*/
static int mcc_atca_read_device_cert(const uint8_t *signer_certificate, size_t signer_certificate_size, uint8_t *device_certificate, size_t *device_certificate_size_out)
{
    int atca_status = ATCACERT_E_SUCCESS;
    uint8_t signer_public_key[SIGNER_PUBLIC_KEY_MAX_LEN];

    //Read signer public key
    atca_status = atcacert_get_subj_public_key((const atcacert_def_t*)g_mcc_cert_def_1_signer, signer_certificate, signer_certificate_size, signer_public_key);
    if (atca_status != ATCACERT_E_SUCCESS) {
        tr_error("atcacert_get_subj_public_key error (%" PRIu32 ")", (uint32_t)atca_status);
        return -1;
    }

    // read device certificate using signer public key
    atca_status = atcacert_read_cert((const atcacert_def_t*)g_mcc_cert_def_2_device, signer_public_key, device_certificate, device_certificate_size_out);
    if (atca_status != ATCACERT_E_SUCCESS) {
        tr_error("atcacert_read_cert error (%" PRIu32 ")", (uint32_t)atca_status);
        return -1;
    }

    tr_debug("Read of device certificated finished");
    return 0;
}

/*Get max size of the certificate*/
static int mcc_atca_get_max_cert_size(const atcacert_def_t* cert_def, size_t *max_cert_size_out)
{
    int atca_status = ATCACERT_E_SUCCESS;

    // Get signer certificate
    atca_status = atcacert_max_cert_size((const atcacert_def_t*)cert_def, max_cert_size_out);
    if (atca_status != ATCA_SUCCESS) {
        tr_error("atcacert_max_cert_size error (%" PRIu32 ")", (uint32_t)atca_status);
        return -1;
    }
    tr_debug("certificate size is (%" PRIu32 ")", (uint32_t)max_cert_size_out);
    return 0;
}

/*Initialize atca resources */
static int mcc_atca_init(void)
{
    tng_type_t type;

    //Initialize atca driver
    ATCA_STATUS atca_status = atecc608a_init();
    if (atca_status != ATCA_SUCCESS) {
        tr_error("atcab_init error (%" PRIu32 ")", (uint32_t)atca_status);
        return -1;
    }

    /*Initialize atca credentials templates
    If the device is set to default MCHP credentials, the global credential variables should use default templates according to 
    the device type, otherwise - use custom credentials templates*/
    if (g_cert_def_2_device.cert_template == NULL && g_cert_def_1_signer.cert_template == NULL) { //Default credentials

        /*Default credentials :  check tng type and set template pointers according to the type 
        Get TNG type*/
        atca_status = tng_get_type(&type);
        if (atca_status != ATCA_SUCCESS) {
            tr_error("tng_get_type error (%" PRIu32 ")", (uint32_t)atca_status);
            return -1;
        }

        //Set CA public key of the signer, use cryptoauth define
        g_mcc_cert_ca_public_key_1_signer = &g_cryptoauth_root_ca_002_cert[CRYPTOAUTH_ROOT_CA_002_PUBLIC_KEY_OFFSET];

        if (type == TNGTYPE_LORA) {//TNGTYPE_LORA - use g_tnglora_*** structures
            g_mcc_cert_def_1_signer = (atcacert_def_t *)&g_tnglora_cert_def_1_signer;
            g_mcc_cert_def_2_device = (atcacert_def_t *)&g_tnglora_cert_def_2_device;
        } else {//TNGTYPE_22- use g_tng2_*** structures
            g_mcc_cert_def_1_signer = (atcacert_def_t *)&g_tngtls_cert_def_1_signer;
            g_mcc_cert_def_2_device = (atcacert_def_t *)&g_tngtls_cert_def_2_device;
        }
    } else { //Customer credentials - use custom templates
        g_mcc_cert_def_1_signer = (atcacert_def_t *)&g_cert_def_1_signer;
        g_mcc_cert_def_2_device = (atcacert_def_t *)&g_cert_def_2_device;
        //Set CA public key of the signer, use custom define
        g_mcc_cert_ca_public_key_1_signer = (const uint8_t*)&g_cert_ca_public_key_1_signer;
    }
    return 0;
}

void mcc_atca_release(void)
{
    //Release allocated resources
    ATCA_STATUS atca_status = atecc608a_deinit();
    if (atca_status != ATCA_SUCCESS) {
        tr_error("Failed to releasing Atmel's secure element (%" PRIu32 ")", (uint32_t)atca_status);
    }
}

static int mcc_decompress_device_cert_chain(void)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS, close_chain_status = KCM_STATUS_SUCCESS;
    kcm_cert_chain_handle cert_chain_h = NULL;
    size_t device_cert_size = 0, signer_cert_size = 0;
    uint8_t *signer_certificate_buffer = NULL;
    uint8_t *device_certificate_buffer = NULL;
    int res = 0;

    // Create chain for device and signer certificate.
    kcm_status = storage_cert_chain_create(&cert_chain_h, (uint8_t *)g_fcc_bootstrap_device_certificate_name, strlen(g_fcc_bootstrap_device_certificate_name), MCC_ATCA_SIGNER_CHAIN_DEPTH, true, STORAGE_ITEM_PREFIX_KCM);
    if (kcm_status == KCM_STATUS_FILE_EXIST) {
        // Already exist. Skip read and store.
        return 0;
    }
    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("kcm_cert_chain_create failed (%" PRIu32 ")", (uint32_t)kcm_status);
        return -1;
    }

    /*Initialize atca resources*/
    res = mcc_atca_init();
    if (res != 0) {
        tr_error("mcc_atca_init failed");
        goto Exit;
    }

    // query device cert size
    res = mcc_atca_get_max_cert_size(g_mcc_cert_def_2_device, &device_cert_size);
    if (res != 0) {
        tr_error("mcc_atca_get_max_device_cert_size failed");
        goto Exit;
    }

    // query signer cert size
    res = mcc_atca_get_max_cert_size(g_mcc_cert_def_1_signer, &signer_cert_size);
    if (res != 0) {
        tr_error("mcc_atca_get_max_signer_cert_size failed");
        goto Exit;
    }

    // allocate buffers to hold the device and signer certificates
    signer_certificate_buffer = malloc(signer_cert_size);
    if (signer_certificate_buffer == NULL) {
        tr_error("Failed to allocate signer certificate buffer");
        res = -1;
        goto Exit;
    }

    device_certificate_buffer = malloc(signer_cert_size);
    if (device_certificate_buffer == NULL) {
        tr_error("Failed to allocate device certificate buffer");
        res = -1;
        goto Exit;
    }

    // read the signer certificate (signer certificate is the actual device certificate CA)
    res = mcc_atca_read_signer_cert(signer_certificate_buffer, &signer_cert_size);
    if (res != 0) {
        tr_error("mcc_atca_read_signer_cert failed");
        goto Exit;
    }

    // read the device certificate using signer certificate
    res = mcc_atca_read_device_cert(signer_certificate_buffer, signer_cert_size, device_certificate_buffer, &device_cert_size);
    if (res != 0) {
        tr_error("mcc_atca_read_device_cert failed");
        goto Exit;
    }

    // read and store the CN of the device X509 certificate
    res = mcc_store_device_cert_cn(device_certificate_buffer, device_cert_size);
    if (res != 0) {
        tr_error("mcc_store_device_cert_cn failed");
        goto Exit;
    }

    // Store the device and signer certificate as KCM chain that is not allowed for deleting
    // start with the leaf - add device certificate
    kcm_status = storage_cert_chain_add_next(cert_chain_h, device_certificate_buffer, device_cert_size, STORAGE_ITEM_PREFIX_KCM, false);
    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("Failed to add Atmel's device certificate (%" PRIu32 ")", (uint32_t)kcm_status);
        res = -1;
        goto Exit;
    }

    //add signer certificate
    kcm_status = storage_cert_chain_add_next(cert_chain_h, signer_certificate_buffer, signer_cert_size, STORAGE_ITEM_PREFIX_KCM, false);
    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("Failed to add Atmel's signer certificate (%" PRIu32 ")", (uint32_t)kcm_status);
        res = -1;
    }

Exit:
    mcc_atca_release();
    free(device_certificate_buffer);
    free(signer_certificate_buffer);

    close_chain_status = storage_cert_chain_close(cert_chain_h, STORAGE_ITEM_PREFIX_KCM);
    if (close_chain_status != KCM_STATUS_SUCCESS) {
        tr_error("Failed closing certificate chain error (%u)", close_chain_status);
        // modify return status only if function succeed but we failed for storage_cert_chain_close
        res = -1;
    }
    return res;
}
/*******************************************************************************
 * Code
 ******************************************************************************/

int mcc_atca_credentials_init(void)
{
    int res = 0;

    /*The function decompresses SE device certificate chain and stores is to the device storage.*/
    res = mcc_decompress_device_cert_chain();
    if (res != 0) {
        tr_error("mcc_decompress_device_cert_chain failed");
    }

    return res;
}
#endif // MBED_CONF_APP_SECURE_ELEMENT_ATCA_SUPPORT
