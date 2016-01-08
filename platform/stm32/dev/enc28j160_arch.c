/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  STM32 for ENC28J60
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :  2015-12-31
 * @Update   :
 *
 */

#include "enc28j60.h"
#include "stm32f10x.h"
#include "stdint.h"

#define ENC_GPIO GPIOB
#define ENC_SI_Pin	GPIO_Pin_15
#define ENC_SO_Pin	GPIO_Pin_14
#define ENC_SCK_Pin	GPIO_Pin_13
#define ENC_CS_Pin	GPIO_Pin_12
#define ENC_RST_Pin	GPIO_Pin_11
#define ENC_INT_Pin GPIO_Pin_10
#define ENC_SPI			SPI2

static void
enc28j60_rcc_config(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
}

static void
enc28j60_gpio_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = ENC_CS_Pin | ENC_RST_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ENC_GPIO, &GPIO_InitStructure);

    /* Configure SPI2 pins:  SCK, MISO and MOSI */
    GPIO_InitStructure.GPIO_Pin = ENC_SI_Pin | ENC_SCK_Pin | ENC_SO_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(ENC_GPIO, &GPIO_InitStructure);
}

static void
enc28j60_spi_config(void)
{
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(ENC_SPI, &SPI_InitStructure);
    SPI_Cmd(ENC_SPI, ENABLE);
}

static void
enc28j60_nvic_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_InitStructure.GPIO_Pin = ENC_INT_Pin;
    GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_IPU;	//IRQ pin should be Pull Down to prevent unnecessary EXT IRQ while DW1000 goes to sleep mode
    GPIO_Init(ENC_GPIO, &GPIO_InitStructure);

    /* Connect EXTI Line to GPIO Pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

    /* Configure EXTI line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;	//MPW3 IRQ polarity is high by default
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Set NVIC Grouping to 16 groups of interrupt without sub-grouping */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Enable and set EXTI Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

//void
//enc28j60_arch_irq_process(void)
//{

//}

void
enc28j60_arch_spi_init(void)
{
    enc28j60_rcc_config();
    enc28j60_spi_config();
    enc28j60_gpio_config();
//		enc28j60_nvic_config();
}

uint8_t
enc28j60_arch_spi_write(uint8_t dat)
{
		while(SPI_I2S_GetFlagStatus(ENC_SPI, SPI_I2S_FLAG_TXE) == RESET)
		{}
    ENC_SPI->DR = dat;
    /* Wait for SPI Tx buffer empty */
    while((ENC_SPI->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);
    return (uint8_t)SPI_I2S_ReceiveData(ENC_SPI);
}

uint8_t
enc28j60_arch_spi_rw(uint8_t dat)
{
    /*!< Wait until the transmit buffer is empty */
    while (SPI_I2S_GetFlagStatus(ENC_SPI, SPI_I2S_FLAG_TXE) == RESET)
    {
    }
    /*!< Send the byte */
    SPI_I2S_SendData(ENC_SPI, dat);
    
    /*!< Wait until a data is received */
    while (SPI_I2S_GetFlagStatus(ENC_SPI, SPI_I2S_FLAG_RXNE) == RESET)
    {
    }
    /*!< Get the received data */
    dat = SPI_I2S_ReceiveData(ENC_SPI);
    
    /*!< Return the shifted data */
    return dat;
}

#pragma O3
uint8_t
enc28j60_arch_spi_read(void)
{
    uint8_t dat;
//		while((ENC_SPI->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
    ENC_SPI->DR = 0xFF;
    /* Wait for SPI Rx */
    while((ENC_SPI->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);
    dat = ENC_SPI->DR;
    return dat;
}

/**
 * \brief  SPI CS Low
 * \param
 *
 */
void
enc28j60_arch_spi_select(void)
{
    GPIO_ResetBits(ENC_GPIO, ENC_CS_Pin);
}

/**
 * \brief  SPI CS High
 * \param
 *
 */
void
enc28j60_arch_spi_deselect(void)
{
    GPIO_SetBits(ENC_GPIO, ENC_CS_Pin);
}

void
enc28j60_arch_enc_rst(uint8_t t)
{
    if(t)
    {
        GPIO_SetBits(ENC_GPIO, ENC_RST_Pin);
    }
    else
    {
        GPIO_ResetBits(ENC_GPIO, ENC_CS_Pin);
    }
}
