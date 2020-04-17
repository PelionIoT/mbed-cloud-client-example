// ----------------------------------------------------------------------------
// Copyright 2018-2020 ARM Ltd.
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
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"

#if PORT_DEBUG
#define PORT_LOG_DEBUG configPRINTF
#else
#define PORT_LOG_DEBUG(...)
#endif

using namespace std; 
void * operator new(size_t size) 
{ 
    PORT_LOG_DEBUG("new(%d) called.\n", size);
    void * p = malloc(size); 
    return p; 
} 
  
void operator delete(void * p) 
{ 
    PORT_LOG_DEBUG("delete(%08X) called.\n", p);
    free(p); 
} 

void* operator new[](size_t size)
{
    PORT_LOG_DEBUG("new[](%d) called.\n", size);
    void * p = malloc(size); 
    return p; 
}

void operator delete[](void* p)
{
    PORT_LOG_DEBUG("delete[](%08X) called.\n", p);
    free(p); 
}

