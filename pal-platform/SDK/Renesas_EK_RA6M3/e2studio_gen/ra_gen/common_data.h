/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "r_ether_phy.h"
#include "r_ether_phy_api.h"
#include "r_ether.h"
#include "r_ether_api.h"
#include "FreeRTOSIPConfig.h"
#include "r_ioport.h"
#include "bsp_pin_cfg.h"
FSP_HEADER
extern const ether_phy_instance_t g_ether_phy0;
/** ether on ether Instance. */
extern const ether_instance_t g_ether0;
#ifndef vEtherISRCallback
void vEtherISRCallback(ether_callback_args_t *p_args);
#endif
extern ether_instance_t const *gp_freertos_ether;
/* IOPORT Instance */
extern const ioport_instance_t g_ioport;

/* IOPORT control structure. */
extern ioport_instance_ctrl_t g_ioport_ctrl;
void g_common_init(void);
FSP_FOOTER
#endif /* COMMON_DATA_H_ */
