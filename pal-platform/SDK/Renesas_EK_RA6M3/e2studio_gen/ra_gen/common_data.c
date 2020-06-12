/* generated common source file - do not edit */
#include "common_data.h"
ether_phy_instance_ctrl_t g_ether_phy0_ctrl;

static const ether_phy_cfg_t g_ether_phy0_cfg =
{

.channel = 0,
  .phy_lsi_address = 0, .phy_reset_wait_time = 0x00020000, .mii_bit_access_wait_time = 8, .flow_control =
          ETHER_PHY_FLOW_CONTROL_DISABLE,
  .p_context = NULL, .p_extend = NULL,

};
/* Instance structure to use this module. */
const ether_phy_instance_t g_ether_phy0 =
{ .p_ctrl = &g_ether_phy0_ctrl, .p_cfg = &g_ether_phy0_cfg, .p_api = &g_ether_phy_on_ether_phy };
static ether_instance_ctrl_t g_ether0_ctrl;

uint8_t g_ether0_mac_address[6] =
{ 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };

__attribute__((__aligned__(16)))  ether_instance_descriptor_t g_ether0_tx_descriptors[1];
__attribute__((__aligned__(16)))  ether_instance_descriptor_t g_ether0_rx_descriptors[1];

__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer0[1514];
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer1[1514];

uint8_t *pp_g_ether0_ether_buffers[(4 + 4)] =
{ (uint8_t *) &g_ether0_ether_buffer0[0], (uint8_t *) &g_ether0_ether_buffer1[0],

};

static const ether_cfg_t g_ether0_cfg =
{ .channel = 0, .zerocopy = ETHER_ZEROCOPY_DISABLE, .multicast = ETHER_MULTICAST_ENABLE, .promiscuous =
          ETHER_PROMISCUOUS_DISABLE,
  .flow_control = ETHER_FLOW_CONTROL_ENABLE, .broadcast_filter = 0, .p_mac_address = g_ether0_mac_address,

  .p_rx_descriptors = g_ether0_rx_descriptors,
  .p_tx_descriptors = g_ether0_tx_descriptors,

  .num_tx_descriptors = 1,
  .num_rx_descriptors = 1,

  .pp_ether_buffers = pp_g_ether0_ether_buffers,

  .ether_buffer_size = 1514,

#if defined(VECTOR_NUMBER_EDMAC0_EINT)
  .irq = VECTOR_NUMBER_EDMAC0_EINT,
#else
  .irq = FSP_INVALID_VECTOR,
#endif

  .interrupt_priority = (12),

  .p_callback = vEtherISRCallback,
  .p_ether_phy_instance = &g_ether_phy0, .p_context = NULL, .p_extend = NULL, };

/* Instance structure to use this module. */
const ether_instance_t g_ether0 =
{ .p_ctrl = &g_ether0_ctrl, .p_cfg = &g_ether0_cfg, .p_api = &g_ether_on_ether };
ether_instance_t const *gp_freertos_ether = &g_ether0;
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_ctrl = &g_ioport_ctrl, .p_cfg = &g_bsp_pin_cfg, };

void g_common_init(void)
{
}
