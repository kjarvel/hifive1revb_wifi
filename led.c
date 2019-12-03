#include <stdint.h>
#include "led.h"

#define GPIO_OUTPUT_EN      *(volatile uint32_t*)0x10012008

void LED_on(led_t led)
{
    GPIO_OUTPUT_EN |= (uint32_t)led;
}

void LED_off(led_t led)
{
    GPIO_OUTPUT_EN &= ~(uint32_t)led;
}

void LED_toggle(led_t led)
{
    uint32_t led_current = GPIO_OUTPUT_EN & (uint32_t)LED_ALL;
    if (led & led_current) {
        // LED is on now, disable it
        LED_off(led);
    } else {
        LED_on(led);
    }
}
