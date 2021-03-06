/* Networking DHCPv4 client */

/*
 * Copyright (c) 2017 ARM Ltd.
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER (dhcp, LOG_LEVEL_DBG);

#include <errno.h>
#include <linker/sections.h>
#include <stdio.h>
#include <zephyr.h>

#include <net/net_context.h>
#include <net/net_core.h>
#include <net/net_if.h>
#include <net/net_mgmt.h>

static struct net_mgmt_event_callback mgmt_cb;
static int initialized = 0; // TODO ugly

static void handler (struct net_mgmt_event_callback *cb, u32_t mgmt_event, struct net_if *iface)
{
        int i = 0;

        if (mgmt_event != NET_EVENT_IPV4_ADDR_ADD) {
                return;
        }

        for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
                char buf[NET_IPV4_ADDR_LEN];

                if (iface->config.ip.ipv4->unicast[i].addr_type != NET_ADDR_DHCP) {
                        continue;
                }

                LOG_INF ("Your address: %s",
                         log_strdup (net_addr_ntop (AF_INET, &iface->config.ip.ipv4->unicast[i].address.in_addr, buf, sizeof (buf))));
                LOG_INF ("Lease time: %u seconds", iface->config.dhcpv4.lease_time);
                LOG_INF ("Subnet: %s", log_strdup (net_addr_ntop (AF_INET, &iface->config.ip.ipv4->netmask, buf, sizeof (buf))));
                LOG_INF ("Router: %s", log_strdup (net_addr_ntop (AF_INET, &iface->config.ip.ipv4->gw, buf, sizeof (buf))));
        }

        initialized = true;
}

int dhcpIsnitializded () { return initialized; }

void dhcpInit ()
{
        struct net_if *iface;

        LOG_INF ("Run dhcpv4 client");

        net_mgmt_init_event_callback (&mgmt_cb, handler, NET_EVENT_IPV4_ADDR_ADD);
        net_mgmt_add_event_callback (&mgmt_cb);

        iface = net_if_get_default ();

        net_dhcpv4_start (iface);
}
