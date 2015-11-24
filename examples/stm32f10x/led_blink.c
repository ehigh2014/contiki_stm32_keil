/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header:
 * @Author :
 * @Date :
 * @Update:
 *
 */

#ifdef WITH_LED_BLINK

#include "stm32f10x.h"
#include "stdint.h"
#include "stdio.h"
#include "stm32f10x_gpio.h"

#include "sys/autostart.h"
#include "debug-uart.h"
#include "clock.h"
#include "etimer.h"
#include "sys/process.h"

#define LED_RCC   RCC_APB2Periph_GPIOF
#define LED_GPIO  GPIOF
#define LED_PIN  GPIO_Pin_8

PROCESS(blink_process, "Blink");

void
blink_led_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LED_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = LED_PIN;
    GPIO_Init(LED_GPIO, &GPIO_InitStructure);
}

PROCESS_THREAD(blink_process, ev, data)
{
    PROCESS_BEGIN();
    blink_led_init();
    while(1)
    {
        static struct etimer et;
        static uint8_t led_state = 0;
        etimer_set(&et, CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        if(led_state)
        {
            led_state = 0;
            GPIO_SetBits(LED_GPIO, LED_PIN);
        }
        else
        {
            led_state = 1;
            GPIO_ResetBits(LED_GPIO, LED_PIN);
        }
    }

    PROCESS_END();
}

#endif
