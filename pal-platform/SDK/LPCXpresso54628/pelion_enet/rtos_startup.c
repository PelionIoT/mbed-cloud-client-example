
///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

/* SDK Included Files */
#include "board.h"
#include "fsl_debug_console.h"
#include "ksdk_mbedtls.h"
#include "pin_mux.h"

#include "FreeRTOS.h"

/* lwIP Includes */
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"
#include "lwip/netifapi.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x25 \
    }

/* Memory not usable by ENET DMA. */
#define NON_DMA_MEMORY_ARRAY \
    {                        \
        {0x0U, 0x80000U},    \
        {                    \
            0x0U, 0x0U       \
        }                    \
    }

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* System clock name. */
#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk

#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

#define INIT_SUCCESS 0
#define INIT_FAIL 1

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern int mbed_cloud_application_entrypoint(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
struct netif network_interface;
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
mem_range_t non_dma_memory[] = NON_DMA_MEMORY_ARRAY;
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */

/*******************************************************************************
 * Code
 ******************************************************************************/

void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

int initNetwork(void)
{
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyAddress = EXAMPLE_PHY_ADDRESS,
        .clockName  = EXAMPLE_CLOCK_NAME,
        .macAddress = configMAC_ADDR,
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
        .non_dma_memory = non_dma_memory,
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    };

    IP4_ADDR(&netif_ipaddr, 0, 0, 0, 0);
    IP4_ADDR(&netif_netmask, 0, 0, 0, 0);
    IP4_ADDR(&netif_gw, 0, 0, 0, 0);

    tcpip_init(NULL, NULL);

    netifapi_netif_add(&network_interface, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&network_interface);
    netifapi_netif_set_up(&network_interface);

    PRINTF(("Getting IP address from DHCP ...\r\n"));
    netifapi_dhcp_start(&network_interface);

    struct dhcp *dhcp;
    dhcp = (struct dhcp *)netif_get_client_data(&network_interface, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

    while (dhcp->state != DHCP_STATE_BOUND)
    {
        vTaskDelay(1000);
    }

    if (dhcp->state == DHCP_STATE_BOUND)
    {
        PRINTF("IPv4 Address: %u.%u.%u.%u\r\n", ((u8_t *)&network_interface.ip_addr.u_addr.ip4)[0],
                      ((u8_t *)&network_interface.ip_addr.u_addr.ip4)[1],
                      ((u8_t *)&network_interface.ip_addr.u_addr.ip4)[2],
                      ((u8_t *)&network_interface.ip_addr.u_addr.ip4)[3]);
    }
    PRINTF(("DHCP OK\r\n"));

    return INIT_SUCCESS;
}

void mainTask(void* pArgument)
{
    PRINTF("mainTask\r\n");

    mbed_cloud_application_entrypoint();
}

void vApplicationDaemonTaskStartupHook(void)
{
    if (initNetwork() != 0)
    {
        PRINTF(("Network init failed, stopping demo.\r\n"));
        vTaskDelete(NULL);
    }
    else
    {
        PRINTF(("Startup complete\r\n"));

        xTaskCreate(mainTask, "mainTask", 1024 * 4, NULL, tskIDLE_PRIORITY + 1, NULL);
    }
}

int main(void)
{
    CLOCK_EnableClock(kCLOCK_InputMux);

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL220M();
    BOARD_InitDebugConsole();
    CRYPTO_InitHardware();

    vTaskStartScheduler();
    for (;;);
}

void vApplicationMallocFailedHook(void)
{
    PRINTF("ERROR: Malloc failed to allocate memory\r\n");
    taskDISABLE_INTERRUPTS();

    /* Loop forever */
    for( ; ; );
}

void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   char* pcTaskName )
{
    PRINTF("ERROR: stack overflow with task %s\r\n", pcTaskName);
    portDISABLE_INTERRUPTS();

    /* Unused Parameters */
    ( void ) xTask;

    /* Loop forever */
    for( ; ; );
}
