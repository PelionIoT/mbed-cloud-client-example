/*
 * Copyright (c) 2020 Arm Limited
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

#include "Driver_Flash.h"

#include <string.h>
#include "RTE_Components.h"
#include CMSIS_device_header

#include "stm32l4xx_hal.h"

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  ((void)arg)
#endif


/* Driver version */
#define ARM_FLASH_DRV_VERSION   ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

static int32_t flash_unlock(void);

static int32_t flash_lock(void);

static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,  /* Defined in the CMSIS Flash Driver header file */
    ARM_FLASH_DRV_VERSION
};

/**
 * \brief Flash driver capability macro definitions \ref ARM_FLASH_CAPABILITIES
 */
/* Flash Ready event generation capability values */
#define EVENT_READY_NOT_AVAILABLE   (0u)
#define EVENT_READY_AVAILABLE       (1u)
/* Data access size values */
#define DATA_WIDTH_8BIT             (0u)
#define DATA_WIDTH_16BIT            (1u)
#define DATA_WIDTH_32BIT            (2u)
/* Chip erase capability values */
#define CHIP_ERASE_NOT_SUPPORTED    (0u)
#define CHIP_ERASE_SUPPORTED        (1u)

/* Driver Capabilities */
static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    EVENT_READY_NOT_AVAILABLE,
    DATA_WIDTH_32BIT,
    CHIP_ERASE_SUPPORTED
};

/**
 * \brief Flash status macro definitions \ref ARM_FLASH_STATUS
 */
/* Busy status values of the Flash driver  */
#define DRIVER_STATUS_IDLE      (0u)
#define DRIVER_STATUS_BUSY      (1u)

/* Error status values of the Flash driver */
#define DRIVER_STATUS_NO_ERROR  (0u)
#define DRIVER_STATUS_ERROR     (1u)

/**
 * \brief Arm Flash device structure.
 */
struct arm_flash_dev_t {
    ARM_FLASH_INFO *data;       /*!< FLASH memory device data */
};

/**
 * \brief      Check if the Flash memory boundaries are not violated.
 * \param[in]  flash_dev  Flash device structure \ref arm_flash_dev_t
 * \param[in]  offset     Highest Flash memory address which would be accessed.
 * \return     Returns true if Flash memory boundaries are not violated, false
 *             otherwise.
 */

static bool is_range_valid(struct arm_flash_dev_t *flash_dev, uint32_t offset)
{
    uint32_t flash_limit = 0;    

    /* Calculating the highest address of the Flash memory address range */
    flash_limit = FLASH_BASE + FLASH_END;

    return (offset > flash_limit) ? (false) : (true) ;
}

/**
 * \brief        Check if the parameter is aligned to program_unit.
 * \param[in]    flash_dev  Flash device structure \ref arm_flash_dev_t
 * \param[in]    param      Any number that can be checked against the
 *                          program_unit, e.g. Flash memory address or
 *                          data length in bytes.
 * \return       Returns true if param is aligned to program_unit, false
 *               otherwise.
 */

static bool is_write_aligned(struct arm_flash_dev_t *flash_dev,
                             uint32_t param)
{
    return ((param % flash_dev->data->program_unit) != 0) ? (false) : (true);
}

/**
 * \brief        compute bank number.
 * \param[in]    flash_dev  Flash device structure \ref arm_flash_dev_t
 * \param[in]    param      &n address in flash
 * \return       Returns true if param is aligned to sector_unit, false
 *               otherwise.
 */
static uint32_t bank_number(struct arm_flash_dev_t *flash_dev,
                             uint32_t param)
{
    uint32_t bank = 0;
#if defined(SYSCFG_MEMRMP_FB_MODE)
    if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0) {
        /* No Bank swap */
        if (param < (FLASH_BASE + FLASH_BANK_SIZE)) {
            bank = FLASH_BANK_1;
        } else {
            bank = FLASH_BANK_2;
        }
    } else {
        /* Bank swap */
        if (param < (FLASH_BASE + FLASH_BANK_SIZE)) {
            bank = FLASH_BANK_2;
        } else {
            bank = FLASH_BANK_1;
        }
    }
#else
    /* Device like L432KC */
    bank = FLASH_BANK_1;
#endif

    return bank;
}

/**
 * \brief        compute page number.
 * \param[in]    flash_dev  Flash device structure \ref arm_flash_dev_t
 * \param[in]    param      &n address in flash
 * \return       Returns page number to erase. 
 */
static uint32_t page_number(struct arm_flash_dev_t *flash_dev, uint32_t param)
{
    uint32_t page = 0;

    page = (param - FLASH_BASE) / FLASH_PAGE_SIZE;

    return page;
}

static ARM_FLASH_SECTOR ARM_FLASH0_DEV_SECTOR_DATA = {
    .start = FLASH_BASE,
    .end = FLASH_END
};

static ARM_FLASH_INFO ARM_FLASH0_DEV_DATA = {
    .sector_info    = &ARM_FLASH0_DEV_SECTOR_DATA,  /* Uniform sector layout */
    .sector_count   = (FLASH_END + 1- FLASH_BASE) / 0x800,
    .sector_size    = 0x800,
    .page_size      = 8u,                           /* Page size is the minimum programable size, which 8 bytes */
    .program_unit   = 8u,                           /* Minimum write size in bytes */
    .erased_value   = 0xFF
};

static struct arm_flash_dev_t ARM_FLASH0_DEV = {
    .data = &(ARM_FLASH0_DEV_DATA)
};

/* Flash Status */
static ARM_FLASH_STATUS ARM_FLASH0_STATUS = {0, 0, 0};

static ARM_DRIVER_VERSION ARM_Flash_GetVersion(void)
{
    return DriverVersion;
}

static ARM_FLASH_CAPABILITIES ARM_Flash_GetCapabilities(void)
{
    return DriverCapabilities;
}

static int32_t ARM_Flash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
  ARG_UNUSED(cb_event);
  FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
  return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_Uninitialize(void)
{
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_PowerControl(ARM_POWER_STATE state)
{
    switch(state) {
    case ARM_POWER_FULL:
        /* Nothing to be done */
        return ARM_DRIVER_OK;
    case ARM_POWER_OFF:
    case ARM_POWER_LOW:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }
}

static int32_t ARM_Flash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    bool is_valid = true;

    ARM_FLASH0_STATUS.error = DRIVER_STATUS_NO_ERROR;

    /* Check Flash memory boundaries */
    is_valid = is_range_valid(&ARM_FLASH0_DEV, addr + cnt -1);

    if(is_valid != true) {
        ARM_FLASH0_STATUS.error = DRIVER_STATUS_ERROR;        
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    memcpy(data,(void*)((uint32_t)addr), cnt);

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_ProgramData(uint32_t address, const void *data, uint32_t size)
{
    uint32_t loop = 0;
    HAL_StatusTypeDef err = HAL_OK;

    uint32_t StartAddress = 0;

    ARM_FLASH0_STATUS.error = DRIVER_STATUS_NO_ERROR;

    if ((size % 8) != 0) {
        /* L4 flash devices can only be programmed 64bits/8 bytes at a time */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (flash_unlock() != HAL_OK) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    ARM_FLASH0_STATUS.busy = DRIVER_STATUS_BUSY;

    /* Clear error programming flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    /* Program the user Flash area word by word */
    StartAddress = address;
 

    /*  HW needs an aligned address to program flash, which data
     *  parameters doesn't ensure  */
    if ((uint32_t) data % 4 != 0) {        
        volatile uint64_t data64;
        while ((address < (StartAddress + size)) && (err == HAL_OK)) {
            for (uint8_t i = 0; i < 8; i++) {
                *(((uint8_t *) &data64) + i) = *((uint8_t*)data + i);
            }

            err = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data64);
            if (err == HAL_OK) {
                address = address + 8;
                data = data + 8;
            } else {
                err = HAL_ERROR;
            }
        }
    } else { /*  case where data is aligned, so let's avoid any copy */        
        while ((address < (StartAddress + size)) && (err == HAL_OK)) {
            err = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, *((uint64_t *) data));
            if (err == HAL_OK) {
                address = address + 8;
                data = data + 8;
            } else {
                err = HAL_ERROR;
            }
        }
    }

    ARM_FLASH0_STATUS.busy = DRIVER_STATUS_IDLE;

    flash_lock();

    if (err == HAL_OK) {        
        return ARM_DRIVER_OK;
    } else {        
        return ARM_DRIVER_ERROR;
    }    
}

static int32_t ARM_Flash_EraseSector(uint32_t address)
{    
    
    
    HAL_StatusTypeDef err = HAL_OK;    
    uint32_t FirstPage = 0, BankNumber = 0;
    uint32_t PAGEError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    int32_t status = 0;

    if ((address >= (FLASH_BASE + FLASH_SIZE)) || (address < FLASH_BASE)) {
        return ARM_DRIVER_ERROR;
    }

    if (flash_unlock() != HAL_OK) {
        return ARM_DRIVER_ERROR;
    }

    ARM_FLASH0_STATUS.busy = DRIVER_STATUS_BUSY;

    /* Clear error programming flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    /* Get the 1st page to erase */
    FirstPage = page_number(&ARM_FLASH0_DEV, address);
    /* MBED HAL erases 1 page  / sector at a time */
    /* Get the bank */
    BankNumber = bank_number(&ARM_FLASH0_DEV, address);
    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = BankNumber;
    EraseInitStruct.Page        = FirstPage;
    EraseInitStruct.NbPages     = 1;

    /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        err = ARM_DRIVER_ERROR;
    }

    ARM_FLASH0_STATUS.busy = DRIVER_STATUS_IDLE;

    flash_lock();

    return (err == HAL_OK) ? ARM_DRIVER_OK :ARM_DRIVER_ERROR;
}

static int32_t ARM_Flash_EraseChip(void)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_FLASH_STATUS ARM_Flash_GetStatus(void)
{
    return ARM_FLASH0_STATUS;
}

static ARM_FLASH_INFO * ARM_Flash_GetInfo(void)
{
    return ARM_FLASH0_DEV.data;
}

ARM_DRIVER_FLASH Driver_FLASH0 = {
    ARM_Flash_GetVersion,
    ARM_Flash_GetCapabilities,
    ARM_Flash_Initialize,
    ARM_Flash_Uninitialize,
    ARM_Flash_PowerControl,
    ARM_Flash_ReadData,
    ARM_Flash_ProgramData,
    ARM_Flash_EraseSector,
    ARM_Flash_EraseChip,
    ARM_Flash_GetStatus,
    ARM_Flash_GetInfo
};

static int32_t flash_unlock(void)
{
    /* Allow Access to Flash control registers and user Falsh */
    if (HAL_FLASH_Unlock()) {
        return -1;
    } else {
        return 0;
    }
}

static int32_t flash_lock(void)
{
    /* Disable the Flash option control register access (recommended to protect
    the option Bytes against possible unwanted operations) */
    if (HAL_FLASH_Lock()) {
        return -1;
    } else {
        return 0;
    }
}