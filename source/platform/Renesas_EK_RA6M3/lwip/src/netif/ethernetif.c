/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

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

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"

// Headers for Renesas RA Ethernet API
#include "r_ether_phy.h"
#include "r_ether_api.h"
#include "r_ether.h"
// The ether control and config are in common_data.h.
#include "common_data.h"
// Additional headers to enable Ethernet RX channel
#include "FreeRTOS.h"
#include "task.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

#define ETHER_EDMAC_INTERRUPT_FACTOR_RECEPTION    (0x01070000)
// #define ETHER_DEBUG
#ifdef ETHER_DEBUG
#define ETHER_LOG_DEBUG configPRINTF
#define ETHER_ASSERT    configASSERT
#else
#define ETHER_LOG_DEBUG(...)
#define ETHER_ASSERT(...)
#endif

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  ether_ctrl_t *ra_ether0_ctrl;
  const ether_cfg_t *ra_ether0_cfg;
};

static TaskHandle_t xRxHanderTaskHandle = NULL;

/* Forward declarations. */
static void  ethernetif_input(struct netif *netif);
err_t ethernetif_init(struct netif *netif);

static void prvRXHandlerTask (void * pvParameters) {
    struct netif *netif = (struct netif *)pvParameters;

    for ( ; ; )
    {
        /* Wait for the Ethernet MAC interrupt to indicate that another packet
         * has been received.  */
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        ethernetif_input(netif);
    }
}

/** The ISR function for Ethernet RX. */
void vEtherISRCallback (ether_callback_args_t * p_args) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* If EDMAC FR (Frame Receive Event) or FDE (Receive Descriptor Empty Event)
     * interrupt occurs, wake up xRxHanderTask. */
    if (p_args->status_eesr & ETHER_EDMAC_INTERRUPT_FACTOR_RECEPTION)
    {
        if (xRxHanderTaskHandle != NULL)
        {
            vTaskNotifyGiveFromISR(xRxHanderTaskHandle, &xHigherPriorityTaskWoken);
        }

        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
         * should be performed to ensure the interrupt returns directly to the highest
         * priority task.  The macro used for this purpose is dependent on the port in
         * use and may be called portEND_SWITCHING_ISR(). */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  struct ethernetif *ethernetif = netif->state;

  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* Set the real MAC hardware address */
  netif->hwaddr[0] = 0x00;
  netif->hwaddr[1] = 0x11;
  netif->hwaddr[2] = 0x22;
  netif->hwaddr[3] = 0x33;
  netif->hwaddr[4] = 0x44;
  netif->hwaddr[5] = 0x55;

  /* maximum transfer unit */
  netif->mtu = ipconfigNETWORK_MTU;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
  /*
   * For hardware/netifs that implement MAC filtering.
   * All-nodes link-local is handled by default, so we must let the hardware know
   * to allow multicast packets in.
   * Should set mld_mac_filter previously. */
  if (netif->mld_mac_filter != NULL) {
    ip6_addr_t ip6_allnodes_ll;
    ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
    netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
  }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

  // Initialize the receiver ISR and task.
  BaseType_t xReturn = xTaskCreate(prvRXHandlerTask,
		  "RXHandlerTask",
		  configMINIMAL_STACK_SIZE,
		  netif,
		  configMAX_PRIORITIES - 1,
		  &xRxHanderTaskHandle);
  LWIP_ASSERT("prvRXHandlerTask task create failed!", (xReturn == pdPASS));
  
  /* Initialize RA Ethernet interface. */
  // The RA Ethernet global variable initialization function.
  ethernetif->ra_ether0_ctrl = g_ether0.p_ctrl;
  ethernetif->ra_ether0_cfg = g_ether0.p_cfg;
  fsp_err_t err_ret = R_ETHER_Open(ethernetif->ra_ether0_ctrl, ethernetif->ra_ether0_cfg);
  LWIP_ASSERT("R_ETHER_Open failed!", (err_ret == FSP_SUCCESS));
  // Successfully opened network interface.
  netif->flags |= NETIF_FLAG_UP;
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct ethernetif *ethernetif = netif->state;
  struct pbuf *q;

  fsp_err_t fsp_err = R_ETHER_LinkProcess(ethernetif->ra_ether0_ctrl);
  if (fsp_err != FSP_SUCCESS)
  {
      ETHER_LOG_DEBUG("%s:%d: R_ETHER_LinkProcess failed!: fsp_err(%d)\n", __FUNCTION__, __LINE__, fsp_err);
      return ERR_IF;
  }

#if ETH_PAD_SIZE
  pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

  char p_buffer[ipconfigNETWORK_MTU];
  uint32_t len = 0;

  for (q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
      memcpy(p_buffer + len, q->payload, q->len);
      len += q->len;
  }
  ETHER_ASSERT(len <= ipconfigNETWORK_MTU);

  fsp_err = R_ETHER_Write(ethernetif->ra_ether0_ctrl, p_buffer, len);
  ETHER_LOG_DEBUG("%s:%d: R_ETHER_Write: fsp_err(%d), len(%d)\n", __FUNCTION__, __LINE__, fsp_err, len);
  ETHER_ASSERT(fsp_err == FSP_SUCCESS);

  MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
  if (((u8_t *)p->payload)[0] & 1) {
    /* broadcast or multicast packet*/
    MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
  } else {
    /* unicast packet */
    MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
  }
  /* increase ifoutdiscards or ifouterrors on error */

#if ETH_PAD_SIZE
  pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
  struct ethernetif *ethernetif = netif->state;
  struct pbuf *p, *q;
  uint32_t len;

  // Read Ethernet with zero copy.
  char p_buffer[ipconfigNETWORK_MTU];
  uint32_t len_temp = ipconfigNETWORK_MTU;
  fsp_err_t fsp_err = R_ETHER_Read(ethernetif->ra_ether0_ctrl, (void ** const)&p_buffer, &len_temp);
  ETHER_LOG_DEBUG("%s:%d: R_ETHER_Read: fsp_err(%d), len(%d)\n", __FUNCTION__, __LINE__, fsp_err, len_temp);
  if (fsp_err == FSP_ERR_ETHER_ERROR_NO_DATA)
  {
      // No data for now.
      return NULL;
  }
  ETHER_ASSERT(fsp_err == FSP_SUCCESS);

  len = len_temp;
#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, (u16_t)len, PBUF_POOL);

  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    uint32_t copied = 0;
    for (q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable.
       * This does not necessarily have to be a memcpy, you can also preallocate
       * pbufs for a DMA-enabled MAC and after receiving truncate it to the
       * actually received size. In this case, ensure the tot_len member of the
       * pbuf is the sum of the chained pbuf len members.
       */
      uint32_t copy_len = configMIN((uint32_t)q->len, len - copied);
      memcpy(q->payload, p_buffer + copied, copy_len);
      copied += copy_len;
    }
    LWIP_ASSERT("copied == len", (copied == len));

    MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
    if (((u8_t *)p->payload)[0] & 1) {
      /* broadcast or multicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
    } else {
      /* unicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
    }
#if ETH_PAD_SIZE
    pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
    MIB2_STATS_NETIF_INC(netif, ifindiscards);
  }

  return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input(struct netif *netif)
{
  // struct ethernetif *ethernetif;
  // struct eth_hdr *ethhdr;
  struct pbuf *p;

  // ethernetif = netif->state;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);
  /* if no packet could be read, silently ignore this */
  if (p != NULL) {
    /* pass all packets to ethernet_input, which decides what packets it supports */
    if (netif->input(p, netif) != ERR_OK) {
      LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
      pbuf_free(p);
      p = NULL;
    }
  }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
#if LWIP_IPV4
  netif->output = etharp_output;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
  netif->linkoutput = low_level_output;

  ethernetif->ethaddr = (struct eth_addr *) & (netif->hwaddr[0]);

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

