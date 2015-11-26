/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  An example of shell command
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :  2015-11-24
 * @Update   :
 *
 */

#include "contiki.h"
#include "shell.h"

#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"


PROCESS(shell_blink_process, "shell blink");
SHELL_COMMAND(blink_command, "blink",
              "blink on/off : led on/off", &shell_blink_process);

static void
led_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
}

/**
 * \brief
 * \param
 *
 */
static void
led_on()
{
    GPIO_SetBits(GPIOF, GPIO_Pin_7);
}

static void
led_off()
{
    GPIO_ResetBits(GPIOF, GPIO_Pin_7);
}

PROCESS_THREAD(shell_blink_process, ev, data)
{
    PROCESS_BEGIN();
    if(data != NULL)
    {
        if(strcmp(data, "on"))
        {
            led_on();
        }
        else if(strcmp(data, "off"))
        {
            led_off();
        }
        else
        {
            printf("Invalid param!\n");
        }
    }

    PROCESS_END();
}

void
shell_blink_init(void)
{
    led_init();
    shell_register_command(&blink_command);
}

