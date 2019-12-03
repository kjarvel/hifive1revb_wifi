#include <string.h>
#include <stdint.h>
#include "uart.h"
#include "cpu.h"

#define IOF_EN          *(volatile uint32_t*)0x10012038
#define IOF_SEL         *(volatile uint32_t*)0x1001203C

#define UART0_TXDATA    *(volatile uint32_t*)0x10013000
#define UART0_RXDATA    *(volatile uint32_t*)0x10013004
#define UART0_TXCTRL    *(volatile uint32_t*)0x10013008
#define UART0_RXCTRL    *(volatile uint32_t*)0x1001300C
#define UART0_DIV       *(volatile uint32_t*)0x10013018

#define UART_TXEN_I     0U
#define UART_RXEN_I     0U

#define BIT_MASK(bit) (1UL<<(bit))
#define UART0_RX    16
#define UART0_TX    17
#define IOF_UART_ENABLE (BIT_MASK(UART0_RX) | BIT_MASK(UART0_TX))

void uart_init(uint32_t baudrate)
{
    // Select HW I/O function 0 (IOF0) for all pins. 
    // IOF0 means UART mode for GPIO pins 16-17.
    IOF_SEL = 0;

    IOF_EN |= IOF_UART_ENABLE;
    UART0_TXCTRL = 0;
    UART0_RXCTRL = 0;
    UART0_DIV = cpu_freq() / baudrate - 1UL;
    UART0_TXCTRL = BIT_MASK(UART_TXEN_I);
    UART0_RXCTRL = BIT_MASK(UART_RXEN_I);
}

void uart_send(const char *str_p)
{
    uint32_t len = strlen(str_p);
    for (uint32_t i=0; i < len; i++)
    {
        while (UART0_TXDATA > 0xFF) {} // full bit set, wait
        UART0_TXDATA = str_p[i] & 0xFFU;
    }
}

int uart_putchar(char c)
{
    while (UART0_TXDATA > 0xFF) {} // full bit set, wait
    UART0_TXDATA = c & 0xFFU;
    return 0;
}

int uart_getchar(void)
{
    uint32_t c;
    
    while (1) {
        c = UART0_RXDATA;   // Read the RX register EXACTLY once
        if (c <= 0xFF) {    // Empty bit was not set, which means DATA is valid
            return c;
        }
    } // Loop as long empty bit is set
}
