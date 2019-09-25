// ----------------------------------------------------------------------------
// Copyright 2018-2019 ARM Ltd.
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

#include "pal.h"
#include "hal_sys.h"
#include "errorcode.h"

#define TRACE_GROUP "ROT"

#if (PAL_USE_HW_ROT)

#define PAL_DEVICE_KEY_SIZE_IN_BYTES 16

palStatus_t pal_plat_osGetRoTFromHW(uint8_t *keyBuf, size_t keyLenBytes)
{
    UINT32 csStatus;

    // Check parameters
    if (keyLenBytes < PAL_DEVICE_KEY_SIZE_IN_BYTES) {
        return PAL_ERR_BUFFER_TOO_SMALL;
    }

    if (NULL == keyBuf) {
        return PAL_ERR_NULL_POINTER;
    }

    csStatus = hal_SysEnterCriticalSection();
    if (ERR_SUCCESS != memd_Flash_security_REG_Read(ROT_MEM_ADDR, PAL_DEVICE_KEY_SIZE_IN_BYTES, keyBuf)) {
        return PAL_ERR_GET_DEV_KEY;
    }
    hal_SysExitCriticalSection(csStatus);

    return PAL_SUCCESS;
}
#endif
