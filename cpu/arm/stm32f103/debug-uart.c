/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header:
 * @Author :  luochao
 * @Email   :  luochao@everhigh.com.cn
 * @Date :
 * @Update:
 *
 */

#include <debug-uart.h>
#include <string.h>
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stdio.h"

#define CK_UART_1 1
#define CK_UART_2 2
#define CK_UART_3 3

#define CK_UART CK_UART_1

#if CK_UART==CK_UART_1
#define DBG_UART	USART1
#define DBG_UART_RCC  RCC_APB2Periph_USART1

#define DBG_UART_TX_RCC  RCC_APB2Periph_GPIOA
#define DBG_UART_TX_GPIO GPIOA
#define DBG_UART_TX_PIN  GPIO_Pin_9

#define DBG_UART_RX_RCC  RCC_APB2Periph_GPIOA
#define DBG_UART_RX_GPIO GPIOA
#define DBG_UART_RX_PIN  GPIO_Pin_10

#else
#error "Please set a valid UART!"

#endif

void
dbg_setup_uart(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    //clock
#if CK_UART==CK_UART_1
    RCC_APB2PeriphClockCmd(DBG_UART_RCC, ENABLE);
	  RCC_APB2PeriphClockCmd(DBG_UART_TX_RCC, ENABLE);
#endif
	
    //TX port
    GPIO_InitStructure.GPIO_Pin= DBG_UART_TX_PIN;
    GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode= GPIO_Mode_AF_PP;
    GPIO_Init(DBG_UART_TX_GPIO,&GPIO_InitStructure);

    //RX port
    GPIO_InitStructure.GPIO_Pin= DBG_UART_RX_PIN;
    GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IN_FLOATING;
    GPIO_Init(DBG_UART_RX_GPIO,&GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate= 115200;
    USART_InitStructure.USART_WordLength= USART_WordLength_8b;
    USART_InitStructure.USART_StopBits= USART_StopBits_1;
    USART_InitStructure.USART_Parity= USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl= USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode= USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(DBG_UART,&USART_InitStructure);
    //UART Enable
    USART_Cmd(DBG_UART,ENABLE);
}

/**
 * \brief  Use the printf, include "stdio.h" also need use the MicroLIB
 * \param 
 * 
 *
 */
int
fputc(int ch, FILE* f)
{
    USART_SendData(DBG_UART,(uint8_t)ch);
    while(USART_GetFlagStatus(DBG_UART, USART_FLAG_TXE)== RESET );
    return ch;
}


