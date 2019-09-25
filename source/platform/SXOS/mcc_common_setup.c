// ----------------------------------------------------------------------------
// Copyright 2018-2019 ARM Ltd.
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
#include "mcc_common_config.h"

#include "cos.h"
#include "drv.h"
#include "errorcode.h"
#include "cfw.h"

#include <assert.h>
#include <string.h>
#include <time.h>

static HANDLE arm_task_handle;

#define GPRS_DEATT  0
#define GPRS_ATT    1
#define GPRS_ACT    2

#define DMA_ALIGN_SIZE 16
#define DMA_ALIGN_MASK (DMA_ALIGN_SIZE-1)

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////
static void *network_interface = NULL;
static DRV_UART_T *uart_drv;

// counter of RX data overflows, which is useful for tracing data losses
static volatile int rx_overflows;

// XXX - Todo: need to have configuration for SIM id and APN
static const UINT8 nSim         = 0;
static char* nApn               = "";

static INT32 nCid               = 0;
static UINT32 g_InitialState    = GPRS_DEATT;

static bool first_platform_init = true;

extern struct netif *getGprsNetIf(UINT8 nSim, UINT8 nCid);

/* UART settings - port and baudrate
 * Port depends of AT port defined in target.config
 * Baudrate is default 115200 - might be higher, not tested?
 */
#if (AT_DEFAULT_UART == 2)
#define ARM_UART_ID 1
#elif (AT_DEFAULT_UART == 1)
#define ARM_UART_ID 2
#else
#error "AT_DEFAULT_UART not set"
#endif

#define ARM_UART_BAUDRATE       115200

#define MCC_CONNECTION_TIMEOUT  (10*60) // Seconds
static uint32_t timeout = 0;

// Semaphore used on communication from rx-callback to the getchar().
static COS_SEMA serial_rx_semaphore = COS_SEMA_UNINIT;


// This function is derived from example code provided by Unisoc
static INT32 startGprsLink1(UINT8 *apn, UINT8 nSIMID)
{
    UINT8 nAttstate = 0xFF;
    UINT32 nRet = 0x00;
    UINT8 nCID = 0x00, nState = 0x00;
    CFW_NW_STATUS_INFO sStatus;
    UINT8 nUTI = 0x00;

    nRet = CFW_GetGprsAttState(&nAttstate, nSIMID);
    printf("startGprsLink GprsAttState nRet=0x%x,nAttstate=%d\n", nRet, nAttstate);
    if (nAttstate != CFW_GPRS_ATTACHED) {
        g_InitialState = GPRS_DEATT;
        CFW_GetFreeUTI(0, &nUTI);
        nRet = CFW_GprsAtt(CFW_GPRS_ATTACHED, nUTI, nSIMID);
        printf("startGprsLink nUTI=%d,nRet=0x%x\n", nUTI, nRet);
        while (nRet == ERR_SUCCESS && nAttstate != CFW_GPRS_ATTACHED) {
            if (timeout > MCC_CONNECTION_TIMEOUT) {
                printf("Timeout!\n");
                return -1;
            }

            printf("startGprsLink Waiting ATT ... nAttstate=%d,nRet=0x%x\n", nAttstate, nRet);
            COS_Sleep(2000);
            timeout += 2;

            // There is no worth continue registration if ATT status is UNKNOWN
            if (CFW_GprsGetstatus(&sStatus, nSIMID) == ERR_SUCCESS &&
                sStatus.nStatus == CFW_NW_STATUS_UNKNOW) {
                printf("Registration failed! (NW status Unknown)\n");
                return -1;
            }

            nRet = CFW_GetGprsAttState(&nAttstate, nSIMID);
        }
    }

    if (nRet == ERR_SUCCESS && nAttstate == CFW_GPRS_ATTACHED) {
        g_InitialState = GPRS_ATT;
    } else {
        return -1;
    }

    nRet = CFW_GprsGetstatus(&sStatus, nSIMID);
    while (nRet == ERR_SUCCESS &&
           sStatus.nStatus != CFW_NW_STATUS_REGISTERED_HOME &&
           sStatus.nStatus != CFW_NW_STATUS_REGISTERED_ROAMING) {

        if (timeout > MCC_CONNECTION_TIMEOUT) {
            printf("Timeout!\n");
            return -1;
        }

        printf("startGprsLink Waiting Service ... sStatus.nStatus=%d,nRet=0x%x\n", sStatus.nStatus, nRet);
        COS_Sleep(1000);
        timeout += 1;
        nRet = CFW_GprsGetstatus(&sStatus, nSIMID);
    }
    if (nRet != ERR_SUCCESS) {
        return -1;
    }

    nRet = CFW_GetFreeCID(&nCID, nSIMID);
    printf("startGprsLink CFW_GetFreeCID nCID=%d,nRet=0x%x\n", nCID, nRet);

    CFW_GPRS_QOS qos;
    qos.nDelay = 4;
    qos.nMean = 16;
    qos.nPeak = 4;
    qos.nPrecedence = 3;
    qos.nReliability = 3;
    nRet = CFW_GprsSetReqQos(nCID, &qos, nSIMID);
    printf("startGprsLink CFW_GprsSetReqQos ret nUTI=%d,nRet=0x%x\n", nUTI, nRet);

    CFW_GPRS_PDPCONT_INFO pdp_cont;
    pdp_cont.nApnSize = strlen(apn);
    pdp_cont.pApn = apn;

    pdp_cont.nPdpAddrSize = 0;
    pdp_cont.pPdpAddr = NULL;
    pdp_cont.nDComp = 0;
    pdp_cont.nHComp = 0;
    pdp_cont.nPdpType = CFW_GPRS_PDP_TYPE_IP;

    pdp_cont.nApnUserSize = 0;
    pdp_cont.pApnUser = NULL;
    pdp_cont.nApnPwdSize = 0;
    pdp_cont.pApnPwd = NULL;

    nRet = CFW_GprsSetPdpCxt(nCID, &pdp_cont, nSIMID);
    printf("startGprsLink CFW_GprsSetPdpCxt nCID=%d,nRet=0x%x\n", nCID, nRet);

    nRet = CFW_GetGprsActState(nCID, &nState, nSIMID);
    if (nState != CFW_GPRS_ACTIVED) {
        nRet = CFW_GprsAct(CFW_GPRS_ACTIVED, nCID, nUTI, nSIMID);
        printf("startGprsLink CFW_GprsAct nCID=%d,nRet=0x%x\n", nCID, nRet);
        while (nRet == ERR_SUCCESS && nState != CFW_GPRS_ACTIVED) {
            if (timeout > MCC_CONNECTION_TIMEOUT) {
                printf("Timeout!\n");
                return -1;
            }
            printf("startGprsLink Waiting Act ... nState=%d,nRet=0x%x\n", nState, nRet);
            COS_Sleep(1000);
            timeout += 1;
            nRet = CFW_GetGprsActState(nCID, &nState, nSIMID);
        }
    }

    if (nRet == ERR_SUCCESS && nState == CFW_GPRS_ACTIVED) {
        g_InitialState = GPRS_ACT;
    }

    if (nRet != ERR_SUCCESS) {
        return -1;
    }

    return nCID;
}

// This function is derived from example code provided by Unisoc
static void restoreGprsLink(UINT8 nSIMID, UINT8 nCID)
{
    UINT8 nAttstate = 0xFF;
    UINT32 nRet = 0x00;
    UINT8 nState = 0x00;
    CFW_NW_STATUS_INFO sStatus;
    UINT8 nUTI = 0x00;

    if (g_InitialState == GPRS_ATT) {
        nRet = CFW_GetGprsAttState(&nAttstate, nSIMID);
        printf("resoreGprsLink GprsAttState nRet=0x%x,nAttstate=%d\n", nRet, nAttstate);
        if (nAttstate != CFW_GPRS_DETACHED) {
            CFW_GetFreeUTI(0, &nUTI);
            nRet = CFW_GprsAtt(CFW_GPRS_DETACHED, nUTI, nSIMID);
            printf("resoreGprsLink nUTI=%d,nRet=0x%x\n", nUTI, nRet);
            while (nRet == ERR_SUCCESS && nAttstate != CFW_GPRS_DETACHED) {
                printf("resoreGprsLink Waiting ATT ... nAttstate=%d,nRet=0x%x\n", nAttstate, nRet);
                COS_Sleep(500);
                nRet = CFW_GetGprsAttState(&nAttstate, nSIMID);
            }
        }
    } else if (g_InitialState == GPRS_ACT) {
        nRet = CFW_GetGprsActState(nCID, &nState, nSIMID);
        if (nState != CFW_GPRS_DEACTIVED) {
            CFW_GetFreeUTI(0, &nUTI);
            nRet = CFW_GprsAct(CFW_GPRS_DEACTIVED, nCID, nUTI, nSIMID);
            printf("resoreGprsLink CFW_GprsDEAct nCID=%d,nRet=0x%x,uti=%d\n", nCID, nRet, nUTI);
            while (nRet == ERR_SUCCESS && nState != CFW_GPRS_DEACTIVED) {
                printf("resoreGprsLink Waiting DEAct ... nState=%d,nRet=0x%x\n", nState, nRet);
                COS_Sleep(500);
                nRet = CFW_GetGprsActState(nCID, &nState, nSIMID);
            }
        }
    }

    g_InitialState = GPRS_DEATT;
}

////////////////////////////////
// SETUP_COMMON.H IMPLEMENTATION
////////////////////////////////

int mcc_platform_init_connection(void) {
    return mcc_platform_interface_connect();
}

void* mcc_platform_get_network_interface(void) {
    return mcc_platform_interface_get();
}

int mcc_platform_close_connection(void) {
    return mcc_platform_interface_close();
}

int mcc_platform_interface_connect(void)
{
    timeout = 0;

    if (g_InitialState != GPRS_ACT) {
        printf("Waiting for network connection...\n");

        nCid = startGprsLink1(nApn, nSim);
        if (nCid < 0) {
            printf("Unable to get context id!\n");
            return -1;
        }

        while (timeout < MCC_CONNECTION_TIMEOUT) {
            network_interface = getGprsNetIf(nSim, (UINT8)nCid);
            if (network_interface != NULL) {
                printf("Connected!\n");
                return 0;
            } else {
                printf(".");
                COS_Sleep(2000);
                timeout += 2;
            }
        }

        printf("Unable to connect!\n");
        return -1;
    } else {
        return 0;
    }
}

void mcc_platform_interface_init(void) {}

int mcc_platform_interface_close(void)
{
    network_interface = NULL;
    restoreGprsLink(nSim, (UINT8)nCid);
    return 0;
}

void* mcc_platform_interface_get(void)
{
    return network_interface;
}

int sxos_uart_write(const char *data, int len)
{
    int offset = 0;

    while (offset < len) {

        int written = DRV_UartWrite(uart_drv, data + offset, len - offset, 500);

        offset += written;
    }

    return offset;
}

static void arm_UartCallback(DRV_UART_T *drv, void *param, DRV_UART_EVENT_T evt)
{
    if (evt &= DRV_UART_RX_ARRIVED) {
        // ping the receival end
        COS_SemaRelease(&serial_rx_semaphore);
    }
    if (evt &= DRV_UART_RX_OVERFLOW) {
        rx_overflows++;
    }
}

static int sxos_uart_init(void)
{
    static bool uart_initted = false;

    // allow calling again, useful for re-using the code from multiple main()'s,
    // eg. testapp (which calls mcc_platform_init()) can call PAL's test code which
    // also performs its initializations.
    if (uart_initted) {
        return 0;
    }

    COS_SemaInit(&serial_rx_semaphore, 0);

    uart_drv =  COS_Malloc((UINT32) DRV_UartStructSize(), COS_MMI_HEAP);
    static DRV_UART_CFG_T drvcfg = {0};
    memset(uart_drv, 0, DRV_UartStructSize());
    COS_PRINTFI("[ARM], sxos_uart_init");
    /* Init UART port */
    /* Fill DRV_UART_CFG_T */
    drvcfg.baud = ARM_UART_BAUDRATE;
    drvcfg.dataBits = DRV_UART_DATA_BITS_8;
    drvcfg.stopBits = DRV_UART_STOP_BITS_1;
    drvcfg.parity = DRV_UART_NO_PARITY;
    drvcfg.autoFlowCtrlLevel = DRV_UART_DISABLE_AUTO_FLOW_CTRL;
    drvcfg.rxDmaBuf = (uint8_t *)COS_Malloc(256 + DMA_ALIGN_MASK, COS_MMI_HEAP);
    /* Align address. rxDmaBuf address must be DMA_ALIGN_MASK aligned (currently, 16 bytes) */
    drvcfg.rxDmaBuf = (uint8_t *)((uint32_t)(drvcfg.rxDmaBuf + DMA_ALIGN_MASK) & (~DMA_ALIGN_MASK));
    drvcfg.rxBuf = (uint8_t *)COS_Malloc((4 * 1024), COS_MMI_HEAP);
    drvcfg.txBuf = (uint8_t *)COS_Malloc((4 * 1024), COS_MMI_HEAP);
    drvcfg.rxDmaSize = 256;
    drvcfg.rxBufSize = (4 * 1024);
    drvcfg.txBufSize = (4 * 1024);

    // My board works also when RX polling is disabled, but (some?) HW is said to
    // have a bug where RX interrupt is not necessarily delivered so polling is needed.
    // So let's have the 100 polls per second as it should be enough for 115,2Kbps link.
    drvcfg.rxPollPeriod = 10;

    drvcfg.evtMask = DRV_UART_RX_ARRIVED | DRV_UART_RX_OVERFLOW | DRV_UART_WAKE_UP;
    drvcfg.callback = arm_UartCallback;
    drvcfg.callbackParam = uart_drv;
    if (false == DRV_UartInit(uart_drv, ARM_UART_ID, &drvcfg)) {
        COS_PRINTFE("[ARM], UART init failed");
        return -1;
    }
    /* Open UART port */
    if (false == DRV_UartOpen(uart_drv)) {
        COS_PRINTFE("[ARM], UART open failed");
        return -1;
    }

    uart_initted = true;

    return 0;
}

// In order for tests to pass for all partition configurations we need to simulate the case of multiple
// partitions using a single path. We do this by creating one or two different sub-paths, depending on
// the configuration.
int mcc_platform_storage_init(void)
{
    return 0;
}

int mcc_platform_init(void)
{
    int err = 0;

    // TODO: This requires a better solution
    // SX OS start up requires a small delay before platform
    // functions can be called.
    if (first_platform_init) {
        COS_Sleep(2000);
    }

    arm_task_handle = COS_GetCurrentTaskHandle();

    first_platform_init = false;

    if (0 != sxos_uart_init()) {
        return -1;
    }

    return err;
}

int mcc_platform_reformat_storage(void)
{
// cleanup folders
// to do:
// PAL_FS_MOUNT_POINT_PRIMARY
// PAL_FS_MOUNT_POINT_SECONDARY
    printf("mcc_platform_reformat_storage is not supported\n");
    return 0;
}

void mcc_platform_do_wait(int timeout_ms)
{
    usleep(timeout_ms * 1000);
}

int mcc_platform_run_program(main_t mainFunc)
{
    mainFunc();
    return 1;
}

void mcc_platform_sw_build_info(void)
{
    printf("Application ready. Build at: " __DATE__ " " __TIME__ "\n");
}

int getchar(void)
{
    int c = 0;
    while (DRV_UartRead(uart_drv, (uint8_t*)&c, 1) == 0) {
        // sleep until the arm_UartCallback() wakes us
        COS_SemaTake(&serial_rx_semaphore);
    }

    // XXX: enable next line to make RX overflows visible. I can't actually make the
    // overflow happen on my machine, so perhaps it could even be enabled by default?!
#if 0
    if (rx_overflows > 0) {
        assert(false);
    }
#endif

    return c;
}
