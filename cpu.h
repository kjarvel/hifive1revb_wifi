#ifndef CPU_H__
#define CPU_H__

#include <stdint.h>

uint32_t cpu_freq(void);
void delay(uint32_t counter);
void cpu_clock_init(void);

#endif
