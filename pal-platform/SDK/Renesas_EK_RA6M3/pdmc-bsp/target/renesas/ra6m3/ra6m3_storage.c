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

#include "port_storage.h"

#define USE_HAL_FLASH_API 0

#if USE_HAL_FLASH_API
#include "hal_data.h"
#else
#include "r_flash_hp.h"
#endif

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if 0
#include "SEGGER_RTT.h"
#define SEGGER_INDEX (0)
#define DEBUG_PRINT(...) SEGGER_RTT_printf(SEGGER_INDEX, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#define enter_critical() __disable_irq()
#define exit_critical() __enable_irq()

#define ERASE_VALUE (0xFF)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

#if USE_HAL_FLASH_API
/* Use flash instance from auto-generated project HAL header file. */
#else
/* Use High-performance flash instance directly, bypassing the project generator. */

/* Handle to flash controller, gets populated on open. */
static flash_hp_instance_ctrl_t g_flash_ctrl = { 0 };

/* Flash configuration. Use blocking API. */
static const flash_cfg_t g_flash_cfg = {
    .data_flash_bgo = false,
    .p_callback = NULL,
    .p_extend = NULL,
    .p_context = NULL,
    .ipl = (3),
    .irq = FSP_INVALID_VECTOR,
    .err_ipl = (3),
    .err_irq = FSP_INVALID_VECTOR,
};

/* Flash instance struct: handle, configuration, and function pointer struct. */
static const flash_instance_t g_flash = {
    .p_ctrl = &g_flash_ctrl,
    .p_cfg = &g_flash_cfg,
    .p_api = &g_flash_on_flash_hp
};
#endif

int32_t port_storage_init(void)
{
    DEBUG_PRINT("port_storage_init\r\n");

    /**
     * Configure flash with settings from p_cfg,
     * populate p_ctrl with instance vales.
     */
    fsp_err_t status = g_flash.p_api->open(g_flash.p_ctrl, g_flash.p_cfg);

    return (status == FSP_SUCCESS) ? 0 : -1;
}

int32_t port_storage_deinit(void)
{
    DEBUG_PRINT("port_storage_deinit\r\n");

    /**
     * Clear handle and close flash.
     */
    fsp_err_t status = g_flash.p_api->close(g_flash.p_ctrl);

    return (status == FSP_SUCCESS) ? 0 : -1;
}

int32_t port_storage_erase_sector(uint32_t address)
{
    DEBUG_PRINT("port_storage_erase_sector: %X\r\n", address);

    /**
     * Erase one sector. Underlying driver does alignment check.
     */
    enter_critical();
    fsp_err_t status = g_flash.p_api->erase(g_flash.p_ctrl, address, 1);
    exit_critical();

    return (status == FSP_SUCCESS) ? 0 : -1;
}

int32_t port_storage_read(uint32_t address, uint8_t *data, uint32_t size)
{
    DEBUG_PRINT("port_storage_read: %X %p %X\r\n", address, data, size);

    /**
     * Copy data to buffer using XIP.
     */
    memcpy(data, (void *) address, size);

    return 0;
}

int32_t port_storage_program_page(uint32_t address, const uint8_t *data, uint32_t size)
{
    DEBUG_PRINT("port_storage_program_page: %X %p %X\r\n", address, data, size);

    uint32_t start_address = port_storage_get_start_address();
    uint32_t flash_size = port_storage_get_size();
    uint32_t flash_page_size = port_storage_get_page_size(address);
    fsp_err_t status = FSP_ERR_INVALID_ARGUMENT;

    /* Check address is within range before writing */
    if ((start_address <= address) && (address < (start_address + flash_size))) {

        /* Check for alignment */
        if (((address % flash_page_size) == 0) && ((size % flash_page_size) == 0)) {

            uint32_t write_address = address;
            uint32_t offset = 0;
            status = FSP_SUCCESS;

            /* Program page */
            while ((offset < size) && (status == FSP_SUCCESS))
            {
                uint32_t read_pointer = (uint32_t) data + offset;
                uint32_t write_pointer = read_pointer;
                uint8_t buffer[flash_page_size];

                /* Copy data to RAM buffer if source is in flash. */
                if ((start_address <= read_pointer) && (read_pointer < (start_address + flash_size))) {
                    port_storage_read(read_pointer, buffer, flash_page_size);
                    write_pointer = (uint32_t) buffer;
                }

                enter_critical();
                status = g_flash.p_api->write(g_flash.p_ctrl, write_pointer, write_address, flash_page_size);
                exit_critical();

                write_address += flash_page_size;
                offset += flash_page_size;
            }
        }
    }

    return (status == FSP_SUCCESS) ? 0 : -1;
}

uint32_t port_storage_get_sector_size(uint32_t address)
{
    DEBUG_PRINT("port_storage_get_sector_size: %X\r\n", address);

    uint32_t sector_size = 0;
    flash_info_t info = { 0 };

    /* Get flash layout structure. */
    fsp_err_t result = g_flash.p_api->infoGet(g_flash.p_ctrl, &info);

    if (result == FSP_SUCCESS) {

        /* Step through each flash region. */
        for (size_t index = 0; index < info.code_flash.num_regions; index++) {

            /**
             * Find region containing address.
             */
            uint32_t start = info.code_flash.p_block_array[index].block_section_st_addr;
            uint32_t end = info.code_flash.p_block_array[index].block_section_end_addr;

            if ((start <= address) && (address <= end)) {

                /* Set sector size and break from loop. */
                sector_size = info.code_flash.p_block_array[index].block_size;
                break;
            }
        }
    }

    return sector_size;
}

uint32_t port_storage_get_page_size(uint32_t address)
{
    DEBUG_PRINT("port_storage_get_page_size: %X\r\n", address);

    uint32_t page_size = 0;
    flash_info_t info = { 0 };

    /* Get flash layout structure. */
    fsp_err_t result = g_flash.p_api->infoGet(g_flash.p_ctrl, &info);

    if (result == FSP_SUCCESS) {

        /* Step through each flash region. */
        for (size_t index = 0; index < info.code_flash.num_regions; index++) {

            /**
             * Find region containing address.
             */
            uint32_t start = info.code_flash.p_block_array[index].block_section_st_addr;
            uint32_t end = info.code_flash.p_block_array[index].block_section_end_addr;

            if ((start <= address) && (address <= end)) {

                /* Set page size and break from loop. */
                page_size = info.code_flash.p_block_array[index].block_size_write;
                break;
            }
        }
    }

    return page_size;
}

uint32_t port_storage_get_start_address(void)
{
    DEBUG_PRINT("port_storage_get_start_address\r\n");

    uint32_t start_address = 0;
    flash_info_t info = { 0 };

    /* Get flash layout structure. */
    fsp_err_t result = g_flash.p_api->infoGet(g_flash.p_ctrl, &info);

    if ((result == FSP_SUCCESS) && (info.code_flash.num_regions != 0)) {

        /* Use the first regions start address. */
        start_address = info.code_flash.p_block_array[0].block_section_st_addr;
    }

    return start_address;
}

uint32_t port_storage_get_size(void)
{
    DEBUG_PRINT("port_storage_get_size\r\n");

    uint32_t flash_size = 0;
    flash_info_t info = { 0 };

    /* Get flash layout structure. */
    fsp_err_t result = g_flash.p_api->infoGet(g_flash.p_ctrl, &info);

    if (result == FSP_SUCCESS) {

        /* Step through each flash region. */
        for (size_t index = 0; index < info.code_flash.num_regions; index++) {

            /**
             * Calculate each regions size and add to total flash size.
             */
            uint32_t start = info.code_flash.p_block_array[index].block_section_st_addr;
            uint32_t end = info.code_flash.p_block_array[index].block_section_end_addr;

            /* The extra 1 accounts for the end address being inside the region. */
            flash_size += (end - start + 1);
        }
    }

    return flash_size;
}

uint8_t port_storage_get_erase_value(void)
{
    DEBUG_PRINT("port_storage_get_erase_value\r\n");

    return ERASE_VALUE;
}
