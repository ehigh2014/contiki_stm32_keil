/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header :
 * @Author : luochao
 * @Email  : luochao@everhigh.com.cn
 * @Date 	 : 2015-11-25
 * @Update :
 *
 * \file
 *     RTIMER for STM32F10x
 */

#include "sys/rtimer.h"
#include "sys/clock.h"
#include "contiki-conf.h"
#include "sys/energest.h"

#include "stm32f10x.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define RTIMER_TIM_RCC RCC_APB1Periph_TIM2
#define RTIMER_TIM_IRQ TIM2_IRQn
#define RTIMER_TIM	TIM2

static volatile uint16_t CCR1_Val = 65535;

void
rtimer_nvic_config()
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the RTIMER_TIM global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTIMER_TIM_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void
rtimer_arch_enable_irq()
{
    TIM_ITConfig(RTIMER_TIM, TIM_IT_CC1, ENABLE);
}

void
rtimer_arch_disable_irq()
{
    TIM_ITConfig(RTIMER_TIM, TIM_IT_CC1, DISABLE);
}

void
rtimer_arch_init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
		/* set the rtimer clock */
    uint16_t prescaler = (uint16_t)(SystemCoreClock  / RTIMER_ARCH_SECOND) -1;

    PRINTF("rtimer_arch_init() \n");
    /* timer clock enable */
    RCC_APB1PeriphClockCmd(RTIMER_TIM_RCC, ENABLE);

    /* timer init */
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(RTIMER_TIM, &TIM_TimeBaseStructure);
    /* change timer counter to RTIMER_ARCH_SECOND */
    TIM_PrescalerConfig(RTIMER_TIM, prescaler, TIM_PSCReloadMode_Immediate);

    /* Output Compare */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(RTIMER_TIM, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);

    TIM_Cmd(RTIMER_TIM, ENABLE);
		/*  config the RTIMER_TIM IRQ */
    rtimer_nvic_config();
}


void
rtimer_arch_schedule(rtimer_clock_t t)
{
    rtimer_clock_t now;
    uint32_t counter;

    now = rtimer_arch_now();
    counter = now + t;
    /* overflow */
    if(counter > 65535)
    {
        CCR1_Val = counter - 65535;
    }
    else
    {
        CCR1_Val = (rtimer_clock_t)counter;
    }
    /* set the CCR1 Val, enable irq */
    RTIMER_TIM->CCR1 = CCR1_Val;
    rtimer_arch_enable_irq();
}

rtimer_clock_t
rtimer_arch_now(void)
{
    return TIM_GetCounter(RTIMER_TIM);
}

/**
 * \brief  TIM2 IRQ Handler
 * \param
 *
 *
 */
void TIM2_IRQHandler(void)
{
    ENERGEST_ON(ENERGEST_TYPE_IRQ);
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
				/* disable irq and shedule */
        rtimer_arch_disable_irq();
        rtimer_run_next();
    }
    ENERGEST_ON(ENERGEST_TYPE_IRQ);
}


