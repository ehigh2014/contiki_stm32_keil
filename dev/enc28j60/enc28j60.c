/*
 * Copyright (c) 2012-2013, Thingsquare, http://www.thingsquare.com/.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "contiki.h"
#include "enc28j60.h"
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

//PROCESS(enc_watchdog_process, "Enc28j60 watchdog");

static uint8_t initialized = 0;
static uint8_t enc_mac_addr[6];
static int received_packets = 0;
static int sent_packets = 0;
//last bank
static uint8_t Enc28j60Bank;
static uint16_t NextPacketPtr;

void _delay_us(uint32_t us)
{
    uint32_t len;
    for (; us > 0; us --)
        for (len = 0; len < 20; len++ );
}

void delay_ms(uint32_t ms)
{
    uint32_t len;
    for (; ms > 0; ms --)
        for (len = 0; len < 100; len++ );
}

static uint8_t enc_read_op(uint8_t op, uint8_t address)
{
    uint8_t dat = 0;
    enc28j60_arch_spi_select();
    enc28j60_arch_spi_write(op | (address & ADDR_MASK));
    dat = enc28j60_arch_spi_read();
    if(address & 0x80)
    {
        //read the second
        dat = enc28j60_arch_spi_read();
    }
    enc28j60_arch_spi_deselect();
    return dat;
}

static uint8_t enc_write_op(uint8_t op, uint8_t address, uint8_t dat)
{
    enc28j60_arch_spi_select();
    enc28j60_arch_spi_write(op | (address & ADDR_MASK));
    enc28j60_arch_spi_write(dat);
    enc28j60_arch_spi_deselect();
}

/*---------------------------------------------------------------------------*/
static void
setregbank(uint8_t bank)
{
    if ((bank & BANK_MASK) != Enc28j60Bank)
    {
        enc_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
        enc_write_op(ENC28J60_BIT_FIELD_SET, ECON1, (bank & BANK_MASK)>>5);
        Enc28j60Bank = (bank & BANK_MASK);
    }
}

/*---------------------------------------------------------------------------*/
static uint8_t
readreg(uint8_t reg)
{
    setregbank(reg);
    return enc_read_op(ENC28J60_READ_CTRL_REG, reg);
}
/*---------------------------------------------------------------------------*/
static void
writereg(uint8_t reg, uint8_t data)
{
    setregbank(reg);
    enc_write_op(ENC28J60_WRITE_CTRL_REG, reg, data);
}

/*---------------------------------------------------------------------------*/
static void
writedatabyte(uint8_t byte)
{
    enc28j60_arch_spi_select();
    _delay_us(10);
    /* The Write Buffer Memory (WBM) command is 0 1 1 1 1 0 1 0  */
    enc28j60_arch_spi_write(ENC28J60_WRITE_BUF_MEM);
    enc28j60_arch_spi_write(byte);
    enc28j60_arch_spi_deselect();
}
/*---------------------------------------------------------------------------*/
static void
writedata(uint8_t *data, int datalen)
{
    enc28j60_arch_spi_select();
    _delay_us(10);
    /* The Write Buffer Memory (WBM) command is 0 1 1 1 1 0 1 0  */
    enc28j60_arch_spi_write(ENC28J60_WRITE_BUF_MEM);
    while(datalen--)
    {
        enc28j60_arch_spi_write(*data++);
    }
    enc28j60_arch_spi_deselect();
}
/*---------------------------------------------------------------------------*/
static uint8_t
readdatabyte(void)
{
    uint8_t r;
    enc28j60_arch_spi_select();
    /* THe Read Buffer Memory (RBM) command is 0 0 1 1 1 0 1 0 */
    enc28j60_arch_spi_write(ENC28J60_READ_BUF_MEM);
    r = enc28j60_arch_spi_read();
    enc28j60_arch_spi_deselect();
    return r;
}
/*---------------------------------------------------------------------------*/
static int
readdata(uint8_t *buf, int len)
{
    enc28j60_arch_spi_select();
    /* THe Read Buffer Memory (RBM) command is 0 0 1 1 1 0 1 0 */
    enc28j60_arch_spi_write(ENC28J60_READ_BUF_MEM);
    while(len--)
    {
        *buf++ = enc28j60_arch_spi_read();
    }
//    *buf = '\0';
    enc28j60_arch_spi_deselect();
    return len;
}

void enc28j60_phy_write(uint8_t address, uint16_t data)
{
    uint16_t retry = 0;
    // set the PHY register address
    writereg(MIREGADR, address);
    // write the PHY data
    writereg(MIWRL, data);
    writereg(MIWRH, data>>8);

    // wait until the PHY write completes
    while((readreg(MISTAT) & MISTAT_BUSY) && (retry < 0xFFF))
    {
        retry++;
    }
}

// read upper 8 bits
uint16_t enc28j60_phy_read(uint8_t address)
{
    // Set the right address and start the register read operation
    writereg(MIREGADR, address);
    writereg(MICMD, 0x01);
    _delay_us(15);
    // wait until the PHY read completes
    while(readreg(MISTAT) & MISTAT_BUSY);
    // reset reading bit
    writereg(MICMD, 0x00);
    return (readreg(MIRDH));
}

/*---------------------------------------------------------------------------*/
static void
softreset(void)
{
    enc28j60_arch_spi_select();
    /* The System Command (soft reset) is 1 1 1 1 1 1 1 1 */
    enc28j60_arch_spi_write(0xff);
    enc28j60_arch_spi_deselect();
}

uint32_t enc28j60_interrupt_disable()
{
    uint32_t level;
    /* switch to bank 0 */
    setregbank(EIE);
    /* get last interrupt level */
    level = readreg(EIE);
    /* disable interrutps */
    enc_write_op(ENC28J60_BIT_FIELD_CLR, EIE, level);
    return level;
}

void enc28j60_interrupt_enable(uint32_t level)
{
    setregbank(EIE);
    enc_write_op(ENC28J60_BIT_FIELD_SET, EIE, level);
}

void enc28j60_clkout(uint8_t clk)
{
    //setup clkout: 2 is 12.5MHz:
    writereg(ECOCON, clk & 0x7);
}

/*---------------------------------------------------------------------------*/
static void
reset(void)
{
    PRINTF("enc28j60: resetting chip\n");

    enc28j60_arch_spi_init();
    enc28j60_arch_spi_select();

    enc28j60_arch_enc_rst(0);
    delay_ms(50);
    enc28j60_arch_enc_rst(1);
    delay_ms(10);
    /*
      6.0 INITIALIZATION

      Before the ENC28J60 can be used to transmit and receive packets,
      certain device settings must be initialized. Depending on the
      application, some configuration options may need to be
      changed. Normally, these tasks may be accomplished once after
      Reset and do not need to be changed thereafter.

      6.1 Receive Buffer

      Before receiving any packets, the receive buffer must be
      initialized by programming the ERXST and ERXND pointers. All
      memory between and including the ERXST and ERXND addresses will be
      dedicated to the receive hardware. It is recommended that the
      ERXST pointer be programmed with an even address.

      Applications expecting large amounts of data and frequent packet
      delivery may wish to allocate most of the memory as the receive
      buffer. Applications that may need to save older packets or have
      several packets ready for transmission should allocate less
      memory.

      When programming the ERXST pointer, the ERXWRPT registers will
      automatically be updated with the same values. The address in
      ERXWRPT will be used as the starting location when the receive
      hardware begins writing received data. For tracking purposes, the
      ERXRDPT registers should additionally be programmed with the same
      value. To program ERXRDPT, the host controller must write to
      ERXRDPTL first, followed by ERXRDPTH.  See Section 7.2.4 “Freeing
      Receive Buffer Space for more information

      6.2 Transmission Buffer

      All memory which is not used by the receive buffer is considered
      the transmission buffer. Data which is to be transmitted should be
      written into any unused space.  After a packet is transmitted,
      however, the hardware will write a seven-byte status vector into
      memory after the last byte in the packet. Therefore, the host
      controller should leave at least seven bytes between each packet
      and the beginning of the receive buffer. No explicit action is
      required to initialize the transmission buffer.

      6.3 Receive Filters

      The appropriate receive filters should be enabled or disabled by
      writing to the ERXFCON register. See Section 8.0 “Receive Filters
      for information on how to configure it.

      6.4 Waiting For OST

      If the initialization procedure is being executed immediately
      following a Power-on Reset, the ESTAT.CLKRDY bit should be polled
      to make certain that enough time has elapsed before proceeding to
      modify the MAC and PHY registers. For more information on the OST,
      see Section 2.2 “Oscillator Start-up Timer.
    */

//  /* Wait for OST , always waiting */
    while((readreg(ESTAT) & ESTAT_CLKRDY) == 0);

    softreset();
    delay_ms(50);

    NextPacketPtr = RXSTART_INIT;
    /* Set up receive buffer */
    writereg(ERXSTL, RXSTART_INIT & 0xff);
    writereg(ERXSTH, RXSTART_INIT >> 8);
    writereg(ERXRDPTL, RXSTART_INIT & 0xff);
    writereg(ERXRDPTH, RXSTART_INIT >> 8);
    //rx end
    writereg(ERXNDL, RXSTOP_INIT & 0xff);
    writereg(ERXNDH, RXSTOP_INIT >> 8);

    //tx start
    writereg(ETXSTL, TXSTART_INIT & 0xff);
    writereg(ETXSTH, TXSTART_INIT >> 8);
    writereg(EWRPTL, TXSTART_INIT & 0xff);
    writereg(EWRPTH, TXSTART_INIT >> 8);
    //TX end
    writereg(ETXNDL, TXSTOP_INIT & 0xff);
    writereg(ETXNDH, TXSTOP_INIT >> 8);

    /* Receive filters */
    /*  writereg(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN |
        ERXFCON_MCEN | ERXFCON_BCEN);*/
    /* XXX: can't seem to get the unicast filter to work right now,
       using promiscous mode for now. */
//    writereg(ERXFCON, 0);
//    writereg(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_BCEN);
    writereg(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
    writereg(EPMM0, 0x3F);
    writereg(EPMM1, 0x30);
    writereg(EPMCSL, 0xF9);
    writereg(EPMCSH, 0xF7);
    /*
      6.5 MAC Initialization Settings

      Several of the MAC registers require configuration during
      initialization. This only needs to be done once; the order of
      programming is unimportant.

      1. Clear the MARST bit in MACON2 to pull the MAC out of Reset.

      2. Set the MARXEN bit in MACON1 to enable the MAC to receive
      frames. If using full duplex, most applications should also set
      TXPAUS and RXPAUS to allow IEEE defined flow control to function.

      3. Configure the PADCFG, TXCRCEN and FULDPX bits of MACON3. Most
      applications should enable automatic padding to at least 60 bytes
      and always append a valid CRC. For convenience, many applications
      may wish to set the FRMLNEN bit as well to enable frame length
      status reporting. The FULDPX bit should be set if the application
      will be connected to a full-duplex configured remote node;
      otherwise, it should be left clear.

      4. Configure the bits in MACON4. Many applications may not need to
      modify the Reset default.

      5. Program the MAMXFL registers with the maximum frame length to
      be permitted to be received or transmitted. Normal network nodes
      are designed to handle packets that are 1518 bytes or less.

      6. Configure the Back-to-Back Inter-Packet Gap register,
      MABBIPG. Most applications will program this register with 15h
      when Full-Duplex mode is used and 12h when Half-Duplex mode is
      used.

      7. Configure the Non-Back-to-Back Inter-Packet Gap register low
      byte, MAIPGL. Most applications will program this register with
      12h.

      8. If half duplex is used, the Non-Back-to-Back Inter-Packet Gap
      register high byte, MAIPGH, should be programmed. Most
      applications will program this register to 0Ch.

      9. If Half-Duplex mode is used, program the Retransmission and
      Collision Window registers, MACLCON1 and MACLCON2. Most
      applications will not need to change the default Reset values.  If
      the network is spread over exceptionally long cables, the default
      value of MACLCON2 may need to be increased.

      10. Program the local MAC address into the
      MAADR0:MAADR5 registers.
    */
    /* Pull MAC out of reset */
    /* Turn on reception and IEEE-defined flow control */
//    writereg(MACON1, readreg(MACON1) | (MACON1_MARXEN + MACON1_TXPAUS +
//                                        MACON1_RXPAUS));
		writereg(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
    writereg(MACON2, 0);//readreg(MACON2) & (~MACON2_MARST));

    enc_write_op(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);

    /* Set non-back-to-back packet gap */
    writereg(MAIPGL, 0x12);
    writereg(MAIPGH, 0x0c);

    /* Set back-to-back inter packet gap */
    writereg(MABBIPG, 0x15);
    /* Don't modify MACON4 */
    /* Set maximum frame length in MAMXFL */
    writereg(MAMXFLL, MAX_FRAMELEN & 0xff);
    writereg(MAMXFLH, MAX_FRAMELEN >> 8);

//    writereg(MACON4, (1<<6));
//    writereg(MACLCON2, 63);

    /* Set MAC address */
    writereg(MAADR5, enc_mac_addr[0]);
    writereg(MAADR4, enc_mac_addr[1]);
    writereg(MAADR3, enc_mac_addr[2]);
    writereg(MAADR2, enc_mac_addr[3]);
    writereg(MAADR1, enc_mac_addr[4]);
    writereg(MAADR0, enc_mac_addr[5]);

    /*
      6.6 PHY Initialization Settings

      Depending on the application, bits in three of the PHY module’s
      registers may also require configuration.  The PHCON1.PDPXMD bit
      partially controls the device’s half/full-duplex
      configuration. Normally, this bit is initialized correctly by the
      external circuitry (see Section 2.6 “LED Configuration). If the
      external circuitry is not present or incorrect, however, the host
      controller must program the bit properly. Alternatively, for an
      externally configurable system, the PDPXMD bit may be read and the
      FULDPX bit be programmed to match.

      For proper duplex operation, the PHCON1.PDPXMD bit must also match
      the value of the MACON3.FULDPX bit.

      If using half duplex, the host controller may wish to set the
      PHCON2.HDLDIS bit to prevent automatic loopback of the data which
      is transmitted.  The PHY register, PHLCON, controls the outputs of
      LEDA and LEDB. If an application requires a LED configuration
      other than the default, PHLCON must be altered to match the new
      requirements. The settings for LED operation are discussed in
      Section 2.6 “LED Configuration. The PHLCON register is shown in
      Register 2-2 (page 9).
    */
    enc28j60_phy_write(PHCON1, PHCON1_PDPXMD); // full duplex
    // no loopback of transmitted frames
    enc28j60_phy_write(PHCON2, PHCON2_HDLDIS);
    /* Receive filters */
    setregbank(ECON1);
    enc_write_op(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
    /* Turn on reception */
    enc_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

    /* Don't worry about PHY configuration for now */

    /* Turn on autoincrement for buffer access */
    setregbank(ECON2);
    writereg(ECON2, readreg(ECON2) | ECON2_AUTOINC);

    enc28j60_phy_write(PHLCON, 0X476);	//0x476
    enc28j60_clkout(2);
    delay_ms(20);
}
/*---------------------------------------------------------------------------*/
void
enc28j60_init(uint8_t *mac_addr)
{
    if(initialized) {
        return;
    }
    memcpy(enc_mac_addr, mac_addr, 6);

    reset();

    initialized = 1;
}
/*---------------------------------------------------------------------------*/
int
enc28j60_send(uint8_t *pdat, uint16_t len)
{
    uint16_t retry = 0;
    uint32_t level = 0;
    if(!initialized) {
        return -1;
    }
//    while ((readreg(ECON1) & ECON1_TXRTS) && (retry < 0xFFFF))
//    {
//        retry++;
//    }
//    if(retry >= 0xFFFF)
//    {
//        enc_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
//    }
    level = enc28j60_interrupt_disable();
    while((readreg(ECON1)& ECON1_TXRTS) != 0);
    writereg(EWRPTL, TXSTART_INIT & 0xFF);
    writereg(EWRPTH, TXSTART_INIT >> 8);
    /* TXND pointer */
    writereg(ETXNDL,(TXSTART_INIT + len) & 0xFF);
    writereg(ETXNDH,(TXSTART_INIT + len) >> 8);

		
    enc_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
    //write to the buffer
    writedata(pdat, len);
    enc_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
    if ((readreg(EIR) & EIR_TXERIF))
    {
        setregbank(ECON1);
        enc_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
    }
    enc28j60_interrupt_enable(level);
    return 0;
}
/*---------------------------------------------------------------------------*/
int
enc28j60_read(uint8_t *buffer, uint16_t bufsize)
{
    int n;

    uint16_t len;
    uint16_t rxstat;

    n = readreg(EPKTCNT);
    if(n == 0) {
        return 0;
    }

    PRINTF("enc28j60: EPKTCNT 0x%02x\n", n);

    /* Read buffer pointer */
    writereg(ERDPTL, (NextPacketPtr));
    writereg(ERDPTH, (NextPacketPtr)>>8);

    /* Read the next packet pointer */
    NextPacketPtr = enc_read_op(ENC28J60_READ_BUF_MEM, 0);
    NextPacketPtr |= (enc_read_op(ENC28J60_READ_BUF_MEM, 0)<<8);

    /* Read the packet length */
    len = enc_read_op(ENC28J60_READ_BUF_MEM, 0);
    len |= (enc_read_op(ENC28J60_READ_BUF_MEM, 0)<<8);
    len -= 4; //remove CRC

    /* Read the recv stat */
    rxstat = enc_read_op(ENC28J60_READ_BUF_MEM, 0);
    rxstat |= (enc_read_op(ENC28J60_READ_BUF_MEM, 0)<<8);

    if( len > (bufsize - 1))
    {
        len = bufsize - 1;
    }

    if((rxstat & 0x80)==0)
    {
        len = 0;
    }
    else
    {
        readdata(buffer, bufsize);
    }

    writereg(ERXRDPTL, (NextPacketPtr));
    writereg(ERXRDPTH, (NextPacketPtr)>>8);

    // decrement the packet counter indicate we are done with this packet
    enc_write_op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

    return len;
}

void
enc28j60_regs(void)
{
    PRINTF("-- enc28j60 registers:\n");
    PRINTF("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\n");
    PRINTF("       0x%02x  0x%02x  0x%02x  0x%02x  0x%02x\n",readreg(ECON1), readreg(ECON2), readreg(ESTAT), readreg(EIR), readreg(EIE));
    PRINTF("HwRevID: 0x%02x\n", readreg(EREVID));
    PRINTF("MAC  : MAC 3 MAADR4 MAADR5\n");
    PRINTF("      0x%02x 0x%02x   0x%02x\n", readreg(MAADR3), readreg(MAADR4), readreg(MAADR5));
    PRINTF("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\n");
    PRINTF("       0x%04x 0x%04x 0x%04x  0x%04x  ",
           (readreg(ERXSTH) << 8) | readreg(ERXSTL),
           (readreg(ERXNDH) << 8) | readreg(ERXNDL),
           (readreg(ERXWRPTH) << 8) | readreg(ERXWRPTL),
           (readreg(ERXRDPTH) << 8) | readreg(ERXRDPTL));
    PRINTF("0x%02x    0x%02x    0x%04x\n", readreg(ERXFCON), readreg(EPKTCNT),
           (readreg(MAMXFLH) << 8) | readreg(MAMXFLL));
    PRINTF("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\n");
    PRINTF("       0x%04x 0x%04x 0x%02x     0x%02x     0x%02x\n",
           (readreg(ETXSTH) << 8) | readreg(ETXSTL),
           (readreg(ETXNDH) << 8) | readreg(ETXNDL),
           readreg(MACLCON1), readreg(MACLCON2), readreg(MAPHSUP));
}


/*---------------------------------------------------------------------------*/
//PROCESS_THREAD(enc_watchdog_process, ev, data)
//{
//  static struct etimer et;

//  PROCESS_BEGIN();

//  while(1) {
//#define RESET_PERIOD (30 * CLOCK_SECOND)
//    etimer_set(&et, RESET_PERIOD);
//    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

//    PRINTF("enc28j60: test received_packet %d > sent_packets %d\n", received_packets, sent_packets);
//    if(received_packets <= sent_packets) {
//      PRINTF("enc28j60: resetting chip\n");
//      reset();
//    }
//    received_packets = 0;
//    sent_packets = 0;
//  }

//  PROCESS_END();
//}
/*---------------------------------------------------------------------------*/
