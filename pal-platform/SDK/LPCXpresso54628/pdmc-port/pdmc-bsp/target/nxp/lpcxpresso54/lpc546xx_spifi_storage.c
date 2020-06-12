/* mbed Microcontroller Library
 * Copyright (c) 2018-2020 ARM Limited
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

#include "fsl_spifi.h"
#include "fsl_common.h"
#include "fsl_iocon.h"

#include <stdio.h>
#include <inttypes.h>

#if 0
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SPIFIF_INSTANCE SPIFI0
#define COMMAND_NUM (6)
#define READ (0)
#define PROGRAM_PAGE (1)
#define GET_STATUS (2)
#define ERASE_SECTOR (3)
#define WRITE_ENABLE (4)
#define WRITE_REGISTER (5)
#define START_ADDRESS (0x10000000)
#define PAGE_SIZE (256)
#define FLASH_SIZE (128*1024*1024)
#define READ_PAGE_SIZE (256)
#define SECTOR_SIZE (4096)
#define ERASE_VALUE (0xFF)
#define SPI_FREQUENCY (96000000)


/* FLASH_W25Q */
#if defined(FLASH_W25Q)
#define QUAD_MODE_VAL 0x02
#define WRITE_PAGE_SIZE (4)
static spifi_command_t command[COMMAND_NUM] = {
    {READ_PAGE_SIZE, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B},
    {WRITE_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x32},
    {1, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x05},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x20},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x06},
    {1, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x31}};
#elif defined(FLASH_MX25R)
#define QUAD_MODE_VAL 0x40
#define WRITE_PAGE_SIZE (4)
static spifi_command_t command[COMMAND_NUM] = {
    {READ_PAGE_SIZE, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B},
    {WRITE_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandOpcodeSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x38},
    {1, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x05},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x20},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x06},
    {1, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x01}};
#elif defined(FLASH_MT25Q)
#define WRITE_PAGE_SIZE (256)
static spifi_command_t command[COMMAND_NUM] = {
    {READ_PAGE_SIZE, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B},
    {WRITE_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandOpcodeSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x38},
    {1, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x05},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x20},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x06},
    {1, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x61}};
#else
// LPCXpresso546XX eval board revision C or D support FLASH_MT25Q.
// LPCXpresso546XX eval board revision E or later support FLASH_W25Q.
#error "Select the correct QSPI for the board. Options are FLASH_W25Q, FLASH_MX25R and FLASH_MT25Q."
#endif

static void check_if_finish()
{
    uint8_t val = 0;
    /* Check WIP bit */
    do
    {
        SPIFI_SetCommand(SPIFIF_INSTANCE, &command[GET_STATUS]);
        while ((SPIFIF_INSTANCE->STAT & SPIFI_STAT_INTRQ_MASK) == 0U)
        {
        }
        val = SPIFI_ReadDataByte(SPIFIF_INSTANCE);
    } while (val & 0x1);
}

#if defined QUAD_MODE_VAL
static void enable_quad_mode()
{
    /* Write enable */
    SPIFI_SetCommand(SPIFIF_INSTANCE, &command[WRITE_ENABLE]);

    /* Set write register command */
    SPIFI_SetCommand(SPIFIF_INSTANCE, &command[WRITE_REGISTER]);

    SPIFI_WriteDataByte(SPIFIF_INSTANCE, QUAD_MODE_VAL);

    check_if_finish();
}
#endif


#define IOCON_PIO_DIGITAL_EN 0x0100u  /*!<@brief Enables digital function */
#define IOCON_PIO_FUNC1 0x01u         /*!<@brief Selects pin function 1 */
#define IOCON_PIO_FUNC6 0x06u         /*!<@brief Selects pin function 6 */
#define IOCON_PIO_INPFILT_OFF 0x0200u /*!<@brief Input filter disabled */
#define IOCON_PIO_INV_DI 0x00u        /*!<@brief Input function is not inverted */
#define IOCON_PIO_MODE_INACT 0x00u    /*!<@brief No addition pin function */
#define IOCON_PIO_MODE_PULLUP 0x20u   /*!<@brief Selects pull-up function */
#define IOCON_PIO_OPENDRAIN_DI 0x00u  /*!<@brief Open drain is disabled */
#define IOCON_PIO_SLEW_STANDARD 0x00u /*!<@brief Standard mode, output slew rate control is enabled */

static void BOARD_InitPins(void)
{
    /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */
    CLOCK_EnableClock(kCLOCK_Iocon);

    const uint32_t port0_pin23_config = (/* Pin is configured as SPIFI_CSN */
                                         IOCON_PIO_FUNC6 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN23 (coords: N7) is configured as SPIFI_CSN */
    IOCON_PinMuxSet(IOCON, 0U, 23U, port0_pin23_config);

    const uint32_t port0_pin24_config = (/* Pin is configured as SPIFI_IO(0) */
                                         IOCON_PIO_FUNC6 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN24 (coords: M7) is configured as SPIFI_IO(0) */
    IOCON_PinMuxSet(IOCON, 0U, 24U, port0_pin24_config);

    const uint32_t port0_pin25_config = (/* Pin is configured as SPIFI_IO(1) */
                                         IOCON_PIO_FUNC6 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN25 (coords: K8) is configured as SPIFI_IO(1) */
    IOCON_PinMuxSet(IOCON, 0U, 25U, port0_pin25_config);

    const uint32_t port0_pin26_config = (/* Pin is configured as SPIFI_CLK */
                                         IOCON_PIO_FUNC6 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN26 (coords: M13) is configured as SPIFI_CLK */
    IOCON_PinMuxSet(IOCON, 0U, 26U, port0_pin26_config);

    const uint32_t port0_pin27_config = (/* Pin is configured as SPIFI_IO(3) */
                                         IOCON_PIO_FUNC6 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN27 (coords: L9) is configured as SPIFI_IO(3) */
    IOCON_PinMuxSet(IOCON, 0U, 27U, port0_pin27_config);

    const uint32_t port0_pin28_config = (/* Pin is configured as SPIFI_IO(2) */
                                         IOCON_PIO_FUNC6 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN28 (coords: M9) is configured as SPIFI_IO(2) */
    IOCON_PinMuxSet(IOCON, 0U, 28U, port0_pin28_config);

    const uint32_t port0_pin29_config = (/* Pin is configured as FC0_RXD_SDA_MOSI */
                                         IOCON_PIO_FUNC1 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN29 (coords: B13) is configured as FC0_RXD_SDA_MOSI */
    IOCON_PinMuxSet(IOCON, 0U, 29U, port0_pin29_config);

    const uint32_t port0_pin30_config = (/* Pin is configured as FC0_TXD_SCL_MISO */
                                         IOCON_PIO_FUNC1 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN30 (coords: A2) is configured as FC0_TXD_SCL_MISO */
    IOCON_PinMuxSet(IOCON, 0U, 30U, port0_pin30_config);
}

int32_t port_storage_init(void)
{
    spifi_config_t config = { 0 };
    uint32_t sourceClockFreq;

    DEBUG_PRINT("port_storage_init\r\n");

    /* Configure pins */
    BOARD_InitPins();

    /* Set the clock divider */
    CLOCK_AttachClk(kFRO_HF_to_SPIFI_CLK);
    sourceClockFreq = CLOCK_GetFroHfFreq();
    CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, sourceClockFreq / SPI_FREQUENCY, false);

    /* Initialize SPIFI */
    SPIFI_GetDefaultConfig(&config);
    SPIFI_Init(SPIFIF_INSTANCE, &config);

#if defined QUAD_MODE_VAL
    /* Enable Quad mode */
    enable_quad_mode();
#endif

    /* Setup memory command */
    SPIFI_SetMemoryCommand(SPIFIF_INSTANCE, &command[READ]);

    /* Enable interrupt */
    EnableIRQ(SPIFI0_IRQn);

    return 0;
}

int32_t port_storage_deinit(void)
{
    SPIFI_Deinit(SPIFIF_INSTANCE);

    return 0;
}

int32_t port_storage_erase_sector(uint32_t address)
{
    int32_t result = -1;

    DEBUG_PRINT("port_storage_erase_sector: %" PRIX32 "\r\n", address);

    /* Check address is within range before erasing */
    if ((address >= START_ADDRESS) && (address < (START_ADDRESS + FLASH_SIZE))) {

        uint32_t erase_address = address - START_ADDRESS;

        /* Reset the SPIFI to switch to command mode */
        SPIFI_ResetCommand(SPIFIF_INSTANCE);

        /* Write enable */
        SPIFI_SetCommand(SPIFIF_INSTANCE, &command[WRITE_ENABLE]);

        /* Set address */
        SPIFI_SetCommandAddress(SPIFIF_INSTANCE, erase_address);

        /* Erase sector */
        SPIFI_SetCommand(SPIFIF_INSTANCE, &command[ERASE_SECTOR]);

        /* Check if finished */
        check_if_finish();

        /* Setup memory command */
        SPIFI_SetMemoryCommand(SPIFIF_INSTANCE, &command[READ]);

        result = 0;
    }

    return result;
}

int32_t port_storage_read(uint32_t address, uint8_t *data, uint32_t size)
{
    DEBUG_PRINT("port_storage_read: %" PRIX32 " %p %" PRIX32 "\r\n", address, data, size);

    /* Setup memory command */
    SPIFI_SetMemoryCommand(SPIFIF_INSTANCE, &command[READ]);

    memcpy(data, (const void *)address, size);

    return 0;
}

int32_t port_storage_program_page(uint32_t address, const uint8_t *data, uint32_t size)
{
    int32_t result = -1;

    DEBUG_PRINT("port_storage_program_page: %" PRIX32 " %p %" PRIX32 "\r\n", address, data, size);

    /* Check address is within range before writing */
    if ((address >= START_ADDRESS) && (address < (START_ADDRESS + FLASH_SIZE))) {

        /* Check for alignment */
        if (((address % WRITE_PAGE_SIZE) == 0) && ((size % WRITE_PAGE_SIZE) == 0)) {

            uint32_t write_address = address - START_ADDRESS;
            uint32_t offset = 0;

            /* Reset the SPIFI to switch to command mode */
            SPIFI_ResetCommand(SPIFIF_INSTANCE);

            /* Program page */
            while (offset < size)
            {
                /* Write enable */
                SPIFI_SetCommand(SPIFIF_INSTANCE, &command[WRITE_ENABLE]);

                /* Set address */
                SPIFI_SetCommandAddress(SPIFIF_INSTANCE, write_address);

                /* Write page */
                SPIFI_SetCommand(SPIFIF_INSTANCE, &command[PROGRAM_PAGE]);

                for (size_t page = 0; page < WRITE_PAGE_SIZE; page += 4) {

                    uint32_t word = 0;

                    /* Combine bytes to word */
                    for (size_t byte = 0; byte < 4; byte++) {
                        word |= ((uint32_t)(data[offset + page + byte])) << (byte * 8);
                    }

                    /* Write word */
                    SPIFI_WriteData(SPIFIF_INSTANCE, word);
                }

                check_if_finish();

                write_address += WRITE_PAGE_SIZE;
                offset += WRITE_PAGE_SIZE;
            }

            /* Setup memory command */
            SPIFI_SetMemoryCommand(SPIFIF_INSTANCE, &command[READ]);

            result = 0;
        }
    }

    return result;
}

uint32_t port_storage_get_sector_size(uint32_t address)
{
    uint32_t sectorsize = 0;

    DEBUG_PRINT("port_storage_get_sector_size: %" PRIX32 "\r\n", address);

    if ((address >= START_ADDRESS) && (address < (START_ADDRESS + FLASH_SIZE))) {
        sectorsize = SECTOR_SIZE;
    }

    return sectorsize;
}

uint32_t port_storage_get_page_size(uint32_t address)
{
    (void) address;

    return WRITE_PAGE_SIZE;
}

uint32_t port_storage_get_start_address(void)
{
    return START_ADDRESS;
}

uint32_t port_storage_get_size(void)
{
    return FLASH_SIZE;
}

uint8_t port_storage_get_erase_value(void)
{
    return ERASE_VALUE;
}
