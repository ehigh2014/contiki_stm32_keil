/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  the example of rtimer, led blink period 200ms, rtimer 10KHz
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :  2015-11-26
 * @Update   :
 *
 */
#include "stm32f10x.h"
#include "stdint.h"
#include "stdio.h"
#include "stm32f10x_gpio.h"

#include "sys/autostart.h"
#include "debug-uart.h"
#include "clock.h"
#include "etimer.h"
#include "rtimer.h"
#include "sys/process.h"

#ifdef WITH_RTIMER_TEST

PROCESS(rtimer_ex_process, "rtimer example");

static void led_off(struct rtimer *t, void *ptr);
static void led_on(struct rtimer *t, void *ptr);

/* gloabal rtimer */
static struct rtimer ex_timer;

static void
led_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
}

static void
led_on(struct rtimer *t, void *ptr)
{
    GPIO_SetBits(GPIOF, GPIO_Pin_9);
    rtimer_set(&ex_timer, 1000, 0, led_off, NULL);
}

static void 
led_off(struct rtimer *t, void *ptr)
{
    GPIO_ResetBits(GPIOF, GPIO_Pin_9);
    rtimer_set(&ex_timer, 1000, 0, led_on, NULL);
}

/**
 * \brief  rtimer process only set the callback ok
 * \param 
 * 
 */
PROCESS_THREAD(rtimer_ex_process, ev, data)
{
    PROCESS_BEGIN();
    led_init();
	  /* start the led, period 200ms */
    rtimer_set(&ex_timer, 1000, 0, led_on, NULL);
    PROCESS_END();
}
#endif
