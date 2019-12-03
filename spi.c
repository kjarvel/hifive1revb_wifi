/* ------------------------------------------------------------------------
GPIO 3 = SPI MOSI
GPIO 4 = SPI MISO
GPIO 5 = SPI SCK
GPIO 9 = SPI CS2 (Chip select)
GPIO 10 = WF INT (Handshake)

Solder across SJ1 to connect GPIO_9 to SPI_CA2
Solder across SJ2 to connect GPIO_10 to WF_RESET (should be WF_INT/Handshake)

Each GPIO pin can implement up to 2 HW-Driven functions (IOF):

GPIO    IOF0
3       SPI1_DQ0 (MOSI)
4       SPI1_DQ1 (MISO)
5		SPI1_SCK (CLOCK)
9		SPI1_CS2 (CHIP SELECT)

- ESP32 is slave
- Hifive1 FE310-G002 is master

For receving part in ESP32, see:
https://github.com/espressif/esp-at/blob/release/v1.1.0.0/main/interface/hspi/at_hspi_task.c
https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
------------------------------------------------------------------------*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "spi.h"
#include "cpu.h"


#define IOF_EN      *(volatile uint32_t*)0x10012038
#define IOF_SEL     *(volatile uint32_t*)0x1001203C
#define INPUT_VAL   *(volatile uint32_t*)0x10012000
#define INPUT_EN    *(volatile uint32_t*)0x10012004

#define SPI1_FMT    *(volatile uint32_t*)0x10024040
#define PROTO_I     0U
#define ENDIAN_I    2U
#define DIR_I       3U
#define LEN_I       16U

#define SPI1_SCKDIV *(volatile uint32_t*)0x10024000
#define SPI1_CSID   *(volatile uint32_t*)0x10024010
#define SPI1_CSDEF  *(volatile uint32_t*)0x10024014
#define SPI1_CSMODE *(volatile uint32_t*)0x10024018
#define SPI1_FCTRL  *(volatile uint32_t*)0x10024060
#define SPI1_RXDATA *(volatile uint32_t*)0x1002404C
#define SPI1_TXDATA *(volatile uint32_t*)0x10024048

#define BITS(idx, val)  ((val) << (idx))

#define BIT_MASK(bit) (1UL<<(bit))
#define SPI_DELAY 100
#define SPI_DELAY_LONG 10000

#define HS_PIN 10
#define SPI1_DQ0_MOSI 3
#define SPI1_DQ1_MISO 4
#define SPI1_SCK 5
#define SPI1_CS2 9

#define CS_AUTO 0
#define CS_HOLD 2
#define CS_OFF  3

#define IOF_SPI_ENABLE (BIT_MASK(SPI1_DQ0_MOSI) | BIT_MASK(SPI1_DQ1_MISO) | BIT_MASK(SPI1_SCK)| BIT_MASK(SPI1_CS2))

static uint32_t handshake_ready(void);
static void spi_rxdata_read(void);
static void cs_deassert(void);
static void spi_send_recv_header(void);
static void spi_send_recv_length(uint8_t len);

//----------------------------------------------------------------------
static void cs_deassert(void)
{
    delay(SPI_DELAY_LONG);
}

//----------------------------------------------------------------------
void spi_init(uint32_t spi_clock)
{
    IOF_EN &= ~BIT_MASK(HS_PIN);    // Make sure Handhake pin is GPIO
    INPUT_EN |= BIT_MASK(HS_PIN);   // Handshake pin is input on master
    
    SPI1_FCTRL = 0;                 // 1:SPI flash mode, 0:programmed I/O mode

    // PROTO_I = 0  : SPI single mode
    // ENDIAN_I = 0 : MSB
    // DIR_I = 0    : Rx: The DQ0 pin is driven with the transmit data as normal.
    SPI1_FMT = BITS(LEN_I, 8);      // 8 bits per frame

    SPI1_CSDEF = 0xFFFFFFFFUL;      // CS inactive state high
    SPI1_CSMODE = CS_AUTO;
    SPI1_CSID = 2;                  // index of the CS pin to be toggled (CS2)

    // Set SPI clock
    SPI1_SCKDIV = (cpu_freq() / (2UL * spi_clock)) - 1UL;
    delay(SPI_DELAY);
    
    // Select HW I/O function 0 (IOF0) for all pins. 
    // IOF0 means SPI mode for GPIO pins 2-10.
    IOF_SEL = 0;
    
    printf("[+] Enabling IOF/SPI pins...");
    delay(SPI_DELAY);

    // Enable IOF for pin 3,4,5,9 (don't touch other pins)
    IOF_EN |= IOF_SPI_ENABLE;
    
    printf("DONE\r\n");
}

//----------------------------------------------------------------------
static void spi_rxdata_read(void)
{
    
    delay(SPI_DELAY); // delay required
    while (1) {
        if (SPI1_RXDATA > 0xFF) { // empty is set, return
            return;
        }
    }
}

//----------------------------------------------------------------------
static uint32_t handshake_ready(void)
{
    return ((INPUT_VAL & BIT_MASK(HS_PIN)) != 0) ? 1: 0;
}

//----------------------------------------------------------------------
static void spi_send_recv_header(void)
{
    const uint8_t at_flag_buf[] = {0x02, 0x00, 0x00, 0x00};

    printf(" | spi_send_recv_header: sending: %02x %02x %02x %02x\r\n",
           at_flag_buf[0], at_flag_buf[1], at_flag_buf[2], at_flag_buf[3]);
    
    for (uint32_t i = 0; i < 4; i++) {
        while (SPI1_TXDATA > 0xFF) {} // full bit set, wait
        SPI1_TXDATA = at_flag_buf[i];
        spi_rxdata_read();
    }
    
    cs_deassert();
    printf(" | Waiting for handshake pin ready...");
    while (!handshake_ready()) {}
    printf("DONE\r\n");
}

//----------------------------------------------------------------------
static void spi_send_recv_length(uint8_t len)
{
    uint8_t len_buf[] = {0x00, 0x00, 0x00, 'A'};

    len_buf[0] = len;
    
    printf(" | spi_send length: sending: (%d) %02x %02x %02x %02x\r\n",
           len_buf[0], len_buf[0], len_buf[1], len_buf[2], len_buf[3]);

    for (uint32_t i = 0; i < 4; i++) {
        while (SPI1_TXDATA > 0xFF) {} // full bit set, wait
        SPI1_TXDATA = len_buf[i];
        spi_rxdata_read();
    }

    cs_deassert();    
    printf(" | Waiting for handshake pin ready...");
    while (!handshake_ready()) {}
    printf("DONE\r\n");
}

//----------------------------------------------------------------------
void spi_send(const char *str_p)
{
    uint32_t len;

    printf("[+] spi_send: %s", str_p);
    
    // 1. Send the header where 0x02 tells ESP to receive a command
    spi_send_recv_header();

    // 2. Send the length of data, len_buf[3] must be 'A'
    len = strlen(str_p);
    spi_send_recv_length(len);

    // 3. Send the actual data
    printf(" | spi_send data: sending:\r\n");
    for (uint32_t i = 0; i < len; i++) {
        if (i && (i%8==0)) {
            printf("\r\n");
        }
        printf(" %02x", str_p[i]);
    }
    printf("\r\n");
    
    for (uint32_t i = 0; i < len; i++) {
        while (SPI1_TXDATA > 0xFF) {} // full bit set, wait
        SPI1_TXDATA = str_p[i];
        spi_rxdata_read();
    }

    cs_deassert();

    printf(" | Waiting for handshake pin ready...");
    while (!handshake_ready()) {}
    printf("DONE\r\n");
}


/* ----------------------------------------------------------------------
 * From https://github.com/sifive/riscv-zephyr/blob/hifive1-revb/drivers/wifi/eswifi/eswifi_bus_spi.c
 * CMD/DATA protocol:
 * 1. Module raises data-ready when ready for **command phase**
 * 2. Host announces command start by lowering chip-select (csn)
 * 3. Host write the command (possibly several spi transfers)
 * 4. Host announces end of command by raising chip-select
 * 5. Module lowers data-ready signal
 * 6. Module raises data-ready to signal start of the **data phase**
 * 7. Host lowers chip-select
 * 8. Host fetch data as long as data-ready pin is up
 * 9. Module lowers data-ready to signal the end of the data Phase
 * 10. Host raises chip-select
 */
