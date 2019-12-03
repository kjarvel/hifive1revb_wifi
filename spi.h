#ifndef SPI_H__
#define SPI_H__

#include <stdint.h>

void spi_init(uint32_t spi_clock);
void spi_send(const char *str_p);

#endif
