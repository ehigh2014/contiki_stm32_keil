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
    SPI_SSOutputCmd(ENC_SPI, DISABLE);
    SPI_Cmd(ENC_SPI, ENABLE);
}

void
enc28j60_arch_spi_init(void)
{
    enc28j60_rcc_config();
    enc28j60_gpio_config();
    enc28j60_spi_config();
}

uint8_t
enc28j60_arch_spi_write(uint8_t dat)
{
    /* Wait for SPI Tx buffer empty */
    while((ENC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);
    /* Send data */
    ENC_SPI->DR = dat;
    return 0;
}

uint8_t
enc28j60_arch_spi_read(void)
{
    /* Wait for SPI Rx */
    while((ENC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);
    return   ENC_SPI->DR;
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

