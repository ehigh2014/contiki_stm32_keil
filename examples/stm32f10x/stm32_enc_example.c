/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  The example for the enc28j60 on the STM32F10x
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :	2015-12-31
 * @Update   :
 *
 */

#include "enc28j60.h"
#include "contiki-conf.h"
#include "uip_arp.h"

//MAC
struct uip_eth_addr uip_mac;

static uint8_t ethernet_mac[6] = {0xA3, 0x52, 0x33, 0x01, 0x00, 0x01};

void
uip_mac_init(void)
{
		uint8_t i;
    //init the enc28j60
    enc28j60_init(ethernet_mac);
    for(i = 0; i<6; i++)
    {
        uip_mac.addr[i] = ethernet_mac[i];
    }
    uip_setethaddr(uip_mac);
}

void
enc_example_init(void)
{
	uip_mac_init();
	/* uip stack init */
	uip_init();
}


