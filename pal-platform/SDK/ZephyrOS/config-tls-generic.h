/* Copyright (c) 2021 Pelion
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

/*****************************************************************************/
/* Pelion Client                                                             */
/*****************************************************************************/

/**
 * Platform agnostic Mbed TLS settings used by Pelion Client.
 */
#include "pelion_mbedtls_config.h"

/**
 * Platform specific Mbed TLS settings used by Pelion Client.
 */

/**
 * Default to factory injected entropy seed which doesn't require special
 * hardware. For platforms that do support hardware entropy, use hardware
 * entropy instead of entropy seed.
 */

/**
 * \def MBEDTLS_NO_PLATFORM_ENTROPY
 *
 * Do not use built-in platform entropy functions.
 * This is useful if your platform does not support
 * standards like the /dev/urandom or Windows CryptoAPI.
 *
 * Uncomment this macro to disable the built-in platform entropy functions.
 */
#define MBEDTLS_NO_PLATFORM_ENTROPY

/**
 * \def MBEDTLS_ENTROPY_NV_SEED
 *
 * Enable the non-volatile (NV) seed file-based entropy source.
 * (Also enables the NV seed read/write functions in the platform layer)
 *
 * This is crucial (if not required) on systems that do not have a
 * cryptographic entropy source (in hardware or kernel) available.
 *
 * Requires: MBEDTLS_ENTROPY_C, MBEDTLS_PLATFORM_C
 *
 * \note The read/write functions that are used by the entropy source are
 *       determined in the platform layer, and can be modified at runtime and/or
 *       compile-time depending on the flags (MBEDTLS_PLATFORM_NV_SEED_*) used.
 *
 * \note If you use the default implementation functions that read a seedfile
 *       with regular fopen(), please make sure you make a seedfile with the
 *       proper name (defined in MBEDTLS_PLATFORM_STD_NV_SEED_FILE) and at
 *       least MBEDTLS_ENTROPY_BLOCK_SIZE bytes in size that can be read from
 *       and written to or you will get an entropy source error! The default
 *       implementation will only use the first MBEDTLS_ENTROPY_BLOCK_SIZE
 *       bytes from the file.
 *
 * \note The entropy collector will write to the seed file before entropy is
 *       given to an external source, to update it.
 */
#define MBEDTLS_ENTROPY_NV_SEED

/*****************************************************************************/
/* Generic Mbed TLS settings                                                 */
/*****************************************************************************/

// used by TCP2 in Zephyr 2.6
#define MBEDTLS_MD5_C

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

// Reduces size particularly in case PSA crypto is used
#undef MBEDTLS_CHACHA20_C
#undef MBEDTLS_CHACHAPOLY_C
#undef MBEDTLS_POLY1305_C

// Do not save a copy of the peer certificate.
// This will reduce the RAM consumption roughly by 1500 bytes.
#undef MBEDTLS_SSL_KEEP_PEER_CERTIFICATE

#include "mbedtls/check_config.h"

#if defined(MBEDTLS_TEST_NULL_ENTROPY)
#warning "MBEDTLS_TEST_NULL_ENTROPY has been enabled. This " \
    "configuration is not secure and is not suitable for production use"
#endif

#endif /* MBEDTLS_CONFIG_H */
