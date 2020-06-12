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

#include "flash_api.h"

#include "fsl_iap.h"
#include "fsl_common.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#if 0
#define debug_print(...) printf(__VA_ARGS__)
#else
#define debug_print(...)
#endif

#define FLASH_START_ADDRESS 0

int32_t flash_init(flash_t *obj)
{
    (void) obj;

    return 0;
}

int32_t flash_free(flash_t *obj)
{
    (void) obj;

    return 0;
}


int32_t flash_read(flash_t *obj, uint32_t addr, uint8_t *buffer, uint32_t size)
{
    (void) obj;

    memcpy(buffer, (void*)addr, size);
    return 0;
}

static void calculate_start_end_sectors(uint32_t addr, uint32_t size, uint32_t* start, uint32_t* end)
{
    *start = addr / FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES;
    *end = (addr+size) / FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES;
}

int32_t flash_program_page(flash_t *obj, uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    (void) obj;

    debug_print("flash_program: %" PRIu32 " %" PRIu32 "\r\n", addr, size);

    uint32_t start_sector, end_sector;
    calculate_start_end_sectors(addr, size, &start_sector, &end_sector);

    status_t res = IAP_PrepareSectorForWrite(start_sector, end_sector);
    debug_print("IAP_PrepareSectorForWrite: %" PRIu32 " %" PRIu32 " returned %ld.\r\n", start_sector, end_sector, res);

    if (res != kStatus_IAP_Success)
        return -1;

    /* Ensure source buffer is in RAM and word aligned. */
    uint32_t local_buffer[FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES / sizeof(uint32_t)];
    uint32_t* write_pointer = local_buffer;

    if (((FLASH_START_ADDRESS <= (uint32_t) buffer) &&
        ((uint32_t) buffer < (FLASH_START_ADDRESS + FSL_FEATURE_SYSCON_FLASH_SIZE_BYTES))) ||
        ((uint32_t) buffer % 4)) {

        debug_print("IAP: unaligned access\r\n");

        /* Copy un-aligned buffer one byte at a time. */
        uint8_t* copy_pointer = (uint8_t*) local_buffer;

        for (size_t index = 0; index < FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES; index++) {
            copy_pointer[index] = buffer[index];
        }

    } else {
        write_pointer = (uint32_t*) buffer;
    }

    uint32_t mask = DisableGlobalIRQ();
    res = IAP_CopyRamToFlash(addr, write_pointer, size, SystemCoreClock);
    EnableGlobalIRQ(mask);

    debug_print("IAP_CopyRamToFlash returned %ld.\r\n", res);

    return res == kStatus_IAP_Success ? 0 : -1;
}

int32_t flash_erase_sector(flash_t *obj, uint32_t addr)
{
    (void) obj;

    debug_print("flash_erase: %" PRIu32 "\r\n", addr);
    uint32_t start_page = addr / FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES;
    uint32_t end_page = ((addr + FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES) / FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES);
    uint32_t start_sector, end_sector;
    calculate_start_end_sectors(addr, FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES, &start_sector, &end_sector);

    status_t res = IAP_PrepareSectorForWrite(start_sector, end_sector);
    debug_print("IAP_PrepareSectorForWrite: %" PRIu32 " %" PRIu32 " returned %ld.\r\n", start_sector, end_sector, res);

    if (res != kStatus_IAP_Success)
        return -1;

    uint32_t mask = DisableGlobalIRQ();
    res = IAP_ErasePage(start_page, end_page, SystemCoreClock);
    EnableGlobalIRQ(mask);

    debug_print("IAP_ErasePage returned %ld.\r\n", res);

    return res == kStatus_IAP_Success ? 0 : -1;
}

uint32_t flash_get_sector_size(const flash_t *obj, uint32_t addr)
{
    (void) obj;

    return FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES;
}

uint32_t flash_get_page_size(const flash_t *obj)
{
    (void) obj;

    return FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES;
}

uint32_t flash_get_start_address(const flash_t *obj)
{
    (void) obj;

    return FLASH_START_ADDRESS;
}

uint32_t flash_get_size(const flash_t *obj)
{
    (void) obj;

    return FSL_FEATURE_SYSCON_FLASH_SIZE_BYTES;
}

uint8_t flash_get_erase_value(const flash_t *obj)
{
    (void) obj;

    return 0xFF;
}
