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
#if WITH_ENC_EX_TEST
#include "enc28j60.h"
#include "contiki-conf.h"
#include "uip_arp.h"
#include "clock.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/**
 * periodic timer
 */
typedef struct
{
    uint16_t interval;
    uint32_t start;
} ptimer_t;

void ptimer_set(ptimer_t* ptimer, uint16_t interval)
{
    ptimer->interval = interval;
    ptimer->start = clock_time();
}

void ptimer_reset(ptimer_t* ptimer)
{
    ptimer->start = clock_time();
}

/**
 * check if expired
 */
uint8_t ptimer_expired(ptimer_t* ptimer)
{
    if(clock_time() > (ptimer->start + ptimer->interval)) {
        return 1;
    }
    else {
        return 0;
    }
}

//MAC
struct uip_eth_addr uip_mac;

static uint8_t ethernet_mac[6] = {0x04, 0x02, 0x35, 0x00, 0x01, 0x01};

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
    uip_ipaddr_t ipaddr;
    uip_mac_init();
    enc28j60_regs();

    /* uip stack init */
    uip_init();
//    uip_arp_init();
    //set local ip addr
    uip_ipaddr(&ipaddr, 192, 168, 1, 16);
    uip_sethostaddr(&ipaddr);
    //set the router addr
    uip_ipaddr(&ipaddr, 192, 168, 1, 1);
    uip_setdraddr(&ipaddr);
    //set the netmask
    uip_ipaddr(&ipaddr, 255, 255, 255, 0);
    uip_setnetmask(&ipaddr);

    uip_listen(uip_htons(1234));
}
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])


PROCESS(enc_ex_process, "Enc28j60 example");

PROCESS_THREAD(enc_ex_process, ev, data)
{
    static ptimer_t periodic_timer, arp_timer;
    PROCESS_BEGIN();
    enc_example_init();
    ptimer_set(&periodic_timer, CLOCK_SECOND/2);
    ptimer_set(&arp_timer, CLOCK_SECOND * 10);
    while(1)
    {
        static struct etimer et;
        uint8_t i;
        etimer_set(&et, 1);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
//				PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_POLL);
        //todo:
        uip_len = enc28j60_read(uip_buf, UIP_CONF_BUFFER_SIZE);
        if(uip_len >0)
        {
            //ip data
            if(BUF->type == uip_htons(UIP_ETHTYPE_IP))
            {
                uip_arp_ipin();
                uip_input();
                if(uip_len > 0)
                {
                    uip_arp_out();
                    enc28j60_send(uip_buf, uip_len);
                }
            }
            //ARP
            else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP))
            {
                uip_arp_arpin();
                if(uip_len > 0)
                {
									PRINTF("uip len:%d \n",uip_len);
                    enc28j60_send(uip_buf, uip_len);
                }
            }
        }

        //check if the timer expired
        if(ptimer_expired(&periodic_timer))
        {
            ptimer_reset(&periodic_timer);
            //all tcp
            for(i=0; i<UIP_CONNS; i++)
            {
                uip_periodic(i);
                if(uip_len > 0)
                {
                    uip_arp_out();
                    enc28j60_send(uip_buf, uip_len);
                }
            }
#if UIP_UDP
            for(i=0; i<UIP_UDP_CONNS; i++)
            {
                uip_periodic(i);
                if(uip_len >0)
                {
                    uip_arp_out();
                    enc28j60_send(uip_buf, uip_len);
                }
            }
#endif

            if(ptimer_expired(&arp_timer))
            {
                ptimer_reset(&arp_timer);
                uip_arp_timer();
            }
        }
    }
    PROCESS_END();
}

void
enc28j60_arch_irq_process(void)
{
	process_poll(&enc_ex_process);
}

void exa_appcall(void)
{
//    if(uip_newdata())
//    {
//        uip_send(uip_appdata, uip_len);
//    }
}

void exa_uip_appcall(void)
{

}

void uip_log(char *m)
{			    
	PRINTF("uIP log: %s \r\n", m);
}

#endif

