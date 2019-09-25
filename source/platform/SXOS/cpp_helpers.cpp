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

#include "cos.h"

#include <assert.h>

void* operator new(size_t count)
{
    return COS_Malloc((UINT32)count, COS_MMI_HEAP);
}

void* operator new[](size_t count)
{
    return COS_Malloc((UINT32)count, COS_MMI_HEAP);
}

void operator delete(void* ptr)
{
    // unlike other free() implementations, the COS version does not
    // handle NULL
    if (ptr) {
        COS_Free(ptr);
    }
}

// XXX: m2mfirmware needs this for some reason
void operator delete(void *ptr, size_t sz)
{
    if (ptr) {
        COS_Free(ptr);
    }
}

void operator delete[](void* ptr)
{
    if (ptr) {
        COS_Free(ptr);
    }
}

extern "C"
void __cxa_pure_virtual()
{
    assert(false);
}
