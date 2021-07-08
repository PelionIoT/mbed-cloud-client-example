/* Copyright (c) 2021 Pelion
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

#include <zephyr.h>
#include <net/net_if.h>
#include <net/dhcpv4.h>
#include <net/net_mgmt.h>

#include <stdio.h>

extern "C" int mbed_cloud_application_entrypoint(void);

void main(void)
{
	printf("Pelion Device Management Client Example\r\n");

    struct net_if *iface = net_if_get_default();

    net_dhcpv4_start(iface);

    net_mgmt_event_wait(NET_EVENT_IPV4_ADDR_ADD, NULL, NULL, NULL, NULL, K_FOREVER);

	mbed_cloud_application_entrypoint();
}
