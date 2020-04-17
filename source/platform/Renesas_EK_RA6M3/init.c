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

#include "mcc_common_setup.h"
#include "mcc_common_serial.h"

#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#ifndef MBED_CONF_MBED_CLOUD_CLIENT_EXTERNAL_SST_SUPPORT
#include "ff.h"
#include "diskio.h"
#include "sdhc_config.h"
#endif // #ifndef MBED_CONF_MBED_CLOUD_CLIENT_EXTERNAL_SST_SUPPORT
#include "pal.h"

// This is the BPS starter header for Renesas RA system
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_RTT.h"

#include <stdio.h>

#define MAX_SD_READ_RETRIES	5
#define configIP_ADDR0 0
#define configIP_ADDR1 0
#define configIP_ADDR2 0
#define configIP_ADDR3 0

/* Netmask configuration. */
#define configNET_MASK0 0
#define configNET_MASK1 0
#define configNET_MASK2 0
#define configNET_MASK3 0

/* Default gateway address configuration */
#define configGW_ADDR0 0
#define configGW_ADDR1 0
#define configGW_ADDR2 0
#define configGW_ADDR3 0

/*! @brief Ring buffer size (Unit: Byte). */
#define RING_BUFFER_SIZE 768

err_t ethernetif_init(struct netif *netif);

/*
  Ring buffer for data input and output,input data are saved to ring buffer in IRQ handler. The main function polls the ring buffer status,
  if there are new data, then send them out.
*/
uint8_t uartRingBuffer[RING_BUFFER_SIZE];
volatile uint16_t txIndex;    /* Index of the data to send out. */
volatile uint16_t rxIndex;    /* Index of the memory to save new arrived data. */

/*! @brief   Debug console baud rate */
#define APP_DEBUG_UART_BAUDRATE 115200

/*! @brief System clock. */
#define APP_DEBUG_UART_CLKSRC_NAME kCLOCK_CoreSysClk

/*! @brief File System initialization flag, true if the filesystem has been initialized successfully */
static bool fileSystemInit = false;

/*! @brief LWIP network interface structure */
static struct netif netif0;

extern const void* __Vectors;

/*! @brief LWIP IP structure */
static ip_addr_t netif0_ipaddr, netif0_netmask, netif0_gw;

/*! @brief is DHCP ready flag  */
volatile int dhcp_done = 0;

#ifndef MBED_CONF_MBED_CLOUD_CLIENT_EXTERNAL_SST_SUPPORT
/*! @brief Preallocated Work area (file system object) for logical drive, should NOT be free or lost*/
static FATFS fileSystem[2];
#endif

// If the FAT is configured to support _MULTI_PARTITION, we need to provide it a volume-to-partition
// conversion table. The amount of array items need to match with the PAL configuration.
#if _MULTI_PARTITION
// use auto-detect feature of FAT on partition numbers, hence the 0 in each partition definition
PARTITION VolToPart[] = {
#if (PAL_NUMBER_OF_PARTITIONS > 0)
    {SDDISK, 0}, /* 0: */
#endif
#if (PAL_NUMBER_OF_PARTITIONS > 1)
    {SDDISK, 0}  /* 1: */
#endif
};
#endif


/*! @brief Network interface status callback function */
static void netif_status(struct netif *n)
{
    // TODO: to be investigated, if the context of this callback is really one where
    // the printf() is allowed, ie. not from a interrupt.
    if (n->flags & NETIF_FLAG_UP) {
        struct dhcp *dhcp = netif_dhcp_data(n);
        printf("Interface is up : %d\r\n", dhcp->state);
        printf("IP %s\r\n", ipaddr_ntoa(&n->ip_addr));
        printf("NM %s\r\n", ipaddr_ntoa(&n->netmask));
        printf("GW %s\r\n", ipaddr_ntoa(&n->gw));
        dhcp_done = 1;
    } else {
        printf("Interface Down.\n");
    }
}

/*!
 * @brief APP_InitPlatformTRNG
 * @param void
 * @return void
 */
static void APP_InitPlatformTRNG();

/*!
 * @brief boardInit - initialized Board H/W
 * @param void
 * @return void
 */
static void boardInit();

int mcc_platform_run_program(main_t mainFunc)
{
    SEGGER_RTT_SetFlagsUpBuffer(0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    SEGGER_RTT_SetFlagsDownBuffer(0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

    // Note: the boardInit is ran from mcc_platform_init(), which is typically among the first
    // calls in the mainFunc(). But as the task creation and/or scheduling might actually need
    // some board functionality (interrupts/MPU setup), we better still initialize the platform here.
    // This makes the platforms behave a bit differently, which is not good. But the stability
    // is still more important than symmetry.
    if (mcc_platform_init() == 0) {

        // XXX: use the defines from some config file, not from a hardcoded value here
        BaseType_t status = xTaskCreate((TaskFunction_t)mainFunc, "_main_", 10*1024/sizeof(int), NULL, tskIDLE_PRIORITY + 1, NULL);

        if (status == pdPASS) {

            // Start OS, this call will never return unless there is too little RAM to start idle thread
            vTaskStartScheduler();

            return 1;
        }
    }
    return 0;
}

int mcc_platform_initFreeRTOSPlatform()
{
    //Init Board
    boardInit();

    return 1;
}

int networkInit(void)
{
    err_t err = 0;

    printf("Initializing network.\r\n");
    tcpip_init(NULL, NULL);
    printf("TCP/IP initialized.\r\n");
    IP4_ADDR(&netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    netif_add(&netif0, &netif0_ipaddr, &netif0_netmask, &netif0_gw, NULL, ethernetif_init, tcpip_input);
    netif_set_default(&netif0);
    netif_set_status_callback(&netif0, netif_status);

    /* obtain the IP address, default gateway and subnet mask by using DHCP*/
    err = dhcp_start(&netif0);
    printf("Started DCHP request (%s)\r\n", lwip_strerr(err));
    struct dhcp *dhcp = netif_dhcp_data(&netif0);
    for(int i=0; i < 40 && dhcp->state != DHCP_STATE_BOUND; i++) {
        printf("Current DHCP State : (%d)\r\n", dhcp->state);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf("DHCP state, activating interface (%d)\r\n", dhcp->state);

    if (dhcp->state != DHCP_STATE_BOUND) {
        printf("DHCP state, TIMEOUT (%d)\r\n", dhcp->state);
        err = -1;
    } else {
        // complete setup if success
        netif_set_up(&netif0);
        err = 0;
    }

    return err;
}

// Currently we support only one interface
void* mcc_platform_GetNetWorkInterfaceContext()
{
    return (void *)&netif0;
}

int fileSystemMountDrive(void)
{
#ifndef MBED_CONF_MBED_CLOUD_CLIENT_EXTERNAL_SST_SUPPORT
    char folder1[PAL_MAX_FILE_AND_FOLDER_LENGTH] = {0};
    char folder2[PAL_MAX_FILE_AND_FOLDER_LENGTH] = {0};
    printf("fileSystemMountDrive, partitions: %d\n", PAL_NUMBER_OF_PARTITIONS);
    FRESULT fatResult;
    int count = 0;
    palStatus_t status = PAL_SUCCESS;

    if (fileSystemInit == false)
    {
        //Detected SD card inserted
        while (!(GPIO_ReadPinInput(BOARD_SDHC_CD_GPIO_BASE, BOARD_SDHC_CD_GPIO_PIN)))
        {
            vTaskDelay(pdMS_TO_TICKS(500));
            if (count++ > MAX_SD_READ_RETRIES)
            {
                break;
            }
        }

        if (count < MAX_SD_READ_RETRIES)
        {
            // Delay some time to make card stable.
            vTaskDelay(pdMS_TO_TICKS(1000));

#if 0
            // this code does not belong here, it needs a move to mcc_platform_reformat_storage(),
            // but even then, there is no proper setup mechanism for the partition sizes.
            // In any case, the code needs to be inside a ifdef _MULTI_PARTITION, as f_fdisk() function
            // is also ifdeffed out on ff.c
#ifdef PAL_EXAMPLE_GENERATE_PARTITION
#if (PAL_NUMBER_OF_PARTITIONS == 1)
            DWORD plist[] = {100,0,0,0};
#elif	(PAL_NUMBER_OF_PARTITIONS == 2) //else of (PAL_NUMBER_OF_PARTITIONS == 1)
            DWORD plist[] = {50,50,0,0};
#endif //(PAL_NUMBER_OF_PARTITIONS == 1)
            BYTE work[_MAX_SS];

            fatResult= f_fdisk(SDDISK,plist, work);
            printf("f_fdisk fatResult=%d\r\n",fatResult);
            if (FR_OK != fatResult)
            {
                printf("Failed to create partitions in disk\r\n");
            }
#endif //PAL_EXAMPLE_GENERATE_PARTITION
#endif

            status = pal_fsGetMountPoint(PAL_FS_PARTITION_PRIMARY,PAL_MAX_FILE_AND_FOLDER_LENGTH,folder1);
            if (PAL_SUCCESS == status)
            {
                fatResult = f_mount(&fileSystem[0], folder1, 1U);
                if (FR_OK != fatResult)
                {
                    printf("Failed to mount partition %s in disk\r\n",folder1);
                }
            }
            else
            {
                printf("Failed to get mount point for primary partition\r\n");
            }

            status = pal_fsGetMountPoint(PAL_FS_PARTITION_SECONDARY,PAL_MAX_FILE_AND_FOLDER_LENGTH,folder2);
            if (PAL_SUCCESS == status)
            {
                //if there is a different root folder for partition 1 and 2, mount the 2nd partition
                if (strncmp(folder1,folder2,PAL_MAX_FILE_AND_FOLDER_LENGTH))
                {
                    fatResult = f_mount(&fileSystem[1], folder2, 1U);
                    if (FR_OK != fatResult)
                    {
                        printf("Failed to mount partition %s in disk\r\n",folder2);
                    }
                }
            }
            else
            {
                PRINTF("Failed to get mount point for secondary partition\r\n");
            }

            if (fatResult == FR_OK)
            {
                fileSystemInit = true;
            }
        }
    }

    if (fileSystemInit) {
        printf("FileSystem setup successful\n");
        status = 0;
    } else {
        printf("FileSystem setup failed, status: %d, fat: %d\n", (int)status, (int)fatResult);
        status = -1;
    }
    return status;
#else
    return 0;
#endif // #ifndef MBED_CONF_MBED_CLOUD_CLIENT_EXTERNAL_SST_SUPPORT
}

static void APP_InitPlatformTRNG()
{
    // TRNG is not yet implemented in RA6M3 application.
}

static void boardInit()
{
    /* Temporary workaround. VTOR should be set by bootloader. */
    SCB->VTOR = &__Vectors;
    printf("Setting VTOR: %p\r\n", SCB->VTOR);

    APP_InitPlatformTRNG();

    // No need to initialize ioport driver or handle RA6M3's board init.
    // It will be handled by SystemInit().
    //R_IOPORT_Open(&g_ioport_ctrl, &g_bsp_pin_cfg);
}

/**
 * Redirect libc printf to Segger RTT printf.
 */
int _write(int fd, const void *buf, size_t count)
{
    char* out = (char*) buf;
    // index other than 0 will rather complicate Viewer reading.
    // We tried using 'fd' but value 1 didn't show up on RTT viewer.
    // Just stick to 0.
    return (int)SEGGER_RTT_Write(0, out, count);
}
