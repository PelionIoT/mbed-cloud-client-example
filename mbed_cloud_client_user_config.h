// ----------------------------------------------------------------------------
// Copyright 2016-2020 ARM Ltd.
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

#ifndef MBED_CLOUD_CLIENT_ENDPOINT_TYPE
#define MBED_CLOUD_CLIENT_ENDPOINT_TYPE "default"
#endif

#define MBED_CLOUD_CLIENT_LIFETIME                  (2*60*60)

// Option to migrate the bootstrap server to another URI.
// You must specify both, activate feature via 
// #define MBED_CLOUD_CLIENT_MIGRATE_BOOTSTRAP and
// new address via 
// #define MBED_CLOUD_CLIENT_MIGRATE_BOOTSTRAP_TO "coaps://new-bootstrap-server-uri.com:port"
/*
#define MBED_CLOUD_CLIENT_MIGRATE_BOOTSTRAP
#define MBED_CLOUD_CLIENT_MIGRATE_BOOTSTRAP_TO "coaps://new-bootstrap-server-uri.com:port"
*/

#ifdef LWM2M_COMPLIANT
#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP
#define MBED_CONF_MBED_CLIENT_MAX_CERTIFICATE_SIZE   2048
#endif

#if !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP) && !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP) && !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE)
#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP
#endif

#ifdef MBED_CONF_MBED_CLIENT_SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE      MBED_CONF_MBED_CLIENT_SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE
#elif defined LWM2M_COMPLIANT
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE      1024
#else
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE      512
#endif

/* Developer flags for Update feature */
#if defined(MBED_CONF_APP_DEVELOPER_MODE) &&  (MBED_CONF_APP_DEVELOPER_MODE == 1)
    #define MBED_CLOUD_DEV_UPDATE_CERT
    #define MBED_CLOUD_DEV_UPDATE_ID
#endif /* MBED_CONF_APP_DEVELOPER_MODE */

/*
// Try port 443 instead of default CoAP port, will fallback to CoAP port 5684 if 443 fails.
// NOTE! Only port 443 is supported as alternative port in the cloud service.
*/
//#define MBED_CLOUD_CLIENT_CUSTOM_URI_PORT           443

#endif /* MBED_CLOUD_CLIENT_USER_CONFIG_H */
