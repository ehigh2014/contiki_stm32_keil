/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :
 * @Update   :
 *
 */

#include "uart1.h"
#include "sys/energest.h"

#include "stm32f10x.h"

#ifndef NULL
#define NULL (void *)0
#endif

static int (* uart1_input_handler)(unsigned char c);

void uart1_set_input(int (* input)(unsigned char c))
{
    uart1_input_handler = input;
}

/**
 * \brief UART IRQ Handler
 * \param void
 *
 *
 */
void USART1_IRQHandler(void)
{
    ENERGEST_ON(ENERGEST_TYPE_IRQ);
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
			if(uart1_input_handler != NULL)
			{
        /* Read one byte from the receive data register */
        uint8_t buf = USART_ReceiveData(USART1);
				uart1_input_handler(buf);
			}
			
    }
    ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}


