/* mbed Microcontroller Library
 * Copyright (c) 2020 ARM Limited
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

#include "PALBlockDevice.h"

#include "port_storage.h"

using namespace mbed;
#include <inttypes.h>

#define READ_SIZE 1

// Debug available
#ifndef FLASHIAP_DEBUG
#define FLASHIAP_DEBUG      0
#endif

#if FLASHIAP_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

mbed::BlockDevice* mbed::BlockDevice::get_default_instance()
{
    static PALBlockDevice default_bd;

    return &default_bd;
}


PALBlockDevice::PALBlockDevice()
{
    init();
}

PALBlockDevice::~PALBlockDevice()
{
    deinit();
}

int PALBlockDevice::init()
{
    return port_storage_init();
}

int PALBlockDevice::deinit()
{
    return port_storage_deinit();
}

int PALBlockDevice::read(void *buffer,
                            bd_addr_t address,
                            bd_size_t size)
{
    uint32_t offset = port_storage_get_start_address();

    return port_storage_read(address + offset, (uint8_t*) buffer, size);
}

int PALBlockDevice::program(const void *buffer,
                               bd_addr_t address,
                               bd_size_t size)
{
    uint32_t offset = port_storage_get_start_address();

    return port_storage_program_page(address + offset, (const uint8_t*) buffer, size);
}

int PALBlockDevice::erase(bd_addr_t address,
                             bd_size_t size)
{
    int result = 0;
    uint32_t offset = port_storage_get_start_address();

    uint32_t erase_address = address + offset;
    uint32_t end_address = address + offset + size;

    /* erase one sector at a time */
    while ((erase_address < end_address) && (result == 0)) {

        /* erase current sector */
        result = port_storage_erase_sector(erase_address);

        /* set next erase address */
        erase_address += port_storage_get_sector_size(erase_address);
    }

    return result;
}

bd_size_t PALBlockDevice::get_read_size() const
{
    return READ_SIZE;
}

bd_size_t PALBlockDevice::get_program_size() const
{
    uint32_t address = port_storage_get_start_address();

    return port_storage_get_page_size(address);
}

bd_size_t PALBlockDevice::get_erase_size() const
{
    uint32_t address = port_storage_get_start_address();

    return port_storage_get_sector_size(address);
}

bd_size_t PALBlockDevice::get_erase_size(bd_addr_t address) const
{
    return port_storage_get_sector_size(address);
}

int PALBlockDevice::get_erase_value() const
{
    return port_storage_get_erase_value();
}

bd_size_t PALBlockDevice::size() const
{
    return port_storage_get_size();
}

const char *PALBlockDevice::get_type() const
{
    return "STORAGE";
}
