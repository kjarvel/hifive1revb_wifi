#include <stdint.h>
#include "cpu.h"

#define CPU_FREQ 320000000
#define PLLR_2 1
#define PLLQ_2 1

uint32_t cpu_freq(void)
{
    return CPU_FREQ;
}

void delay(uint32_t counter)
{
    volatile uint32_t i = 0;
    while (i < counter) {
        i++;
    }
}

// METAL_SIFIVE_FE310_G000_PRCI_10008000_BASE_ADDRESS is defined in
// <metal/machine/platform.h> but that's not used here (who would
// want to use a decimal number for an address?)
#define PLLCFG          *(volatile uint32_t*)0x10008008
#define BITS(idx, val)  ((val) << (idx))

// PLLCFG register bit indexes
#define PLLR_I        0U
#define PLLF_I        4U
#define PLLQ_I        10U
#define PLLSEL_I      16U
#define PLLREFSEL_I   17U
#define PLLBYPASS_I   18U
#define PLLLOCK_I     31U

/* Sets CPU speed to 320 MHz */
void cpu_clock_init(void)
{
    uint32_t cfg_temp = 0;

    /* There is a 16 MHz crystal oscillator HFXOSC on the board */
    cfg_temp |= BITS(PLLREFSEL_I, 1);     // Drive PLL from 16 MHz HFXOSC
    cfg_temp |= BITS(PLLR_I, PLLR_2);     // Divide ratio. R=2 for 8 MHz out
    cfg_temp |= BITS(PLLF_I, 80/2 - 1U);  // Multiply ratio. 8 MHz x 40 is 640 MHz out
    cfg_temp |= BITS(PLLQ_I, PLLQ_2);     // Divide 640MHz with 2 to get 320 MHz
    // PLLSEL_I = 0    : PLL is not driving hfclk for now
    // PLLBYPASS_I = 0 : Enables PLL
    PLLCFG = cfg_temp;

    delay(1000);

    while ( PLLCFG & BITS(PLLLOCK_I, 1) == 0) {} // Wait until PLL locks
    PLLCFG |= BITS(PLLSEL_I, 1);          // Let PLL drive hfclk
}
