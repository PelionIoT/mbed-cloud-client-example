/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cinttypes>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "FlashIAP.h"
#include "fsl_iap.h"

#if 0
#define debug_print(...) printf(__VA_ARGS__)
#else
#define debug_print(...)
#endif

namespace mbed {

int FlashIAP::init()
{
    return 0;
}

int FlashIAP::deinit()
{
    return 0;
}


int FlashIAP::read(void *buffer, uint32_t addr, uint32_t size)
{
    memcpy(buffer, (void*)addr, size);
    return 0;
}

static void calculate_start_end_sectors(uint32_t addr, uint32_t size, uint32_t& start, uint32_t& end)
{
    start = addr / FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES;
    end = (addr+size) / FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES;
}

int FlashIAP::program(const void *buffer, uint32_t addr, uint32_t size)
{
    debug_print("FlashIAP::program: %" PRIu32 " %" PRIu32 "\r\n", addr, size);

    uint32_t start_sector, end_sector;
    calculate_start_end_sectors(addr, size, start_sector, end_sector);

    status_t res = IAP_PrepareSectorForWrite(start_sector, end_sector);
    debug_print("IAP_PrepareSectorForWrite: %" PRIu32 " %" PRIu32 " returned %ld.\r\n", start_sector, end_sector, res);

    if (res != kStatus_IAP_Success)
        return -1;

    fflush(stdout);

    res = IAP_CopyRamToFlash(addr, (uint32_t*)buffer, size, SystemCoreClock);
    debug_print("IAP_CopyRamToFlash returned %ld.\r\n", res);

    return res == kStatus_IAP_Success ? 0 : -1;
}

int FlashIAP::erase(uint32_t addr, uint32_t size)
{
    debug_print("FlashIAP::erase: %" PRIu32 " %" PRIu32 "\r\n", addr, size);
    uint32_t start_page = addr / FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES;
    uint32_t end_page = ((addr+size) / FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES);
    uint32_t start_sector, end_sector;
    calculate_start_end_sectors(addr, size, start_sector, end_sector);

    status_t res = IAP_PrepareSectorForWrite(start_sector, end_sector);
    debug_print("IAP_PrepareSectorForWrite: %" PRIu32 " %" PRIu32 " returned %ld.\r\n", start_sector, end_sector, res);

    if (res != kStatus_IAP_Success)
        return -1;

    res = IAP_ErasePage(start_page, end_page, SystemCoreClock);

    debug_print("IAP_ErasePage returned %ld.\r\n", res);

    return res == kStatus_IAP_Success ? 0 : -1;
}

uint32_t FlashIAP::get_page_size() const
{
    return FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES;
}

uint32_t FlashIAP::get_sector_size(uint32_t addr) const
{
    return FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES;
}

uint32_t FlashIAP::get_flash_start() const
{
    return 0;
}

uint32_t FlashIAP::get_flash_size() const
{
    return FSL_FEATURE_SYSCON_FLASH_SIZE_BYTES;
}

uint8_t FlashIAP::get_erase_value() const
{
    return 0xff;
}

} // namespace
