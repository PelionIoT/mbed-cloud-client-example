/* mbed Microcontroller Library
 * Copyright (c) 2020 ARM Limited
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

/***********************************************************************************************************************
* File Name    : FlashIAP_Renesas_RX65N-CK.cpp
* Description  : This module implements the FLASH IAP (flash in application programming)
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include <string.h>
#include <stdio.h>
#include "FlashIAP.h"

extern "C"
{
    #include "r_flash_rx_if.h"
    #include "r_flash_rx65n.h" // to get address mapping
    #include "r_flash_rx.h"
    #include "r_flash_group.h" // to get status error code
}

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
// Debug available
#ifndef FLASHIAP_DEBUG
    #define FLASHIAP_DEBUG 0
#endif

#if FLASHIAP_DEBUG
    #define debug_print(...) printf(__VA_ARGS__)
#else
    #define debug_print(...)
#endif

#if defined(MCU_RX65N) && defined(BSP_MCU_RX65N)
    // Initialization of RAM area blocks
    // RAM : ORIGIN = 0x4, LENGTH = 262140, end = 0x0004 0000
    // RAM2 : ORIGIN = 0x00800000, LENGTH = 393216, end = 0x0086 0000
    #define MCU_MEMORY_RAM_START_ADDR   0x4            
    #define MCU_MEMORY_RAM2_START_ADDR  0x00800000   
    #define MCU_MEMORY_RAM_END_ADDR     0x00040000
    #define MCU_MEMORY_RAM2_END_ADDR    0x00860000
#else
    #error "Unsupported memory allocation of target board!"
#endif

#define FLASHIAP_AREA_START_ADDR    MBED_CONF_STORAGE_TDB_INTERNAL_INTERNAL_BASE_ADDRESS // allow write from KVStore start ...
#define FLASHIAP_AREA_PAGE_SIZE     FLASH_CF_MIN_PGM_SIZE                           // 128 Bytes
#define FLASHIAP_AREA_SIZE          (MBED_CONF_STORAGE_TDB_INTERNAL_INTERNAL_SIZE + MBED_CONF_UPDATE_CLIENT_STORAGE_SIZE) // KVStore size + Update Storage
#define FLASHIAP_AREA_END_ADDR      (FLASHIAP_AREA_START_ADDR + FLASHIAP_AREA_SIZE) // ... to app metadata header & bootloader start
#define ERASE_VALUE                 (0xFF)
#ifndef MBED_FLASH_INVALID_SIZE
    #define MBED_FLASH_INVALID_SIZE     0xFFFFFFFF
#endif
/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
External functions
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/

static inline bool is_aligned_to(uint32_t addr, uint32_t size, uint32_t alignment)
{
    if ( ((addr % alignment) == 0) && ((size % alignment) == 0))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool is_lay_in_boundary(uint32_t addr, uint32_t size)
{
    bool return_status = true;

    // Verification of address in avaliable flash area
    if ((addr > FLASHIAP_AREA_END_ADDR) || (addr < FLASHIAP_AREA_START_ADDR) || ((addr + size) > FLASHIAP_AREA_END_ADDR))
    {
        debug_print("FlashIAP:: Address 0x%08x out of bound.\n\r", addr);
        return_status = false;
    }
    // Check for alignment to min page size
    else if ( !is_aligned_to(addr, size, FLASHIAP_AREA_PAGE_SIZE) )
    {
        debug_print("FlashIAP:: Address 0x%08x and size %i aren't alignment.\n\r", addr, size);
        return_status = false;
    }
    return return_status;
}

static inline bool is_ram_addr(uint32_t addr)
{
    // Copy data to RAM buffer if source is in flash.
    if ( ((MCU_MEMORY_RAM_START_ADDR <= addr) && (addr < MCU_MEMORY_RAM_END_ADDR)) 
        || ((MCU_MEMORY_RAM2_START_ADDR <= addr) && (addr < MCU_MEMORY_RAM2_END_ADDR)) )
    {
        debug_print("FlashIAP:: Source in RAM (take current data from RAM).\n\r");
        return true;      // source in RAM (take current data from RAM)
    }
    else
    {
        debug_print("FlashIAP:: Source in flash.\n\r");
        return false;    // source in flash
    }
}

namespace mbed
{

/** Initialize a flash IAP device
*
*  Should be called once per lifetime of the object.
*  @return 0 on success or a negative error code on failure
*/
int FlashIAP::init()
{
    flash_err_t status = FLASH_SUCCESS;

    // Initialize the flash peripheral.
    status = R_FLASH_Open();

    if (FLASH_SUCCESS == status)
    {
        debug_print("FlashIAP::init() Successfully init.\n\r");
    }
    else
    {
        debug_print("FlashIAP::init() Unsuccessful init.\n\r");
    }
    return (FLASH_SUCCESS == status) ? 0 : (-status);
}

/** Deinitialize a flash IAP device
 *
 *  @return 0 on success or a negative error code on failure
 */
int FlashIAP::deinit()
{
    flash_err_t status = FLASH_SUCCESS;

    // Closes the flash driver.
    status = R_FLASH_Close();

    if (FLASH_SUCCESS == status)
    {
        debug_print("FlashIAP::deinit() Successfully deinit.\n\r");
    }
    else
    {
        debug_print("FlashIAP::deinit() Unsuccessful deinit.\n\r");
    }
    return (FLASH_SUCCESS == status) ? 0 : (-status);
}

/** Read data from a flash device.
 *
 *  This method invokes memcpy - reads number of bytes from the address
 *
 *  @param buffer Buffer to write to
 *  @param addr   Flash address to begin reading from
 *  @param size   Size to read in bytes
 *  @return       always 0
 */
int FlashIAP::read(void *buffer, uint32_t addr, uint32_t size)
{
    memcpy(buffer, (void *)addr, size);
    debug_print("FlashIAP::read() Successfully read buffer 0x%x, size 0x%x.\n\r", buffer, size);

    return 0;
}

/** Program data to pages
 *
 *  The sectors must have been erased prior to being programmed
 *
 *  @param buffer Buffer of data to be written
 *  @param addr   Address of a page to begin writing to
 *  @param size   Size to write in bytes, must be a multiple of program size
 *  @return       0 on success, negative error code on failure
 */
int FlashIAP::program(const void *buffer, uint32_t addr, uint32_t size)
{
    flash_err_t status = FLASH_SUCCESS;
    debug_print("port_storage_program_page: from 0x%08x to 0x%08x of %i Bytes\r\n", buffer, addr, size);

    // Verification of address in correct area and check for alignment
    if ( !is_lay_in_boundary(addr, size) )
    {
        return (-FLASH_ERR_FAILURE);
    }

    int number_of_cycles = 1;
    if (size > FLASHIAP_AREA_PAGE_SIZE)
    {
        number_of_cycles = size / FLASHIAP_AREA_PAGE_SIZE;
    }

    uint32_t read_pointer = (uint32_t)buffer;   // read from address
    uint32_t write_pointer = addr;              // write to address
    uint32_t data_pointer = NULL;               // data pointer to source buffer
    uint32_t curr_ram_buffer[FLASHIAP_AREA_PAGE_SIZE]; 

    while ((number_of_cycles))
    {
        // Copy data to RAM buffer if source is in flash.
        if (is_ram_addr(read_pointer)) 
        {
            data_pointer = read_pointer;      // source in RAM (take current data from RAM)
        }
        else
        {
            memcpy(curr_ram_buffer, (void *)read_pointer, FLASHIAP_AREA_PAGE_SIZE);     // source in flash
            data_pointer = (uint32_t) curr_ram_buffer;
        }

        debug_print("FlashIAP::program()::DEBUG number_of_cycles %i\n\r", number_of_cycles);
        debug_print("Destination address 0x08%x Size %i bytes\n\r", write_pointer, FLASHIAP_AREA_PAGE_SIZE);

        portDISABLE_INTERRUPTS();

        status = R_FLASH_Write((uint32_t)data_pointer, write_pointer, FLASHIAP_AREA_PAGE_SIZE);

        portENABLE_INTERRUPTS();

        // Verify results
        if (memcmp((const void *)data_pointer, (const void *)write_pointer, FLASHIAP_AREA_PAGE_SIZE))
        {
            debug_print("FlashIAP::program() Verify results -> FLASH_ERR_FAILURE.\n\r");            
        }

        read_pointer += FLASHIAP_AREA_PAGE_SIZE;
        write_pointer += FLASHIAP_AREA_PAGE_SIZE;
        number_of_cycles--;
    }

    if (FLASH_SUCCESS == status)
    {
        debug_print("FlashIAP::program() Successfully write.\n\r");
        // Verify results
        if (memcmp(buffer, (const void *)addr, size))
        {
            debug_print("FlashIAP::program() Verify results -> FLASH_ERR_FAILURE.\n\r");
            status = FLASH_ERR_FAILURE;
        }
    }
    return (FLASH_SUCCESS == status) ? 0 : (-status);
}

/** Erase sectors
 *
 *  The state of an erased sector is undefined until it has been programmed
 *
 *  @param addr Address of a sector to begin erasing, must be a multiple of the sector size
 *  @param size Size to erase in bytes, must be a multiple of the sector size
 *  @return     0 on success, negative error code on failure
 */
int FlashIAP::erase(uint32_t addr, uint32_t size)
{
    flash_err_t status = FLASH_SUCCESS;
    uint32_t num_bloks_to_erase;
    uint32_t curr_addr_to_erase = addr;

    // Verification of address in correct area
    if ( !is_lay_in_boundary(addr, size) )
    {
        debug_print("FlashIAP::erase() Verification of address -> FLASH_ERR_FAILURE.\n\r");
        return (-FLASH_ERR_FAILURE);
    }

    if ( is_aligned_to_sector(addr, size) )
    {
        num_bloks_to_erase = size / get_sector_size(addr);
    }
    else
    {
        return (-FLASH_ERR_FAILURE);
    }
    debug_print("FlashIAP::erase() INput -> address 0x%x num_bloks_to_erase %i .\n\r", addr, num_bloks_to_erase);

    portDISABLE_INTERRUPTS();
    // LOOP THROUGH EACH BLOCK
    // This cycle is added caused by Erase function implementation (decrementing addr from start)
    while (num_bloks_to_erase)
    {
        status = R_FLASH_Erase((flash_block_address_t)curr_addr_to_erase, 1);
        curr_addr_to_erase += get_sector_size(addr);
        num_bloks_to_erase--;
    }
    portENABLE_INTERRUPTS();

    if (FLASH_SUCCESS != status)
    {
        debug_print("FlashIAP::erase() FLASH_ERR_FAILURE.\n\r");
    }
    return (status == FLASH_SUCCESS) ? 0 : (-status);
}

/** Get the program page size
     *
     *  The page size defines the writable page size
     *  @return Size of a program page in bytes
     */
uint32_t FlashIAP::get_page_size() const
{
    return FLASHIAP_AREA_PAGE_SIZE;
}

/** Get the sector size at the defined address
     *
     *  Sector size might differ at address ranges.
     *  An example <0-0x1000, sector size=1024; 0x10000-0x20000, size=2048>
     *
     *  @param addr Address of or inside the sector to query
     *  @return Size of a sector in bytes or MBED_FLASH_INVALID_SIZE if not mapped
     */
uint32_t FlashIAP::get_sector_size(uint32_t addr) const
{
    uint32_t flash_area_sector_size = 0;
    
    // Check if address is in boundary of Code Flash Area
    if ( (addr > FLASH_CF_BLOCK_END) || (addr < FLASH_CF_BLOCK_INVALID) )
    {
        return MBED_FLASH_INVALID_SIZE;
    }
#ifndef FLASH_IN_DUAL_BANK_MODE                     /* LINEAR MODE */
    if (addr >= FLASH_CF_BLOCK_7)
    {
        flash_area_sector_size = 8192;          // 8 KiB
    }
    else
    {
        flash_area_sector_size = 32768;         // 32 KiB
    }
#else                                               /* DUAL MODE */
    if (addr >= FLASH_CF_BLOCK_7) || ((addr >= FLASH_CF_BLOCK_45) && (addr <= FLASH_CF_BLOCK_38))
    {
        flash_area_sector_size = 8192; // 8 KiB
    }
    else
    {
        flash_area_sector_size = 32768; // 32 KiB
    }
#endif

    return flash_area_sector_size;
}

/** Get the flash start address
     *
     *  @return Flash start address
     */
uint32_t FlashIAP::get_flash_start() const
{
    return FLASHIAP_AREA_START_ADDR;
}

/** Get the flash size
     *
     *  @return Flash size
     */
uint32_t FlashIAP::get_flash_size() const
{
    return FLASHIAP_AREA_SIZE;
}

/** Get the flash erase value
     *
     *  Get the value we read after erase operation
     *  @return flash erase value
     */
uint8_t FlashIAP::get_erase_value() const
{
    return ERASE_VALUE;
}

/* Check if address and size are aligned to a sector
     *
     *  @param addr Address of block to check for alignment
     *  @param size Size of block to check for alignment
     *  @return true if the block is sector aligned, false otherwise
     */
bool FlashIAP::is_aligned_to_sector(uint32_t addr, uint32_t size)
{
    uint32_t current_sector_size = get_sector_size(addr);
    return is_aligned_to(addr, size, current_sector_size);
}

} // namespace mbed
