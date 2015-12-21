/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  Ultrasonic ranging module - SR04
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :  2015-12-21
 * @Update   :
 *
 */

#include "SR04.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define SR_GPIO			GPIOB
#define SR_TRIG_PIN GPIO_Pin_14
#define SR_ECHO_PIN GPIO_Pin_15

void
SR04_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOB clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = SR_TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SR_ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SR_GPIO, &GPIO_InitStructure);
}

void
SR04_Timer_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    /* set the clock 4MHz */
    uint16_t prescaler = (uint16_t)(SystemCoreClock  / 1000000) -1;

    /* timer clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* timer init */
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    /* change timer counter to RTIMER_ARCH_SECOND */
    TIM_PrescalerConfig(TIM3, prescaler, TIM_PSCReloadMode_Immediate);
}

void
SR04_Init(void)
{
    SR04_GPIO_Init();
    SR04_Timer_Init();
}


/**
 * \brief  Delay about 10us
 */
void
SR04_Delay(uint32_t t)
{
    uint32_t i=0;
    while(--t)
    {
        for(i=0; i<79; i++);
    }
}

/**
 * \brief  SR04 Ranging
 * \param
 * \return	distance in mm
 */
double
SR04_Ranging()
{
    uint32_t count;
    uint32_t timeout;
    double distance;
    GPIO_SetBits(SR_GPIO, SR_TRIG_PIN);
    SR04_Delay(5);
    GPIO_ResetBits(SR_GPIO, SR_TRIG_PIN);
    TIM3->CNT = 0;
    while(GPIO_ReadInputDataBit(SR_GPIO, SR_ECHO_PIN)==0)
    {
        timeout++;
        if(timeout>7200)  //about 10 ms
        {
            printf("Time out\n");
            return 0;
        }
    }
    TIM_Cmd(TIM3, ENABLE);
    while((GPIO_ReadInputDataBit(SR_GPIO, SR_ECHO_PIN)==1) && (TIM3->CNT < TIM3->ARR - 10));
    TIM_Cmd(TIM3, DISABLE);
    count = TIM3->CNT;
    distance = 0.034 * count / 2;

    return distance;
}





