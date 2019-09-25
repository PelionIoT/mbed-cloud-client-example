// ----------------------------------------------------------------------------
// Copyright 2016-2019 ARM Ltd.
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


#ifndef MBED_CLOUD_CLIENT_USER_CONFIG_H
#define MBED_CLOUD_CLIENT_USER_CONFIG_H

#define MBED_CLOUD_CLIENT_ENDPOINT_TYPE             "default"
#define MBED_CLOUD_CLIENT_LIFETIME                  3600

#ifdef __FREERTOS__
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE      512
    #define MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP
#elif defined(__SXOS__)
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE      512
    #define MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
#else
#ifdef MBED_CONF_MBED_CLIENT_SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE    MBED_CONF_MBED_CLIENT_SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE
#else
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE      512
#endif
    #define MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP
#endif

/* set flag to enable update support in Pelion Device Management Client */
#if defined(__linux__) || defined(TARGET_LIKE_MBED) || defined(__SXOS__)
    #define MBED_CLOUD_CLIENT_SUPPORT_UPDATE
#endif

/* set download buffer size in bytes (min. 1024 bytes) */
/* Use larger buffers in Linux */
#ifdef __linux__
#define MBED_CLOUD_CLIENT_UPDATE_BUFFER             (2 * 1024 * 1024)
#else
#define MBED_CLOUD_CLIENT_UPDATE_BUFFER             2048
#endif

/* Developer flags for Update feature */
#if MBED_CONF_APP_DEVELOPER_MODE == 1
    #define MBED_CLOUD_DEV_UPDATE_CERT
    #define MBED_CLOUD_DEV_UPDATE_ID
#endif /* MBED_CONF_APP_DEVELOPER_MODE */

#endif /* MBED_CLOUD_CLIENT_USER_CONFIG_H */
