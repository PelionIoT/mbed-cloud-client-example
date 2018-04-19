/*
 * Copyright (c) 2015-2018 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include "common_setup.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "ff.h"
#include "diskio.h"
#include "sdhc_config.h"
#include "pin_mux.h"
#include "pal.h"
#include "board.h"
#include "fsl_uart.h"
#include "clock_config.h"
#include "common_serial.h"
#include "FreeRTOS.h"
#include "task.h"

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

/* UART instance and clock */
#define UART_CLKSRC UART0_CLK_SRC
#define UART_CLK_FREQ 	BOARD_DEBUG_UART_CLK_FREQ
#define UART_IRQn UART0_RX_TX_IRQn
#define UART_IRQHandler UART0_RX_TX_IRQHandler

/*! @brief Ring buffer size (Unit: Byte). */
#define RING_BUFFER_SIZE 768

/*
  Ring buffer for data input and output,input data are saved to ring buffer in IRQ handler. The main function polls the ring buffer status,
  if there are new data, then send them out.
*/
uint8_t uartRingBuffer[RING_BUFFER_SIZE];
volatile uint16_t txIndex;    /* Index of the data to send out. */
volatile uint16_t rxIndex;    /* Index of the memory to save new arrived data. */
static main_t mainFuncLocal;  /* Pointer for main function. */

/*! @brief   Debug console baud rate */
#define APP_DEBUG_UART_BAUDRATE 115200

/*! @brief System clock. */
#define APP_DEBUG_UART_CLKSRC_NAME kCLOCK_CoreSysClk

/*! @brief File System initialization flag  */
static int fileSystemInit = 0;

/*! @brief LWIP network interface structure */
static struct netif fsl_netif0;

/*! @brief LWIP IP structure */
static ip_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;

/*! @brief is DHCP ready flag  */
volatile int dhcp_done = 0;

/*! @brief FileSystem Mounting point -> "2:/" */
static const TCHAR cDriverNumberBuffer[] = {SDDISK + '0', ':', '/'};

/*! @brief Preallocated Work area (file system object) for logical drive, should NOT be free or lost*/
static FATFS fileSystem[2];

/*! @brief Network interface status callback function */
static void netif_status(struct netif *n)
{
    if (n->flags & NETIF_FLAG_UP) {
        printf("Interface is up : %d\r\n", n->dhcp->state);
        printf("IP %s\r\n", ipaddr_ntoa(&n->ip_addr));
        printf("NM %s\r\n", ipaddr_ntoa(&n->netmask));
        printf("GW %s\r\n", ipaddr_ntoa(&n->gw));
        dhcp_done = 1;
    } else {
        printf("Interface Down.\n");
    }
}

/*!
 * @brief fileSystemMountDrive - mount the SD card to  "cDriverNumberBuffer"
 * @param void
 * @return void
 */
static void fileSystemMountDrive();

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

/*!
 * @brief Network interface initialization
 * @param *arg - Not in use
 * @return void
 */
static void network_Init(void *arg);

/*! \brief blockDelay - Blocks the task and count the number of ticks given
 * @param void
 * \return TRUE - on success
 */
static void blockDelay(uint32_t Ticks);

static void runMainFunction(void)
{
    // wait until DHCP request is completed
    while(dhcp_done == 0) {
        mcc_platform_do_wait(100);
    }

    mainFuncLocal();
}

int mcc_platform_run_program(main_t mainFunc)
{
    mcc_platform_init();

    mainFuncLocal = mainFunc;

    xTaskCreate((TaskFunction_t)runMainFunction, "_main_", (uint16_t)1024*18, NULL, tskIDLE_PRIORITY + 1, NULL);

    //Start OS
    vTaskStartScheduler();

    vTaskDelete( NULL );

    return 1;
}

int mcc_platform_initFreeRTOSPlatform()
{
    //Init Board
    boardInit();

    //Init FileSystem
    xTaskCreate((TaskFunction_t)fileSystemMountDrive, "FileSystemInit", (uint16_t)1024*4, NULL, tskIDLE_PRIORITY + 3, NULL);

    //Init DHCP thread
    sys_thread_new("network_Init", network_Init, NULL, 1024, tskIDLE_PRIORITY + 2);

    return 1;
}

static void network_Init(void *arg)
{
    printf("Starting HTTP thread!\r\n");
    err_t err = 0;
    (void) (arg);

    tcpip_init(NULL, NULL);
    printf("TCP/IP initialized.\r\n");
    IP4_ADDR(&fsl_netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&fsl_netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&fsl_netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, NULL, ethernetif_init, tcpip_input);
    netif_set_default(&fsl_netif0);
    netif_set_status_callback(&fsl_netif0, netif_status);

    /* obtain the IP address, default gateway and subnet mask by using DHCP*/
    err = dhcp_start(&fsl_netif0);
    printf("Started DCHP request (%s)\r\n", lwip_strerr(err));
    for(int i=0; i < 40 && fsl_netif0.dhcp->state != DHCP_BOUND; i++) {
        printf("Current DHCP State : (%d)\r\n", fsl_netif0.dhcp->state);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

    printf("DHCP state, activating interface (%d)\r\n", fsl_netif0.dhcp->state);
    if (fsl_netif0.dhcp->state != DHCP_BOUND) {
        printf("DHCP state, TIMEOUT (%d)\r\n", fsl_netif0.dhcp->state);
    }

    netif_set_up(&fsl_netif0);

    vTaskDelete( NULL );
}

static void blockDelay(uint32_t Ticks)
{
    uint32_t tickCounts = 0;
    for(tickCounts = 0; tickCounts < Ticks; tickCounts++){}
}

// Currently we support only one interface
void* mcc_platform_GetNetWorkInterfaceContext()
{
    return (void *)&fsl_netif0;
}

static void fileSystemMountDrive(void)
{
    char folder1[PAL_MAX_FILE_AND_FOLDER_LENGTH] = {0};
    char folder2[PAL_MAX_FILE_AND_FOLDER_LENGTH] = {0};
    printf("Creating FileSystem SetUp thread!\r\n");
    FRESULT fatResult;
    int count = 0;
    palStatus_t status = PAL_SUCCESS;

    if (fileSystemInit == false)
    {
        //Detected SD card inserted
        while (!(GPIO_ReadPinInput(BOARD_SDHC_CD_GPIO_BASE, BOARD_SDHC_CD_GPIO_PIN)))
        {
            blockDelay(1000U);
            if (count++ > MAX_SD_READ_RETRIES)
            {
                break;
            }
        }

        if(count < MAX_SD_READ_RETRIES)
        {
            /* Delay some time to make card stable. */
            blockDelay(10000000U);
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
                printf("Exit FileSystem SetUp thread!\r\n");
            }
        }
    }
    vTaskDelete( NULL );
}

static void APP_InitPlatformTRNG()
{
    CLOCK_EnableClock(kCLOCK_Rnga0);
    CLOCK_DisableClock(kCLOCK_Rnga0);
    CLOCK_EnableClock(kCLOCK_Rnga0);
}

void UART_IRQHandler(void)
{
    uint8_t data;
    if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(UART0)) /* If new data arrived. */
    {
        data = UART_ReadByte(UART0);

        /* If ring buffer is not full, add data to ring buffer. */
        if (((rxIndex + 1) % RING_BUFFER_SIZE) != txIndex)
        {
            uartRingBuffer[rxIndex] = data;
            rxIndex++;
            rxIndex %= RING_BUFFER_SIZE;
        }
    }
}

static void UART_Init_EnableInterrupts()
{
    uart_config_t config;

    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx = true;
    config.enableRx = true;

    UART_Init(UART0, &config, UART_CLK_FREQ);

    /* Enable RX interrupt. */
    UART_EnableInterrupts(UART0, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable);
    EnableIRQ(UART_IRQn);
}

static void boardInit()
{
    MPU_Type *base = MPU;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    UART_Init_EnableInterrupts();
    APP_InitPlatformTRNG();
    /* Disable MPU. */
    base->CESR &= ~MPU_CESR_VLD_MASK;
}

void SerialBytesTest(uartParser Func)
{
    while(1)
    {
        /* Send data only when UART TX register is empty and ring buffer has data to send out. */
        if ((kUART_TxDataRegEmptyFlag & UART_GetStatusFlags(UART0)) && (rxIndex != txIndex))
        {
            Func(uartRingBuffer[txIndex]);
            txIndex++;
            txIndex %= RING_BUFFER_SIZE;
        }
        else
        {
            taskYIELD();
        }
    }
}
